#include "texturegnm.h"

#include "am/graphics/image/image.h"
#include "am/graphics/dxgi_utils.h"
#include "am/graphics/render.h"

#define DXT1	0x31545844
#define DXT2	0x32545844
#define DXT3	0x33545844
#define DXT4	0x34545844
#define DXT5	0x35545844
#define ATI1	0x31495441
#define ATI2	0x32495441
#define RGBG	0x47424752
#define GRGB	0x42475247
#define G8R8	0x38523847
#define YUY2	0x32595559
#define R16		0x20363152
#define BC6		0x20364342
#define BC7		0x20374342
#define R8		0x20203852

#pragma region grcOrbisDurangoTextureBase
rage::grcOrbisDurangoTextureBase::grcOrbisDurangoTextureBase() : grcTexture(TEXTURE_NORMAL)
{
	m_pGraphicsMem = nullptr;
	m_GraphicsMemorySize = 0;
	m_pLockInfoPtr = nullptr;
	memset(&m_UserMemory, 0, sizeof(m_UserMemory));
}

rage::grcOrbisDurangoTextureBase::grcOrbisDurangoTextureBase(eTextureType type) : grcTexture(type)
{
	m_pGraphicsMem = nullptr;
	m_GraphicsMemorySize = 0;
	m_pLockInfoPtr = nullptr;
	memset(&m_UserMemory, 0, sizeof(m_UserMemory));
}

rage:: grcOrbisDurangoTextureBase::grcOrbisDurangoTextureBase(GRC_ORBIS_DURANGO_TEXTURE_DESC& info, eTextureType type) : grcTexture(type)
{
	// SetFromDescription(info);
	memset(&m_UserMemory, 0, sizeof(m_UserMemory));
}

// ReSharper disable once CppPossiblyUninitializedMember
rage::grcOrbisDurangoTextureBase::grcOrbisDurangoTextureBase(const datResource& rsc) : grcTexture(rsc)
{
	rsc.Fixup(m_Name);
	rsc.Fixup(m_pGraphicsMem);
	rsc.Fixup(m_pLockInfoPtr);
}

bool rage::grcOrbisDurangoTextureBase::Copy(const grcImage* pImage)
{
	AM_UNREACHABLE("grcOrbisDurangoTextureBase::Copy() -> Not implemented.");
}

enum
{
	CHANNEL_ALPHA = 0,
	CHANNEL_RED = 1,
	CHANNEL_GREEN = 2,
	CHANNEL_BLUE = 3,
	CHANNEL_DEPTH = 4,
	CHANNEL_STENCIL = 5,
};

