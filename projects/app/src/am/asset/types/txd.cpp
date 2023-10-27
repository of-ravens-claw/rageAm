#include "txd.h"

#include "am/file/iterator.h"
#include "am/graphics/texture/imageformat.h"
#include "am/graphics/texture/surface.h"
#include "am/string/string.h"
#include "am/system/enum.h"
#include "am/task/worker.h"
#include "am/xml/iterator.h"
#include "helpers/format.h"
#include "rage/atl/set.h"

void rageam::asset::TextureOptions::GetCompressionOptions(texture::CompressionOptions& outOptions) const
{
	outOptions.MipMaps = GenerateMips;
	outOptions.MaxResolution = MaxSize;
	outOptions.Format = Format;
	outOptions.MipFilter = MipFilter;
	outOptions.ResizeFilter = ResizeFilter;
	outOptions.Quality = Quality;
}

void rageam::asset::TextureOptions::Serialize(XmlHandle& node) const
{
	XML_SET_CHILD_VALUE(node, MaxSize);
	XML_SET_CHILD_VALUE(node, GenerateMips);
	XML_SET_CHILD_VALUE(node, Format);
	XML_SET_CHILD_VALUE(node, MipFilter);
	XML_SET_CHILD_VALUE(node, ResizeFilter);
	XML_SET_CHILD_VALUE(node, Quality);
}

void rageam::asset::TextureOptions::Deserialize(const XmlHandle& node)
{
	XML_GET_CHILD_VALUE(node, MaxSize);
	XML_GET_CHILD_VALUE(node, GenerateMips);
	XML_GET_CHILD_VALUE(node, Format);
	XML_GET_CHILD_VALUE(node, MipFilter);
	XML_GET_CHILD_VALUE(node, ResizeFilter);
	XML_GET_CHILD_VALUE(node, Quality);
}

void rageam::asset::Texture::Serialize(XmlHandle& node) const
{
	Options.Serialize(node);
}

void rageam::asset::Texture::Deserialize(const XmlHandle& node)
{
	Options.Deserialize(node);
}

bool rageam::asset::TxdAsset::CompileToGame(rage::grcTextureDictionary* ppOutGameFormat)
{
	ReportProgress(L"- Compressing textures", 0);

	u32 textureCount = m_Textures.GetNumUsedSlots();

	// Step 1: Compress all textures in parallel threads
	struct CompressJob
	{
		char* PixelData;
		texture::CompressedInfo Info;
		const Texture* Texture;
	};

	Tasks compressTasks;
	rage::atArray<CompressJob> compressJobs;

	compressTasks.Reserve(textureCount);
	compressJobs.Resize(textureCount);

	// For calculating % of completion
	std::atomic_int doneTextures = 0;

	u32 index = 0;
	for (const Texture& texture : m_Textures)
	{
		// Take pointer so it can be used in lambda
		const Texture* pTexture = &texture;

		ConstWString fmt = pTexture->IsPreCompressed ? L"[TxdAsset] Load %ls" : L"[TxdAsset] Compress %ls";

		compressTasks.Emplace(BackgroundWorker::Run(
			[this, pTexture, &doneTextures, textureCount, &compressJobs, index]
			{
				file::WPath texturePath = pTexture->GetFullPath();

				CompressJob& job = compressJobs[index];
				job.Texture = pTexture;

				texture::CompressionOptions options;
				pTexture->Options.GetCompressionOptions(options);

				texture::Surface surface;
				surface.SetCompressOptions(options);
				if (!surface.LoadToRam(texturePath, &job.PixelData))
					return false;

				job.Info = surface.GetInfo();

				// Accumulate and report progress
				if (CompileCallback)
				{
					++doneTextures;

					double progress = static_cast<double>(doneTextures) / textureCount;

					char sizeText[32];
					FormatBytes(sizeText, 32, job.Info.Size);

					CompileCallback(String::FormatTemp(L"Texture %i/%u: %hs -> %hs",
						doneTextures.load(), textureCount, pTexture->Name, sizeText), progress);
				}

				return true;
			}, fmt, pTexture->GetFileName()));
		index++;
	}

	// Step 2: Wait compression tasks to finish
	if (!BackgroundWorker::WaitFor(compressTasks))
		return false;

	// Step 3: Create grcTexture's
	rage::grcTextureDictionary& txd = *ppOutGameFormat;
	for (CompressJob& job : compressJobs)
	{
		texture::CompressedInfo& info = job.Info;

		ConstString textureName = job.Texture->Name;
		auto gameTexture = new rage::grcTextureDX11(
			info.Width, info.Height, 1, info.MipCount, info.Format, job.PixelData);
		gameTexture->SetName(textureName);

		txd.Insert(textureName, gameTexture);
	}

	return true;
}

