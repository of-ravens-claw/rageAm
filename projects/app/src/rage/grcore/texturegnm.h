// File: texturegnm.h
//
// Copyright (C) 2023-2024 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include "texture.h"

namespace rage
{
	enum GRC_TEMP_XG_FORMAT
	{
		GRC_TEMP_XG_FORMAT_UNKNOWN = 0,
		GRC_TEMP_XG_FORMAT_R32G32B32A32_TYPELESS = 1,
		GRC_TEMP_XG_FORMAT_R32G32B32A32_FLOAT = 2,
		GRC_TEMP_XG_FORMAT_R32G32B32A32_UINT = 3,
		GRC_TEMP_XG_FORMAT_R32G32B32A32_SINT = 4,
		GRC_TEMP_XG_FORMAT_R32G32B32_TYPELESS = 5,
		GRC_TEMP_XG_FORMAT_R32G32B32_FLOAT = 6,
		GRC_TEMP_XG_FORMAT_R32G32B32_UINT = 7,
		GRC_TEMP_XG_FORMAT_R32G32B32_SINT = 8,
		GRC_TEMP_XG_FORMAT_R16G16B16A16_TYPELESS = 9,
		GRC_TEMP_XG_FORMAT_R16G16B16A16_FLOAT = 10,
		GRC_TEMP_XG_FORMAT_R16G16B16A16_UNORM = 11,
		GRC_TEMP_XG_FORMAT_R16G16B16A16_UINT = 12,
		GRC_TEMP_XG_FORMAT_R16G16B16A16_SNORM = 13,
		GRC_TEMP_XG_FORMAT_R16G16B16A16_SINT = 14,
		GRC_TEMP_XG_FORMAT_R32G32_TYPELESS = 15,
		GRC_TEMP_XG_FORMAT_R32G32_FLOAT = 16,
		GRC_TEMP_XG_FORMAT_R32G32_UINT = 17,
		GRC_TEMP_XG_FORMAT_R32G32_SINT = 18,
		GRC_TEMP_XG_FORMAT_R32G8X24_TYPELESS = 19,
		GRC_TEMP_XG_FORMAT_D32_FLOAT_S8X24_UINT = 20,
		GRC_TEMP_XG_FORMAT_R32_FLOAT_X8X24_TYPELESS = 21,
		GRC_TEMP_XG_FORMAT_X32_TYPELESS_G8X24_UINT = 22,
		GRC_TEMP_XG_FORMAT_R10G10B10A2_TYPELESS = 23,
		GRC_TEMP_XG_FORMAT_R10G10B10A2_UNORM = 24,
		GRC_TEMP_XG_FORMAT_R10G10B10A2_UINT = 25,
		GRC_TEMP_XG_FORMAT_R11G11B10_FLOAT = 26,
		GRC_TEMP_XG_FORMAT_R8G8B8A8_TYPELESS = 27,
		GRC_TEMP_XG_FORMAT_R8G8B8A8_UNORM = 28,
		GRC_TEMP_XG_FORMAT_R8G8B8A8_UNORM_SRGB = 29,
		GRC_TEMP_XG_FORMAT_R8G8B8A8_UINT = 30,
		GRC_TEMP_XG_FORMAT_R8G8B8A8_SNORM = 31,
		GRC_TEMP_XG_FORMAT_R8G8B8A8_SINT = 32,
		GRC_TEMP_XG_FORMAT_R16G16_TYPELESS = 33,
		GRC_TEMP_XG_FORMAT_R16G16_FLOAT = 34,
		GRC_TEMP_XG_FORMAT_R16G16_UNORM = 35,
		GRC_TEMP_XG_FORMAT_R16G16_UINT = 36,
		GRC_TEMP_XG_FORMAT_R16G16_SNORM = 37,
		GRC_TEMP_XG_FORMAT_R16G16_SINT = 38,
		GRC_TEMP_XG_FORMAT_R32_TYPELESS = 39,
		GRC_TEMP_XG_FORMAT_D32_FLOAT = 40,
		GRC_TEMP_XG_FORMAT_R32_FLOAT = 41,
		GRC_TEMP_XG_FORMAT_R32_UINT = 42,
		GRC_TEMP_XG_FORMAT_R32_SINT = 43,
		GRC_TEMP_XG_FORMAT_R24G8_TYPELESS = 44,
		GRC_TEMP_XG_FORMAT_D24_UNORM_S8_UINT = 45,
		GRC_TEMP_XG_FORMAT_R24_UNORM_X8_TYPELESS = 46,
		GRC_TEMP_XG_FORMAT_X24_TYPELESS_G8_UINT = 47,
		GRC_TEMP_XG_FORMAT_R8G8_TYPELESS = 48,
		GRC_TEMP_XG_FORMAT_R8G8_UNORM = 49,
		GRC_TEMP_XG_FORMAT_R8G8_UINT = 50,
		GRC_TEMP_XG_FORMAT_R8G8_SNORM = 51,
		GRC_TEMP_XG_FORMAT_R8G8_SINT = 52,
		GRC_TEMP_XG_FORMAT_R16_TYPELESS = 53,
		GRC_TEMP_XG_FORMAT_R16_FLOAT = 54,
		GRC_TEMP_XG_FORMAT_D16_UNORM = 55,
		GRC_TEMP_XG_FORMAT_R16_UNORM = 56,
		GRC_TEMP_XG_FORMAT_R16_UINT = 57,
		GRC_TEMP_XG_FORMAT_R16_SNORM = 58,
		GRC_TEMP_XG_FORMAT_R16_SINT = 59,
		GRC_TEMP_XG_FORMAT_R8_TYPELESS = 60,
		GRC_TEMP_XG_FORMAT_R8_UNORM = 61,
		GRC_TEMP_XG_FORMAT_R8_UINT = 62,
		GRC_TEMP_XG_FORMAT_R8_SNORM = 63,
		GRC_TEMP_XG_FORMAT_R8_SINT = 64,
		GRC_TEMP_XG_FORMAT_A8_UNORM = 65,
		GRC_TEMP_XG_FORMAT_R1_UNORM = 66,
		GRC_TEMP_XG_FORMAT_R9G9B9E5_SHAREDEXP = 67,
		GRC_TEMP_XG_FORMAT_R8G8_B8G8_UNORM = 68,
		GRC_TEMP_XG_FORMAT_G8R8_G8B8_UNORM = 69,
		GRC_TEMP_XG_FORMAT_BC1_TYPELESS = 70,
		GRC_TEMP_XG_FORMAT_BC1_UNORM = 71,
		GRC_TEMP_XG_FORMAT_BC1_UNORM_SRGB = 72,
		GRC_TEMP_XG_FORMAT_BC2_TYPELESS = 73,
		GRC_TEMP_XG_FORMAT_BC2_UNORM = 74,
		GRC_TEMP_XG_FORMAT_BC2_UNORM_SRGB = 75,
		GRC_TEMP_XG_FORMAT_BC3_TYPELESS = 76,
		GRC_TEMP_XG_FORMAT_BC3_UNORM = 77,
		GRC_TEMP_XG_FORMAT_BC3_UNORM_SRGB = 78,
		GRC_TEMP_XG_FORMAT_BC4_TYPELESS = 79,
		GRC_TEMP_XG_FORMAT_BC4_UNORM = 80,
		GRC_TEMP_XG_FORMAT_BC4_SNORM = 81,
		GRC_TEMP_XG_FORMAT_BC5_TYPELESS = 82,
		GRC_TEMP_XG_FORMAT_BC5_UNORM = 83,
		GRC_TEMP_XG_FORMAT_BC5_SNORM = 84,
		GRC_TEMP_XG_FORMAT_B5G6R5_UNORM = 85,
		GRC_TEMP_XG_FORMAT_B5G5R5A1_UNORM = 86,
		GRC_TEMP_XG_FORMAT_B8G8R8A8_UNORM = 87,
		GRC_TEMP_XG_FORMAT_B8G8R8X8_UNORM = 88,
		GRC_TEMP_XG_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 89,
		GRC_TEMP_XG_FORMAT_B8G8R8A8_TYPELESS = 90,
		GRC_TEMP_XG_FORMAT_B8G8R8A8_UNORM_SRGB = 91,
		GRC_TEMP_XG_FORMAT_B8G8R8X8_TYPELESS = 92,
		GRC_TEMP_XG_FORMAT_B8G8R8X8_UNORM_SRGB = 93,
		GRC_TEMP_XG_FORMAT_BC6H_TYPELESS = 94,
		GRC_TEMP_XG_FORMAT_BC6H_UF16 = 95,
		GRC_TEMP_XG_FORMAT_BC6H_SF16 = 96,
		GRC_TEMP_XG_FORMAT_BC7_TYPELESS = 97,
		GRC_TEMP_XG_FORMAT_BC7_UNORM = 98,
		GRC_TEMP_XG_FORMAT_BC7_UNORM_SRGB = 99,
		GRC_TEMP_XG_FORMAT_AYUV = 100,
		GRC_TEMP_XG_FORMAT_Y410 = 101,
		GRC_TEMP_XG_FORMAT_Y416 = 102,
		GRC_TEMP_XG_FORMAT_NV12 = 103,
		GRC_TEMP_XG_FORMAT_P010 = 104,
		GRC_TEMP_XG_FORMAT_P016 = 105,
		GRC_TEMP_XG_FORMAT_420_OPAQUE = 106,
		GRC_TEMP_XG_FORMAT_YUY2 = 107,
		GRC_TEMP_XG_FORMAT_Y210 = 108,
		GRC_TEMP_XG_FORMAT_Y216 = 109,
		GRC_TEMP_XG_FORMAT_NV11 = 110,
		GRC_TEMP_XG_FORMAT_AI44 = 111,
		GRC_TEMP_XG_FORMAT_IA44 = 112,
		GRC_TEMP_XG_FORMAT_P8 = 113,
		GRC_TEMP_XG_FORMAT_A8P8 = 114,
		GRC_TEMP_XG_FORMAT_B4G4R4A4_UNORM = 115,
		GRC_TEMP_XG_FORMAT_R10G10B10_7E3_A2_FLOAT = 116,
		GRC_TEMP_XG_FORMAT_R10G10B10_6E4_A2_FLOAT = 117,
		GRC_TEMP_XG_FORMAT_FORCE_UINT = 0xffffffff
	};