rage::atFixedBitSet<8, u8> rage::grcOrbisDurangoTextureBase::FindUsedChannels() const
{
	auto eFormat = m_Texture.Format;
	rage::atFixedBitSet<8, unsigned char> bits;

	switch (eFormat)
	{
		// RGBA	
	case GRC_TEMP_XG_FORMAT_R32G32B32A32_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R32G32B32A32_FLOAT:
	case GRC_TEMP_XG_FORMAT_R32G32B32A32_UINT:
	case GRC_TEMP_XG_FORMAT_R32G32B32A32_SINT:
	case GRC_TEMP_XG_FORMAT_R16G16B16A16_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R16G16B16A16_FLOAT:
	case GRC_TEMP_XG_FORMAT_R16G16B16A16_UNORM:
	case GRC_TEMP_XG_FORMAT_R16G16B16A16_UINT:
	case GRC_TEMP_XG_FORMAT_R16G16B16A16_SNORM:
	case GRC_TEMP_XG_FORMAT_R16G16B16A16_SINT:
	case GRC_TEMP_XG_FORMAT_R10G10B10A2_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R10G10B10A2_UNORM:
	case GRC_TEMP_XG_FORMAT_R10G10B10A2_UINT:
	case GRC_TEMP_XG_FORMAT_R8G8B8A8_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R8G8B8A8_UNORM:
	case GRC_TEMP_XG_FORMAT_R8G8B8A8_UNORM_SRGB:
	case GRC_TEMP_XG_FORMAT_R8G8B8A8_UINT:
	case GRC_TEMP_XG_FORMAT_R8G8B8A8_SNORM:
	case GRC_TEMP_XG_FORMAT_R8G8B8A8_SINT:
	case GRC_TEMP_XG_FORMAT_B8G8R8A8_UNORM:
	case GRC_TEMP_XG_FORMAT_B5G5R5A1_UNORM:
	case GRC_TEMP_XG_FORMAT_BC1_TYPELESS:
	case GRC_TEMP_XG_FORMAT_BC1_UNORM:
	case GRC_TEMP_XG_FORMAT_BC1_UNORM_SRGB:
	case GRC_TEMP_XG_FORMAT_BC2_TYPELESS:
	case GRC_TEMP_XG_FORMAT_BC2_UNORM:
	case GRC_TEMP_XG_FORMAT_BC2_UNORM_SRGB:
	case GRC_TEMP_XG_FORMAT_BC3_TYPELESS:
	case GRC_TEMP_XG_FORMAT_BC3_UNORM:
	case GRC_TEMP_XG_FORMAT_BC3_UNORM_SRGB:
	case GRC_TEMP_XG_FORMAT_BC7_TYPELESS:
	case GRC_TEMP_XG_FORMAT_BC7_UNORM:
	case GRC_TEMP_XG_FORMAT_BC7_UNORM_SRGB:
		bits.Set(::CHANNEL_RED);
		bits.Set(::CHANNEL_GREEN);
		bits.Set(::CHANNEL_BLUE);
		bits.Set(::CHANNEL_ALPHA);
		break;

		// RGB
	case GRC_TEMP_XG_FORMAT_R32G32B32_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R32G32B32_FLOAT:
	case GRC_TEMP_XG_FORMAT_R32G32B32_UINT:
	case GRC_TEMP_XG_FORMAT_R32G32B32_SINT:
	case GRC_TEMP_XG_FORMAT_R9G9B9E5_SHAREDEXP:
	case GRC_TEMP_XG_FORMAT_R11G11B10_FLOAT:
	case GRC_TEMP_XG_FORMAT_B5G6R5_UNORM:
	case GRC_TEMP_XG_FORMAT_B8G8R8X8_UNORM:
	case GRC_TEMP_XG_FORMAT_R8G8_B8G8_UNORM:
	case GRC_TEMP_XG_FORMAT_G8R8_G8B8_UNORM:
	case GRC_TEMP_XG_FORMAT_BC6H_TYPELESS:
	case GRC_TEMP_XG_FORMAT_BC6H_UF16:
	case GRC_TEMP_XG_FORMAT_BC6H_SF16:
		bits.Set(::CHANNEL_RED);
		bits.Set(::CHANNEL_GREEN);
		bits.Set(::CHANNEL_BLUE);
		break;

		// RG
	case GRC_TEMP_XG_FORMAT_R32G32_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R32G32_FLOAT:
	case GRC_TEMP_XG_FORMAT_R32G32_UINT:
	case GRC_TEMP_XG_FORMAT_R32G32_SINT:
	case GRC_TEMP_XG_FORMAT_R32G8X24_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R16G16_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R16G16_FLOAT:
	case GRC_TEMP_XG_FORMAT_R16G16_UNORM:
	case GRC_TEMP_XG_FORMAT_R16G16_UINT:
	case GRC_TEMP_XG_FORMAT_R16G16_SNORM:
	case GRC_TEMP_XG_FORMAT_R16G16_SINT:
	case GRC_TEMP_XG_FORMAT_R24G8_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R8G8_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R8G8_UNORM:
	case GRC_TEMP_XG_FORMAT_R8G8_UINT:
	case GRC_TEMP_XG_FORMAT_R8G8_SNORM:
	case GRC_TEMP_XG_FORMAT_R8G8_SINT:
	case GRC_TEMP_XG_FORMAT_BC5_TYPELESS:
	case GRC_TEMP_XG_FORMAT_BC5_UNORM:
	case GRC_TEMP_XG_FORMAT_BC5_SNORM:
#if RSG_DURANGO
	case GRC_TEMP_XG_FORMAT_NV12:
#endif
		bits.Set(::CHANNEL_RED);
		bits.Set(::CHANNEL_GREEN);
		break;

		// R
	case GRC_TEMP_XG_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R32_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R32_FLOAT:
	case GRC_TEMP_XG_FORMAT_R32_UINT:
	case GRC_TEMP_XG_FORMAT_R32_SINT:
	case GRC_TEMP_XG_FORMAT_R24_UNORM_X8_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R16_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R16_FLOAT:
	case GRC_TEMP_XG_FORMAT_R16_UNORM:
	case GRC_TEMP_XG_FORMAT_R16_UINT:
	case GRC_TEMP_XG_FORMAT_R16_SNORM:
	case GRC_TEMP_XG_FORMAT_R16_SINT:
	case GRC_TEMP_XG_FORMAT_R8_TYPELESS:
	case GRC_TEMP_XG_FORMAT_R8_UNORM:
	case GRC_TEMP_XG_FORMAT_R8_UINT:
	case GRC_TEMP_XG_FORMAT_R8_SNORM:
	case GRC_TEMP_XG_FORMAT_R8_SINT:
	case GRC_TEMP_XG_FORMAT_R1_UNORM:
	case GRC_TEMP_XG_FORMAT_BC4_TYPELESS:
	case GRC_TEMP_XG_FORMAT_BC4_UNORM:
	case GRC_TEMP_XG_FORMAT_BC4_SNORM:
		bits.Set(::CHANNEL_RED);
		break;

		// G
	case GRC_TEMP_XG_FORMAT_X32_TYPELESS_G8X24_UINT:
	case GRC_TEMP_XG_FORMAT_X24_TYPELESS_G8_UINT:
		bits.Set(::CHANNEL_GREEN);
		break;

		// A
	case GRC_TEMP_XG_FORMAT_A8_UNORM:
		bits.Set(::CHANNEL_ALPHA);
		break;

		// Depth
	case GRC_TEMP_XG_FORMAT_D32_FLOAT:
	case GRC_TEMP_XG_FORMAT_D16_UNORM:
		bits.Set(::CHANNEL_DEPTH);
		break;

	case GRC_TEMP_XG_FORMAT_D32_FLOAT_S8X24_UINT:
	case GRC_TEMP_XG_FORMAT_D24_UNORM_S8_UINT:
		bits.Set(::CHANNEL_DEPTH);
		bits.Set(::CHANNEL_STENCIL);
		break;

	default:
		AM_WARNINGF("Don't know what channels are in use in texture format %d", eFormat);
		break;
	}
	return bits;
}