void rageam::asset::TxdAsset::Refresh()
{
	// Scan existing textures
	rage::atSet<rage::atWideString> imageFiles;
	rage::atSet<u32> imageNames; // To check if there's two textures with same name and different extension
	file::FindData entry;
	file::Iterator it(GetDirectoryPath() / L"*.*");
	while (it.Next())
	{
		it.GetCurrent(entry);

		ConstWString extension = file::GetExtension<wchar_t>(entry.Path);
		if (!texture::IsImageFormat(extension))
			continue;

		rage::atWideString fileName = file::GetFileName<wchar_t>(entry.Path);

		// Check if we already parsed image file with such name (but different extension)
		u32 nameHash = rage::joaat(entry.Path.GetFileNameWithoutExtension());
		if (imageNames.Contains(nameHash))
		{
			AM_WARNINGF(
				L"TxdAsset::Refresh() -> Found 2 image files with the same name but different extension, ignoring last occurence: %ls",
				fileName.GetCStr());
			continue;
		}

		if (!ValidateTextureName(fileName))
			continue;

		imageNames.Insert(nameHash);
		imageFiles.Emplace(std::move(fileName));
	}

	// Find and remove image files that were deleted
	rage::atArray<u32> imagesToRemove; // Store temp array because we can't alter collection (m_Textures) we're iterating
	for (const Texture& texture : m_Textures)
	{
		u32 key = texture.GetHashKey();

		// Image is not in directory, file was removed
		if (!imageFiles.ContainsAt(key))
		{
			imagesToRemove.Add(key);
			AM_TRACEF(L"TxdAsset::Refresh() -> Image file %ls was removed.", texture.GetFileName());
		}
		// Both config and image file are present, don't do anything to it
		else
		{
			imageFiles.RemoveAt(key);
		}
	}

	// Now we can safely clean up removes images
	for (u32 key : imagesToRemove)
		m_Textures.RemoveAt(key);

	// Add new images (that didn't exist before)
	for (const rage::atWideString& imageName : imageFiles)
	{
		m_Textures.ConstructAt(joaat(imageName), this, imageName);
	}
}

void rageam::asset::TxdAsset::Serialize(XmlHandle& node) const
{
	for (const Texture& texture : m_Textures)
	{
		XmlHandle xTexture = node.AddChild("Texture");
		xTexture.SetAttribute("File", String::ToUtf8Temp(texture.GetFileName()));
		texture.Serialize(xTexture);
	}
}

void rageam::asset::TxdAsset::Deserialize(const XmlHandle& node)
{
	for (XmlHandle xTexture : XmlIterator(node, "Texture"))
	{
		ConstString fileName;
		xTexture.GetAttribute("File", fileName);

		ConstWString texturePath = String::ToWideTemp(fileName);
		u32 hashKey = rage::joaat(texturePath); // AssetSource::GetHashKey() is file name hash

		Texture& texture = m_Textures.ConstructAt(hashKey, this, texturePath);
		texture.Deserialize(xTexture);
	}
}