	enum ImageType : u8
	{
		IMAGE_TYPE_STANDARD,
		IMAGE_TUBE_CUBE,
		IMAGE_TYPE_DEPTH,
		IMAGE_TYPE_VOLUME,
	};

	struct GRC_ORBIS_DURANGO_TEXTURE_DESC
	{
		GRC_ORBIS_DURANGO_TEXTURE_DESC()
		{
			memset(this, 0, sizeof(GRC_ORBIS_DURANGO_TEXTURE_DESC));
		}

		u16 m_Width;
		u16 m_Height;
		u8 m_Depth;
		u8 m_ArrayDimension;
		u8 m_NoOfMips;
		GRC_TEMP_XG_FORMAT m_XGFormat;
		sce::Gnm::TileMode m_TileMode;
		u16 m_ImageType; // ImageType.
		u16 m_ExtraBindFlags; // grcBindType.
	};

	class grcOrbisDurangoTextureBase : public grcTexture
	{
	public:
		struct OFFSET_AND_PITCH
		{
			void Set(u64 offset, u64 pitch)
			{
				AM_ASSERT((pitch & ~(((u64)1 << 20) - (u64)1)) == 0, "grcOrbisDurangoTextureBase::OFFSET_AND_PITCH::Set()...Pitch too big (Max 20 bits)\n");
				AM_ASSERT((offset & ~(((u64)1 << 43) - (u64)1)) == 0, "grcOrbisDurangoTextureBase::OFFSET_AND_PITCH::Set()...Offset too big (Max 43 bits)\n");

				offsetAndPitch = offset << 20 | pitch;
			}