namespace rage
{
	static const rage::GRC_TEMP_XG_FORMAT translate[] =
	{
		GRC_TEMP_XG_FORMAT_UNKNOWN,
		GRC_TEMP_XG_FORMAT_B5G6R5_UNORM,
		GRC_TEMP_XG_FORMAT_B8G8R8A8_UNORM, // grctfA8R8G8B8
		GRC_TEMP_XG_FORMAT_R16_FLOAT,

		GRC_TEMP_XG_FORMAT_R32_FLOAT,
		GRC_TEMP_XG_FORMAT_R11G11B10_FLOAT, // grctfA2B10G10R10 (PC TODO - No alpha may be trouble)
		GRC_TEMP_XG_FORMAT_R10G10B10A2_UNORM, // grctfA2B10G10R10ATI
		GRC_TEMP_XG_FORMAT_R16G16B16A16_FLOAT,

		GRC_TEMP_XG_FORMAT_R16G16_UNORM,
		GRC_TEMP_XG_FORMAT_R16G16_FLOAT,
		GRC_TEMP_XG_FORMAT_R32G32B32A32_FLOAT,
		GRC_TEMP_XG_FORMAT_R16G16B16A16_FLOAT,

		GRC_TEMP_XG_FORMAT_R16G16B16A16_UNORM,
		GRC_TEMP_XG_FORMAT_R8_UNORM, // lies .. L8
		GRC_TEMP_XG_FORMAT_R16_UNORM, // lies .. L16
		GRC_TEMP_XG_FORMAT_R8G8_UNORM,

		GRC_TEMP_XG_FORMAT_R8G8_UNORM, // grctfG8R8_XENON
		GRC_TEMP_XG_FORMAT_B5G5R5A1_UNORM,
		GRC_TEMP_XG_FORMAT_D24_UNORM_S8_UINT,
		GRC_TEMP_XG_FORMAT_B4G4R4A4_UNORM,

		GRC_TEMP_XG_FORMAT_R32G32_FLOAT,
		GRC_TEMP_XG_FORMAT_D24_UNORM_S8_UINT,
		GRC_TEMP_XG_FORMAT_D16_UNORM,
		GRC_TEMP_XG_FORMAT_R8G8_UNORM, // grctfG8B8

		GRC_TEMP_XG_FORMAT_D32_FLOAT,
		GRC_TEMP_XG_FORMAT_B8G8R8X8_UNORM, // grctfX8R8G8B8
		GRC_TEMP_XG_FORMAT_UNKNOWN,  // PC TODO - No Need for a NULL format.  Color targets can be NULL for DX11.  Need to handle this everywhere
		GRC_TEMP_XG_FORMAT_X24_TYPELESS_G8_UINT,
		GRC_TEMP_XG_FORMAT_A8_UNORM,
		GRC_TEMP_XG_FORMAT_R11G11B10_FLOAT,
		GRC_TEMP_XG_FORMAT_D32_FLOAT_S8X24_UINT,
		GRC_TEMP_XG_FORMAT_X32_TYPELESS_G8X24_UINT,
		GRC_TEMP_XG_FORMAT_BC1_UNORM,
		GRC_TEMP_XG_FORMAT_BC2_UNORM,
		GRC_TEMP_XG_FORMAT_BC3_UNORM,
		GRC_TEMP_XG_FORMAT_BC4_UNORM,
		GRC_TEMP_XG_FORMAT_BC5_UNORM,
		GRC_TEMP_XG_FORMAT_BC6H_UF16,
		GRC_TEMP_XG_FORMAT_BC7_UNORM,
		GRC_TEMP_XG_FORMAT_R8G8B8A8_SNORM, // grctfA8B8G8R8_SNORM
		GRC_TEMP_XG_FORMAT_R8G8B8A8_UNORM, // grctfA8B8G8R8
	#if RSG_DURANGO
		GRC_TEMP_XG_FORMAT_NV12,
	#endif
	};
	static_assert(ARRAYSIZE(translate) == grctfCount);

