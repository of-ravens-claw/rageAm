#include "texture.h"

#include "texturegnm.h"

rage::grcTexture::grcTexture(eTextureType type, ConstString name)
{
	m_Texture = {};
	m_Name = name;
	m_RefCount = 1;
	m_ResourceTypeAndConversionFlags = type;
	m_PhysicalSizeAndTemplateType = 0;
	m_LayerCount = 0;
	m_HandleIndex = 0;
}

rage::grcTexture::grcTexture(const grcTexture& other) : pgBase(other)
{
	m_Texture = other.m_Texture;
	m_Name = other.m_Name;
	m_ResourceTypeAndConversionFlags = other.m_ResourceTypeAndConversionFlags;
	m_LayerCount = other.m_LayerCount;
	m_PhysicalSizeAndTemplateType = other.m_PhysicalSizeAndTemplateType;
	m_HandleIndex = other.m_HandleIndex;

	if (pgRscCompiler::GetCurrent())
		m_RefCount = other.m_RefCount;
	else
		m_RefCount = 1;
}

// ReSharper disable once CppPossiblyUninitializedMember
rage::grcTexture::grcTexture(const datResource& rsc)
{

}

rage::grcTexture* rage::grcTexture::Place(const datResource& rsc, grcTexture* that)
{
	AM_ASSERT(that->GetResourceType() == TEXTURE_NORMAL, "grcTexture::Place() -> Only normal texture type is currently supported!");
	return new (that) grcTextureGNM(rsc);
}

rage::grcTexture* rage::grcTexture::Snapshot(pgSnapshotAllocator* allocator, grcTexture* from)
{
	AM_ASSERTS(from);
	AM_ASSERTS(from->GetResourceType() == TEXTURE_NORMAL);
	pVoid block = allocator->Allocate(sizeof grcTextureGNM);
	return new (block) grcTextureGNM(*reinterpret_cast<grcTextureGNM*>(from));
}