			void Get(u64& offset, u64& pitch)
			{
				offset = offsetAndPitch >> 20;
				pitch = offsetAndPitch & (1 << 20) - 1;
			}

			u64 offsetAndPitch;
		};

	protected:
		void* m_pGraphicsMem;
		size_t m_GraphicsMemorySize;
		OFFSET_AND_PITCH* m_pLockInfoPtr;
		u64 m_UserMemory[6];

	public:
		grcOrbisDurangoTextureBase();
		grcOrbisDurangoTextureBase(eTextureType type);
		grcOrbisDurangoTextureBase(GRC_ORBIS_DURANGO_TEXTURE_DESC& info, eTextureType type);
		grcOrbisDurangoTextureBase(const grcOrbisDurangoTextureBase& other) = default;
		grcOrbisDurangoTextureBase(const datResource& rsc);

		~grcOrbisDurangoTextureBase() override = default;

		void SetFromDescription(GRC_ORBIS_DURANGO_TEXTURE_DESC& desc);
		void SetLockInfo(u32 Mip, u64 offset, u64 pitch);
		void GetLockInfo(u32 Mip, u64& offset, u64& pitch) const;
		bool GetOwnsAllocations() const;
		void SetUsesPreAllocatedMemory(bool uses);
		bool GetUsesPreAllocatedMemory() const;