	GRC_TEMP_XG_FORMAT ConvertToXGFormat(grcTextureFormat eFormat)
	{
		AM_ASSERTS(eFormat < grctfCount);

		GRC_TEMP_XG_FORMAT uFormat = translate[eFormat];
		AM_ASSERTS(uFormat != GRC_TEMP_XG_FORMAT_UNKNOWN);
		return uFormat;
	}

	grcTextureFormat ConvertTogrcFormat(GRC_TEMP_XG_FORMAT fmt)
	{
		for (int i = 0; i < ARRAYSIZE(translate); i++)
		{
			if (translate[i] == fmt)
				return (grcTextureFormat)i;
		}

		if (fmt == GRC_TEMP_XG_FORMAT_R8_UNORM)
			return grctfL8; // we don't have grctfR8, but we do have grctfL8 ..

		if (fmt == GRC_TEMP_XG_FORMAT_R16_UNORM)
			return grctfL16; // we don't have grctfR16, but we do have grctfL16 ..

		return grctfNone;
	}


	u32 GetBitsPerPixelFromFormat(grcTextureFormat eFormat)
	{
		static u32 translate[] =
		{
			0, // grctfNone,
			16, // grctfR5G6B5,
			32, // grctfA8R8G8B8,
			16, // grctfR16F,

			32, // grctfR32F,
			32, // grctfA2B10G10R10, // PC TODO - No alpha may be trouble
			32, // grctfA2B10G10R10ATI,
			64, // grctfA16B16G16R16F,

			32, // grctfG16R16,
			32, // grctfG16R16F,
			128, // grctfA32B32G32R32F,
			64, // grctfA16B16G16R16F_NoExpand,

			64, // grctfA16B16G16R16,
			8, // grctfL8,
			16, // grctfL16,
			16, // grctfG8R8,

			16, // grctfG8R8_XENON,
			16, // grctfA1R5G5B5,
			32, // grctfD24S8,
			16, // grctfA4R4G4B4,

			64, // grctfG32R32F,
			32, // grctfD24FS8_ReadStencil,
			16, // grctfD16,
			16, // grctfG8B8,

			32, // grctfD32F,
			32, // grctfX8R8G8B8,
			0, // grctfNULL,  // PC TODO - No Need for a NULL format.  Color targets can be NULL for DX11.  Need to handle this everywhere
			32, // grctfX24G8,
			8, // grctfA8,
			32, // grctfR11G11B10F,
			32, // grctfD32FS8,
			32, // grctfX32S8,
			4, // grctfDXT1,
			8, // grctfDXT3,
			8, // grctfDXT5,
			4, // grctfDXT5A,
			8, // grctfDXN,
			8, // grctfBC6,
			8, // grctfBC7,
			32, // grctfA8B8G8R8_SNORM
			32, // grctfA8B8G8R8
	#if RSG_DURANGO
			12, // grctfNV12
	#endif
		};
		static_assert(ARRAYSIZE(translate) == grctfCount);
		AM_ASSERTS(eFormat < grctfCount);
		u32 uFormat = translate[eFormat];
		return uFormat;
	}
}

u8 rage::grcOrbisDurangoTextureBase::GetBitsPerPixel() const
{
	return rage::GetBitsPerPixelFromFormat((grcTextureFormat)m_Texture.Format);
}

int	rage::grcOrbisDurangoTextureBase::GetStride(u32 uMipLevel) const
{
	GRC_TEMP_XG_FORMAT xgFormat = (GRC_TEMP_XG_FORMAT)m_Texture.Format;
	int width = Max<int>(1, GetWidth() >> uMipLevel);

	if ((xgFormat >= GRC_TEMP_XG_FORMAT_BC1_TYPELESS && xgFormat <= GRC_TEMP_XG_FORMAT_BC1_UNORM_SRGB) || // DXT1
		(xgFormat >= GRC_TEMP_XG_FORMAT_BC4_TYPELESS && xgFormat <= GRC_TEMP_XG_FORMAT_BC4_SNORM)) // DXT5A
	{
		// Round width up to next multiple of 4, then multiply by 2 (which is really divide by 4 to get # blocks and multiply by 8 bytes per block)
		return ((width + 3) & 0xFFFC) * 2;
	}

	if ((xgFormat >= GRC_TEMP_XG_FORMAT_BC2_TYPELESS && xgFormat <= GRC_TEMP_XG_FORMAT_BC3_UNORM_SRGB) || // DXT3 or DXT5
		(xgFormat >= GRC_TEMP_XG_FORMAT_BC5_TYPELESS && xgFormat <= GRC_TEMP_XG_FORMAT_BC5_SNORM) || // DXN
		(xgFormat >= GRC_TEMP_XG_FORMAT_BC6H_TYPELESS && xgFormat <= GRC_TEMP_XG_FORMAT_BC7_UNORM_SRGB)) // BC6H or BC7
	{
		// Round width up to next multiple of 4, then multiply by 4 (which is really divide by 4 to get # blocks and multiply by 16 bytes per block)
		return ((width + 3) & 0xFFFC) * 4;
	}

	int ret = (GetWidth() >> uMipLevel) * GetBitsPerPixelFromFormat(ConvertTogrcFormat((GRC_TEMP_XG_FORMAT)m_Texture.Format)) >> 3;

	if (ret == 0) 
		ret = 1;

	return ret;
}

