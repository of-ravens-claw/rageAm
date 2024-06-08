//
// File: assettypes.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include "common/types.h"
#include "am/system/asserts.h"

using strAssetID = u32;

enum eStrAssetId
{
	STR_ASSET_ID_RPF = 0,
	STR_ASSET_ID_IDR = 1,
	STR_ASSET_ID_IFT = 2,
	STR_ASSET_ID_IDD = 3,
	STR_ASSET_ID_ITD = 4,
	STR_ASSET_ID_ICD = 5,
	STR_ASSET_ID_IBN = 6,
	STR_ASSET_ID_IBS = 8,
	STR_ASSET_ID_ILD = 9,
	STR_ASSET_ID_IPM = 10,
	STR_ASSET_ID_IED = 11,
	STR_ASSET_ID_IPT = 12,
	STR_ASSET_ID_IMT = 13,
	STR_ASSET_ID_IMAP = 14,
	STR_ASSET_ID_IPDB = 15,
	STR_ASSET_ID_MRF = 16,
	STR_ASSET_ID_INV = 17,
	STR_ASSET_ID_IHN = 18,
	STR_ASSET_ID_ISC = 19,
	STR_ASSET_ID_CUT = 20,
	STR_ASSET_ID_GFX = 21,
	STR_ASSET_ID_IPL = 22,
	STR_ASSET_ID_IAM = 23,
	STR_ASSET_ID_IND = 24,
	STR_ASSET_ID_NOD = 25,
	STR_ASSET_ID_IVR = 26,
	STR_ASSET_ID_IWR = 27,
	STR_ASSET_ID_IMF = 28,
	STR_ASSET_ID_ITYP = 29,
	STR_ASSET_ID_INH = 30,
	STR_ASSET_ID_IFD = 31,
	STR_ASSET_ID_ILDB = 32
};

namespace rage
{
	inline const char* GetPlatformAssetExtensionByID(strAssetID id)
	{
		if (id == 0) return "rpf";
		if (id == 1) return "odr";
		if (id == 2) return "oft";
		if (id == 3) return "odd";
		if (id == 4) return "otd";
		if (id == 5) return "ocd";
		if (id == 6) return "obn";
		if (id == 8) return "obs";
		if (id == 9) return "old";
		if (id == 10) return "opm";
		if (id == 11) return "oed";
		if (id == 12) return "opt";
		if (id == 13) return "omt";
		if (id == 14) return "omap";
		if (id == 15) return "opdb";
		if (id == 16) return "mrf";
		if (id == 17) return "onv";
		if (id == 18) return "ohn";
		if (id == 19) return "osc";
		if (id == 20) return "cut";
		if (id == 21) return "gfx";
		if (id == 22) return "opl";
		if (id == 23) return "oam";
		if (id == 24) return "ond";
		if (id == 25) return "nod";
		if (id == 26) return "ovr";
		if (id == 27) return "owr";
		if (id == 28) return "omf";
		if (id == 29) return "otyp";
		if (id == 30) return "onh";
		if (id == 31) return "ofd";
		if (id == 32) return "oldb";
		AM_UNREACHABLE("GetPlatformAssetExtensionByID() -> ID %u is unknown!", id);
	}
}
