//
// File: drawable.h
//
// Copyright (C) 2023-2024 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include "lightattr.h"
#include "am/graphics/outlinerender.h"
#include "am/ui/extensions.h"
#include "rage/paging/template/array.h"
#include "rage/physics/bounds/boundbase.h"
#include "rage/rmc/drawable.h"

class gtaDrawable : public rage::rmcDrawable
{
	rage::pgArray<CLightAttr>		m_Lights;
	rage::pgUPtr<rage::pgArray<u8>> m_TintData;
	rage::phBoundPtr				m_Bound;

public:
	gtaDrawable() = default;
	// ReSharper disable once CppPossiblyUninitializedMember
	gtaDrawable(const rage::datResource& rsc) : rmcDrawable(rsc) {}
	gtaDrawable(const gtaDrawable& other) : rmcDrawable(other) {}
	~gtaDrawable() override {}

	u16 GetLightCount() const { return m_Lights.GetSize(); }
	CLightAttr* GetLight(u16 index) { return &m_Lights[index]; }
	CLightAttr& AddLight() { return m_Lights.Construct(); }
	rage::atArray<CLightAttr>& GetLightArray() { return *(rage::atArray<CLightAttr>*)&m_Lights; }

	void SetBound(const rage::phBoundPtr& bound) { m_Bound = bound; }
	const rage::phBoundPtr& GetBound() const { return m_Bound; }

	void Draw(const rage::Mat34V& mtx, rage::grcDrawMask mask, rage::eDrawableLod lod) override
	{
		rmcDrawable::Draw(mtx, mask, lod);

		if (mask.Match(1 << rage::RB_MODEL_OUTLINE))
		{
			auto outlineRender = rageam::graphics::OutlineRender::GetInstance();
			outlineRender->Begin();
			rmcDrawable::Draw(mtx, mask, lod);
			outlineRender->End();
		}
	}

	void DrawSkinned(const rage::Mat34V& mtx, u64 a3, rage::grcDrawMask mask, rage::eDrawableLod lod) override
	{
		rmcDrawable::DrawSkinned(mtx, a3, mask, lod);

		if (mask.Match(1 << rage::RB_MODEL_OUTLINE))
		{
			auto outlineRender = rageam::graphics::OutlineRender::GetInstance();
			outlineRender->Begin();
			rmcDrawable::DrawSkinned(mtx, a3, mask, lod);
			outlineRender->End();
		}
	}
};

using gtaDrawablePtr = rage::pgCountedPtr<gtaDrawable>;