int rage::grcOrbisDurangoTextureBase::GetRowCount(u32 uMipLevel) const
{
	GRC_TEMP_XG_FORMAT xgFormat = (GRC_TEMP_XG_FORMAT)m_Texture.Format;
	int ret = Max<int>(1, m_Texture.Height >> uMipLevel);

	if ((xgFormat >= GRC_TEMP_XG_FORMAT_BC1_TYPELESS && xgFormat <= GRC_TEMP_XG_FORMAT_BC5_SNORM) || // DXT1, DXT3, DXT5, DXT5A, DXN
		(xgFormat >= GRC_TEMP_XG_FORMAT_BC6H_TYPELESS && xgFormat <= GRC_TEMP_XG_FORMAT_BC7_UNORM_SRGB)) // BC6H or BC7
	{
		// Account for DXT/BC formats being in 4x4 blocks.
		ret = Max<int>(1, (ret + 3) >> 2);
	}

	return ret;
}

bool rage::grcOrbisDurangoTextureBase::Copy2D(const void* pSrc, const grcPoint& oSrcDim, const grcRect& oDstRect, const grcTextureLock& lock, s32 iMipLevel)
{
	u8* bits = reinterpret_cast<u8*>(lock.Base);

	u8* destPixel = bits + (lock.Pitch * oDstRect.y1) + (oDstRect.x1 * lock.BitsPerPixel >> 3); // first pixel to modify
	const u8* srcPixel = static_cast<const u8*>(pSrc); // first pixel to read from

	u32 width = ((oDstRect.x2 - oDstRect.x1) * lock.BitsPerPixel) >> 3;
	s32 height = (oDstRect.y2 - oDstRect.y1);

	for (int row = 0; row < height; row++)
	{
		memcpy(destPixel, srcPixel, width);
		destPixel += lock.Pitch;
		srcPixel += oSrcDim.x;
	}
	return true;
}

void rage::grcOrbisDurangoTextureBase::EnsureGpuWritable()
{
	// We don't have to do anything here, this only applies to the PS4.
}

void rage::grcOrbisDurangoTextureBase::SetFromDescription(GRC_ORBIS_DURANGO_TEXTURE_DESC& desc)
{
	m_Texture.Width = desc.m_Width;
	m_Texture.Height = desc.m_Height;
	m_Texture.Depth = desc.m_Depth;
	m_Texture.Dimension = desc.m_ArrayDimension;
	m_Texture.Mipmap = desc.m_NoOfMips;
	m_Texture.Padding = (u8)desc.m_ExtraBindFlags;

	// We use this as the "owns allocations" flag.
	if (IsResourceCompiling())
	{
		m_Texture.Remap = false;
	}
	else
	{
		m_Texture.Remap = true;
	}

	m_Texture.Format = (u8)desc.m_XGFormat;
	m_Texture.Cubemap = static_cast<u8>(desc.m_ImageType);
	m_Texture.Location = static_cast<u8>(desc.m_TileMode);
	m_pLockInfoPtr = new OFFSET_AND_PITCH[desc.m_NoOfMips];
	memset(m_pLockInfoPtr, 0, sizeof(OFFSET_AND_PITCH) * desc.m_NoOfMips);
}

void rage::grcOrbisDurangoTextureBase::SetLockInfo(u32 mip, u64 offset, u64 pitch)
{
	AM_ASSERT(m_pLockInfoPtr, "grcOrbisDurangoTextureBase::SetLockInfo()...Expecting lock info.");
	m_pLockInfoPtr[mip].Set(offset, pitch);
}

void rage::grcOrbisDurangoTextureBase::GetLockInfo(u32 mip, u64& offset, u64& pitch) const
{
	AM_ASSERT(m_pLockInfoPtr, "grcOrbisDurangoTextureBase::GetLockInfo()...Expecting lock info.");
	m_pLockInfoPtr[mip].Get(offset, pitch);
}

bool rage::grcOrbisDurangoTextureBase::GetOwnsAllocations() const
{
	return m_Texture.Remap != 0;
}

void rage::grcOrbisDurangoTextureBase::SetUsesPreAllocatedMemory(bool uses)
{
	m_Texture.Pitch = uses ? 1 : 0;
}

bool rage::grcOrbisDurangoTextureBase::GetUsesPreAllocatedMemory() const
{
	return m_Texture.Pitch != 0;
}
#pragma endregion