		u16 GetWidth() const override { return m_Texture.Width; }
		u16 GetHeight() const override { return m_Texture.Height; }
		u16 GetDepth() const override { return m_Texture.Depth; }
		u8 GetMipMapCount() const override { return m_Texture.Mipmap; }
		u8 GetArraySize() const override { return m_Texture.Dimension; }
		u32 GetImageFormat() const override { return m_Texture.Cubemap; }

		bool Copy(const grcImage* pImage) override;
		u8 GetBitsPerPixel() const;
		atFixedBitSet<8, u8> FindUsedChannels() const override;
		int GetStride(u32 uMipLevel) const;
		int GetRowCount(u32 uMipLevel) const;

		bool Copy2D(const void* pSrc, const grcPoint& oSrcDim, const grcRect& oDstRect, const grcTextureLock& lock, s32 iMipLevel);
		void EnsureGpuWritable();
	};

	using TextureCreateParams = void;
	struct grcTextureGNMFormats
	{
		static sce::Gnm::DataFormat grctf_to_Orbis[];
		static sce::Gnm::DataFormat grcImage_to_Orbis[];
	};

	class grcTextureGNM : public grcOrbisDurangoTextureBase
	{
		struct USER_MEMORY
		{
			sce::Gnm::Texture	m_GnmTexture;
			uint64_t			m_Unused[2];
		};
		static_assert(sizeof(USER_MEMORY) <= (sizeof(u64) * 6)); // 8 * 6 = 48

		USER_MEMORY* GetUserMemory() { return (USER_MEMORY*)&m_UserMemory; }
		sce::Gnm::Texture& GetGnmTexture() { return GetUserMemory()->m_GnmTexture; }

		void RecalculatePitch();
		void Create(u32 width, u32 height, u32 depth, GRC_TEMP_XG_FORMAT format, TextureCreateParams* params);
		sce::Gnm::SizeAlign InitGnmTexture();
		void ComputeMipOffsets();
		void CopyBits(int mip, const void* bits);

	public:
		grcTextureGNM(u32 width, u32 height, u32 format, void* pBuffer, TextureCreateParams* params);
		grcTextureGNM(u32 width, u32 height, u32 depth, u32 format, void* pBuffer, TextureCreateParams* params);
		grcTextureGNM(const grcTextureGNM& other) = default;
		grcTextureGNM(const datResource& rsc);
		~grcTextureGNM() override = default;

		void* GetTextureView() const override { return nullptr; } // Only PC and Durango have this
		bool LockRect(int layer, int mipLevel, grcTextureLock& lock, grcLockFlags lockFlags) override;
		void UnlockRect(grcTextureLock& lock) override;
	};
}