bool rageam::asset::TxdAsset::RenameTextureTune(const Texture& textureTune, const file::WPath& newFileName)
{
	file::Path newName;
	if (!GetValidatedTextureName(newFileName, newName))
		return false;

	// We have to make copy because original texture tune will be destroyed on Remove
	Texture textureTuneCopy = textureTune;
	String::Copy(textureTuneCopy.Name, Texture::MAX_NAME, newName);
	textureTuneCopy.SetFileName(newFileName);

	// We have to reinsert tune in set with new name, remove it first
	m_Textures.Remove(textureTune);

	// Now it was renamed, we can insert it back
	m_Textures.Emplace(std::move(textureTuneCopy));

	return true;
}

void rageam::asset::TxdAsset::RemoveTextureTune(const Texture& textureTune)
{
	m_Textures.Remove(textureTune);
}

bool rageam::asset::TxdAsset::ValidateTextureName(ConstWString fileName, bool showWarningMessage)
{
	while (*fileName)
	{
		wchar_t c = *fileName;
		fileName++;

		if (static_cast<u16>(c) <= UINT8_MAX)
			continue;

		if (showWarningMessage)
		{
			AM_WARNINGF(
				L"TxdAsset::ValidateTextureName() -> Found non ASCII symbol in %ls (%lc)! Name cannot be converted safely, skipping!",
				fileName, c);
		}

		return false;
	}
	return true;
}

bool rageam::asset::TxdAsset::IsAssetTexture(const file::WPath& texturePath)
{
	file::WPath assetPath;
	if (!GetTxdAssetPathFromTexture(texturePath, assetPath)) 
		return false; // Not in asset directory
	return IsSupportedTextureExtension(texturePath);
}

bool rageam::asset::TxdAsset::GetTxdAssetPathFromTexture(const file::WPath& texturePath, file::WPath& path)
{
	path = GetTxdAssetPathFromTexture(texturePath);
	return ImmutableWString(path).EndsWith(ASSET_ITD_EXT);
}

rageam::file::WPath rageam::asset::TxdAsset::GetTxdAssetPathFromTexture(const file::WPath& texturePath)
{
	return texturePath.GetParentDirectory();
}

bool rageam::asset::TxdAsset::IsSupportedTextureExtension(const file::WPath& texturePath)
{
	return texture::IsImageFormat(texturePath.GetExtension());
}

rageam::asset::Texture* rageam::asset::TxdAsset::TryFindTextureTuneFromPath(const file::WPath& texturePath) const
{
	ConstWString fileName = file::GetFileName<wchar_t>(texturePath);
	return m_Textures.TryGetAt(rage::joaat(fileName));
}

rageam::asset::Texture* rageam::asset::TxdAsset::CreateTuneForPath(const file::WPath& texturePath)
{
	if (!IsSupportedTextureExtension(texturePath))
		return nullptr;

	Texture newTune(this, texturePath.GetFileName());
	return &m_Textures.Insert(newTune);
}

bool rageam::asset::TxdAsset::GetValidatedTextureName(const file::WPath& texturePath, file::Path& outName) const
{
	outName = "";

	file::WPath fileName = texturePath.GetFileNameWithoutExtension();
	if (!ValidateTextureName(fileName))
		return false;

	outName = String::ToAnsiTemp(fileName);
	return true;
}

rage::grcTexture* rageam::asset::TxdAsset::CompileTexture(const Texture* textureTune) const
{
	const file::WPath& texturePath = textureTune->GetFullPath();

	texture::CompressionOptions options;
	textureTune->Options.GetCompressionOptions(options);

	char* pixelData;

	texture::Surface surface;
	surface.SetCompressOptions(options);
	if (!surface.LoadToRam(texturePath, &pixelData))
	{
		delete[] pixelData;
		return nullptr;
	}

	const texture::CompressedInfo& info = surface.GetInfo();
	rage::grcTexture* gameTexture = new rage::grcTextureDX11(info.Width, info.Height, 1, info.MipCount, info.Format, pixelData);

	const file::WPath& textureName = texturePath.GetFileNameWithoutExtension();
	gameTexture->SetName(String::ToAnsiTemp(textureName));

	return gameTexture;
}