namespace rage
{
	sce::Gnm::DataFormat grcTextureGNMFormats::grctf_to_Orbis[] = {
		sce::Gnm::kDataFormatInvalid,			// grctfNone
		sce::Gnm::kDataFormatB5G6R5Unorm,		// grctfR5G6B5
		sce::Gnm::kDataFormatB8G8R8A8Unorm,		// grctfA8R8G8B8
		sce::Gnm::kDataFormatR16Float,			// grctfR16F
		sce::Gnm::kDataFormatR32Float,			// grctfR32F
		sce::Gnm::kDataFormatR10G10B10A2Unorm,	// grctfA2B10G10R10
		sce::Gnm::kDataFormatR10G10B10A2Unorm,	// grctfA2B10G10R10ATI
		sce::Gnm::kDataFormatB16G16R16A16Float,	// grctfA16B16G16R16F
		sce::Gnm::kDataFormatR16G16Unorm,		// grctfG16R16
		sce::Gnm::kDataFormatR16G16Float,		// grctfG16R16F
		sce::Gnm::kDataFormatR32G32B32A32Float,	// grctfA32B32G32R32F
		sce::Gnm::kDataFormatR32G32B32A32Float,	// grctfA16B16G16R16F_NoExpand
		sce::Gnm::kDataFormatR16G16B16A16Unorm,	// grctfA16B16G16R16
		sce::Gnm::kDataFormatL8Unorm,			// grctfL8
		sce::Gnm::kDataFormatL16Unorm,			// grctfL16
		sce::Gnm::kDataFormatR8G8Unorm,			// grctfG8R8
		sce::Gnm::kDataFormatInvalid,			// grctfG8R8_XENON
		sce::Gnm::kDataFormatB5G5R5A1Unorm,		// grctfA1R5G5B5
		sce::Gnm::kDataFormatInvalid,			// grctfD24S8
		sce::Gnm::kDataFormatB4G4R4A4Unorm,		// grctfA4R4G4B4
		sce::Gnm::kDataFormatR32G32Float,		// grctfG32R32F
		sce::Gnm::kDataFormatInvalid,			// grctfD24FS8_ReadStencil
		sce::Gnm::kDataFormatInvalid,			// grctfD16
		sce::Gnm::kDataFormatInvalid,			// grctfG8B8
		sce::Gnm::kDataFormatInvalid,			// grctfD32F
		sce::Gnm::kDataFormatB8G8R8X8Unorm,		// grctfX8R8G8B8
		sce::Gnm::kDataFormatInvalid,			// grctfNULL
		sce::Gnm::kDataFormatInvalid,			// grctfX24G8 -- note: use kDataFormatX24G8Uint for rendertargets
		sce::Gnm::kDataFormatA8Unorm,			// grctfA8
		sce::Gnm::kDataFormatR11G11B10Float,	// grctfR11G11B10F
		sce::Gnm::kDataFormatInvalid,			// grctfD32FS8
		sce::Gnm::kDataFormatInvalid,			// grctfX32S8
		sce::Gnm::kDataFormatBc1Unorm,			// grctfDXT1
		sce::Gnm::kDataFormatBc2Unorm,			// grctfDXT3
		sce::Gnm::kDataFormatBc3Unorm,			// grctfDXT5
		sce::Gnm::kDataFormatBc4Unorm,			// grctfDXT5A
		sce::Gnm::kDataFormatBc5Unorm,			// grctfDXN
		sce::Gnm::kDataFormatBc6Uf16,			// grctfBC6
		sce::Gnm::kDataFormatBc7Unorm,			// grctfBC7
		sce::Gnm::kDataFormatR8G8B8A8Snorm,		// grctfA8B8G8R8_SNORM
		sce::Gnm::kDataFormatR8G8B8A8Unorm		// grctfA8B8G8R8

	};
	static_assert(ARRAYSIZE(grcTextureGNMFormats::grctf_to_Orbis) == grctfCount);

	const sce::Gnm::DataFormat sce__Gnm__kDataFormatL4A4Unorm = { {{sce::Gnm::kSurfaceFormat4_4, sce::Gnm::kTextureChannelTypeUNorm, sce::Gnm::kTextureChannelX, sce::Gnm::kTextureChannelX, sce::Gnm::kTextureChannelX, sce::Gnm::kTextureChannelY}} };

	sce::Gnm::DataFormat grcTextureGNMFormats::grcImage_to_Orbis[] = {
		sce::Gnm::kDataFormatInvalid,			// UNKNOWN                    , // 0
		sce::Gnm::kDataFormatBc1Unorm,			// DXT1                       , // 1
		sce::Gnm::kDataFormatBc2Unorm,			// DXT3                       , // 2
		sce::Gnm::kDataFormatBc3Unorm,			// DXT5                       , // 3
		sce::Gnm::kDataFormatInvalid,			// CTX1                       , // 4 - like DXT1 but anchor colors are 8.8 instead of 5.6.5 (XENON-specific)
		sce::Gnm::kDataFormatInvalid,			// DXT3A                      , // 5 - alpha block of DXT3 (XENON-specific)
		sce::Gnm::kDataFormatInvalid,			// DXT3A_1111                 , // 6 - alpha block of DXT3, split into four 1-bit channels (XENON-specific)
		sce::Gnm::kDataFormatBc4Unorm,			// DXT5A                      , // 7 - alpha block of DXT5 (aka 'BC4', 'ATI1')
		sce::Gnm::kDataFormatBc5Unorm,			// DXN                        , // 8
		sce::Gnm::kDataFormatBc6Uf16,			// BC6                        , // 8
		sce::Gnm::kDataFormatBc7Unorm,			// BC7                        , // 8
		sce::Gnm::kDataFormatB8G8R8A8Unorm,		// A8R8G8B8                   , // 9
		sce::Gnm::kDataFormatR8G8B8A8Unorm,		// A8B8G8R8                   , // 10
		sce::Gnm::kDataFormatA8Unorm,			// A8                         , // 11 - 8-bit alpha-only (color is black)
		sce::Gnm::kDataFormatL8Unorm,			// L8                         , // 12 - 8-bit luminance (R=G=B=L, alpha is opaque)
		sce::Gnm::kDataFormatL8A8Unorm,			// A8L8                       , // 13 - 16-bit alpha + luminance
		sce::Gnm::kDataFormatB4G4R4A4Unorm,		// A4R4G4B4                   , // 14 - 16-bit color and alpha
		sce::Gnm::kDataFormatB5G5R5A1Unorm,		// A1R5G5B5                   , // 15 - 16-bit color with 1-bit alpha
		sce::Gnm::kDataFormatB5G6R5Unorm,		// R5G6B5                     , // 16 - 16-bit color
		sce::Gnm::kDataFormatInvalid,			// R3G3B2                     , // 17 - 8-bit color (not supported on consoles)
		sce::Gnm::kDataFormatInvalid,			// A8R3G3B2                   , // 18 - 16-bit color with 8-bit alpha (not supported on consoles)
		sce__Gnm__kDataFormatL4A4Unorm,			// A4L4                       , // 19 - 8-bit alpha + luminance (not supported on consoles)
		sce::Gnm::kDataFormatB10G10R10A2Unorm,	// A2R10G10B10                , // 20 - 32-bit color with 2-bit alpha
		sce::Gnm::kDataFormatR10G10B10A2Unorm,	// A2B10G10R10                , // 21 - 32-bit color with 2-bit alpha
		sce::Gnm::kDataFormatR16G16B16A16Unorm,	// A16B16G16R16               , // 22 - 64-bit four channel fixed point (s10e5 per channel -- sign, 5 bit exponent, 10 bit mantissa)
		sce::Gnm::kDataFormatR16G16Unorm,		// G16R16                     , // 23 - 32-bit two channel fixed point
		sce::Gnm::kDataFormatL16Unorm,			// L16                        , // 24 - 16-bit luminance
		sce::Gnm::kDataFormatR16G16B16A16Float,	// A16B16G16R16F              , // 25 - 64-bit four channel floating point (s10e5 per channel)
		sce::Gnm::kDataFormatR16G16Float,		// G16R16F                    , // 26 - 32-bit two channel floating point (s10e5 per channel)
		sce::Gnm::kDataFormatR16Float,			// R16F                       , // 27 - 16-bit single channel floating point (s10e5 per channel)
		sce::Gnm::kDataFormatR32G32B32A32Float,	// A32B32G32R32F              , // 28 - 128-bit four channel floating point (s23e8 per channel)
		sce::Gnm::kDataFormatR32G32Float,		// G32R32F                    , // 29 - 64-bit two channel floating point (s23e8 per channel)
		sce::Gnm::kDataFormatR32Float,			// R32F                       , // 30 - 32-bit single channel floating point (s23e8 per channel)
		sce::Gnm::kDataFormatInvalid,			// D15S1                      , // 31 - 16-bit depth + stencil (depth is 15-bit fixed point, stencil is 1-bit) (not supported on consoles)
		sce::Gnm::kDataFormatInvalid,			// D24S8                      , // 32 - 32-bit depth + stencil (depth is 24-bit fixed point, stencil is 8-bit)
		sce::Gnm::kDataFormatInvalid,			// D24FS8                     , // 33 - 32-bit depth + stencil (depth is 24-bit s15e8, stencil is 8-bit)
		sce::Gnm::kDataFormatInvalid,			// P4                         , // 34 - 4-bit palettized (not supported on consoles)
		sce::Gnm::kDataFormatInvalid,			// P8                         , // 35 - 8-bit palettized (not supported on consoles)
		sce::Gnm::kDataFormatInvalid,			// A8P8                       , // 36 - 16-bit palettized with 8-bit alpha (not supported on consoles)
		sce::Gnm::kDataFormatR8Unorm,			// R8                         , // 37 - non-standard R001 format (8 bits per channel)
		sce::Gnm::kDataFormatR16Unorm,			// R16                        , // 38 - non-standard R001 format (16 bits per channel)
		sce::Gnm::kDataFormatR8G8Unorm,			// G8R8                       , // 39 - non-standard RG01 format (8 bits per channel)
		sce::Gnm::kDataFormatInvalid,			// LINA32B32G32R32F_DEPRECATED, // 40
		sce::Gnm::kDataFormatInvalid,			// LINA8R8G8B8_DEPRECATED     , // 41
		sce::Gnm::kDataFormatInvalid,			// LIN8_DEPRECATED            , // 42
		sce::Gnm::kDataFormatInvalid,			// RGBE                       , // 43
	};
	static_assert(ARRAYSIZE(grcTextureGNMFormats::grcImage_to_Orbis) == 46);

	grcTextureFormat Orbis_to_grctf(sce::Gnm::DataFormat orbis)
	{
		for (int i = 1; i < grctfCount; i++) {
			if (orbis.m_asInt == grcTextureGNMFormats::grctf_to_Orbis[i].m_asInt)
				return (grcTextureFormat)i;
		}
		return grctfNULL;
	}
}

#define GNM_TEXTURE GetUserMemory()->m_GnmTexture

sce::Gnm::SizeAlign rage::grcTextureGNM::InitGnmTexture()
{
	sce::Gnm::SizeAlign result;
	sce::Gnm::DataFormat format = grcTextureGNMFormats::grctf_to_Orbis[ConvertTogrcFormat((GRC_TEMP_XG_FORMAT)m_Texture.Format)];

	if (GetDepth() > 1)
		result = GetGnmTexture().initAs3d(GetWidth(), GetHeight(), GetDepth(), GetMipMapCount(), format, (sce::Gnm::TileMode)m_Texture.Location);
	else
		result = GetGnmTexture().initAs2d(GetWidth(), GetHeight(), GetMipMapCount(), format, (sce::Gnm::TileMode)m_Texture.Location, sce::Gnm::kNumFragments1);

	m_CachedTexturePtr = &GetGnmTexture();
	return result;
}

rage::grcTextureGNM::grcTextureGNM(u32 width, u32 height, u32 format, void* pBuffer, TextureCreateParams* params)
{

}

rage::grcTextureGNM::grcTextureGNM(u32 width, u32 height, u32 depth, u32 format, void* pBuffer,
	TextureCreateParams* params)
{
}

rage::grcTextureGNM::grcTextureGNM(const datResource& rsc) : grcOrbisDurangoTextureBase(rsc)
{
	sce::Gnm::SizeAlign result = InitGnmTexture();
	u64 pAligned = (u64)m_pGraphicsMem + ((u64)result.m_align - 1) & ~((u64)result.m_align - 1);
	AM_ASSERT(pAligned == (u64)m_pGraphicsMem, "grcTextureGNM::grcTextureGNM(datResource& rsc) -> Bad alignment encountered.");

	GetGnmTexture().setBaseAddress256ByteBlocks((u64)m_pGraphicsMem >> 8);
}

bool rage::grcTextureGNM::LockRect(int layer, int mipLevel, grcTextureLock& lock, grcLockFlags lockFlags)
{
	AM_ASSERTS(layer == 0);
	AM_ASSERTS(mipLevel < GetMipMapCount());

	lock.BitsPerPixel = GetBitsPerPixel();
	lock.Width = GetWidth() >> mipLevel;
	lock.Height = GetHeight() >> mipLevel;
	lock.MipLevel = mipLevel;
	lock.Layer = 0;
	lock.RawFormat = GetGnmTexture().getDataFormat().m_asInt;

	size_t offset = 0;
	size_t pitch = 0;

	if (GetDepth() == 1)
	{
		GetLockInfo(mipLevel, offset, pitch);
	}
	else
	{
		for (int i = 0; i <= mipLevel; i++)
		{
			sce::GpuAddress::SurfaceInfo surfaceInfo;
			sce::GpuAddress::TilingParameters tilingParams;
			tilingParams.initFromTexture(&GetGnmTexture(), i, 0);
			sce::GpuAddress::computeSurfaceInfo(&surfaceInfo, &tilingParams);
			pitch = (surfaceInfo.m_pitch * surfaceInfo.m_bitsPerElement) >> 3;

			if (i != mipLevel)
				offset += (lock.Pitch * surfaceInfo.m_height * surfaceInfo.m_depth);
		}
	}
	lock.Base = (char*)m_pGraphicsMem + offset;
	lock.Pitch = pitch;

	return true;
}

void rage::grcTextureGNM::UnlockRect(grcTextureLock& lock)
{
	// Does nothing, yes, really.
}
