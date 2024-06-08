#include "imglue.h"

#include "am/graphics/render.h"
#include "am/graphics/window.h"
#include "am/system/datamgr.h"
#include "am/file/iterator.h"
#include "am/system/system.h"
#include "imgui_impl_dx11.h"
#include "font_icons/icons_awesome.h"

#include "am/asset/ui/assetasynccompiler.h"
#include "am/uiapps/skeleton.h"
#include "am/uiapps/scene.h"

#ifdef AM_TESTBED
#include "am/integration/script/core.h"
#include "am/uiapps/testbed.h"
#else
#include "am/uiapps/explorer/explorer.h"
#endif

#include <backends/imgui_impl_win32.h>
#include <implot.h>
#include <imgui_internal.h>
#include <misc/freetype/imgui_freetype.h>
#include <easy/profiler.h>

#include "am/uiapps/explorer/explorer.h"

void ImGui::PushFont(ImFonts font)
{
	ImGuiIO& io = GetIO();
	PushFont(io.Fonts->Fonts[font]);
}

void rageam::ui::ImGlue::CreateContext() const
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	io.ConfigDockingWithShift = true;

	// Disable 'alt-tab'-ish window selector
	// - It crashes game
	// - Conflicts with debug build hotkey to change keyboard mode
	GImGui->ConfigNavWindowingKeyNext = 0;
	GImGui->ConfigNavWindowingKeyPrev = 0;

	ImGui_ImplWin32_Init(graphics::WindowGetHWND());
	ImGui_ImplDX11_Init(graphics::RenderGetDevice(), graphics::RenderGetContext());
	ImPlot::CreateContext();

	ImPlotStyle& plotStyle = ImPlot::GetStyle();
	plotStyle.Use24HourClock = true;
	plotStyle.UseLocalTime = true;
}

void rageam::ui::ImGlue::DestroyContext() const
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();
}

u32 rageam::ui::ImGlue::GetThemeAccentColor() const
{
	return ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
}

void rageam::ui::ImGlue::EmbedFontIcons(ImFonts font) const
{
	ImGuiIO& io = ImGui::GetIO();

	ImFont* customIconFont = io.Fonts->Fonts[font];

	const auto& fontIcons = DataManager::GetFontIconsFolder();
	file::Iterator iconIterator(fontIcons / L"*.*");
	file::FindData findData;

	// A little bit bigger than font size
	int glyphSize = ALIGN((int)customIconFont->FontSize, 2);

	graphics::ImageFactory::tl_ImagePreferredSvgWidth = glyphSize;
	graphics::ImageFactory::tl_ImagePreferredSvgHeight = glyphSize;

	// Cached file names to not ping file system twice
	rage::atArray<file::WPath> imageFiles;

	// 1: Layout pass - allocate atlas for every texture
	ImWchar customIconBegin = 0xE000;
	rage::atArray<int> iconIds;
	while (iconIterator.Next())
	{
		iconIterator.GetCurrent(findData);

		if (!graphics::ImageFactory::IsSupportedImageFormat(findData.Path))
			continue;

		imageFiles.Add(findData.Path);

		graphics::ImagePtr image = graphics::ImageFactory::LoadFromPath(findData.Path, true);
		if (!image)
			continue;

		ImWchar iconId = customIconBegin + iconIds.GetSize(); // Unicode ID
		iconIds.Add(io.Fonts->AddCustomRectFontGlyph(
			customIconFont, iconId, glyphSize, glyphSize, static_cast<float>(glyphSize)));
	}

	// 2: Finish layout and build atlas
	io.Fonts->Build();

	// 3: Copy texture pixel data to pixel rectangle on atlas
	unsigned char* atlasPixels = nullptr;
	int atlastWidth, atlasHeight;
	io.Fonts->GetTexDataAsRGBA32(&atlasPixels, &atlastWidth, &atlasHeight);

	graphics::ColorU32 baseColor = GetThemeAccentColor();
	float bgL = ColorGetLuminosity(baseColor);

	u16 iconId = 0;
	for (file::WPath& path : imageFiles)
	{
		graphics::ImagePtr image = graphics::ImageFactory::LoadFromPath(path);
		if (!image)
			continue;

		image = image->ConvertPixelFormat(graphics::ImagePixelFormat_U32);
		image = image->Resize(glyphSize, glyphSize, graphics::ResizeFilter_Box);

		const ImFontAtlasCustomRect* rect = io.Fonts->GetCustomRectByIndex(iconIds[iconId++]);

		ImmutableWString imageFileName = file::GetFileName(path.GetCStr());
		bool adjustColors = !imageFileName.StartsWith('_'); // We use '_' prefix to indicate that image colors needs to be used as is

		graphics::ColorU32* imagePixel = reinterpret_cast<graphics::ColorU32*>(image->GetPixelData().Data());

		// Write pixels to atlas region from png icon
		for (int y = 0; y < image->GetHeight(); y++)
		{
			ImU32* atlasPixel = reinterpret_cast<ImU32*>(atlasPixels) + (rect->Y + y) * atlastWidth + rect->X;
			for (int x = 0; x < image->GetWidth(); x++)
			{
				graphics::ColorU32 imagePixelColor = *imagePixel;
				if (adjustColors)
					imagePixelColor = ColorTransformLuminosity(imagePixelColor, bgL);
				*atlasPixel = imagePixelColor;
				imagePixel++;
				atlasPixel++;
			}
		}
	}
}

void rageam::ui::ImGlue::BuildFontRanges(List<ImWchar>& fontRanges) const
{
#define FONT_ADD_RANGE(from, to) fontRanges.Add(from); fontRanges.Add(to)

	FONT_ADD_RANGE(0x0020, 0x00FF); // Basic Latin + Latin Supplement

	if (ExtraFontRanges & ImExtraFontRanges_Cyrillic)
	{
		FONT_ADD_RANGE(0x0400, 0x052F); // Cyrillic + Cyrillic Supplement
		FONT_ADD_RANGE(0x2DE0, 0x2DFF); // Cyrillic Extended-A
		FONT_ADD_RANGE(0xA640, 0xA69F); // Cyrillic Extended-B
	}

	if (ExtraFontRanges & ImExtraFontRanges_Chinese)
	{
		FONT_ADD_RANGE(0x2000, 0x206F); // General Punctuation
		FONT_ADD_RANGE(0x3000, 0x30FF); // CJK Symbols and Punctuations, Hiragana, Katakana
		FONT_ADD_RANGE(0x31F0, 0x31FF); // Katakana Phonetic Extensions
		FONT_ADD_RANGE(0xFF00, 0xFFEF); // Half-width characters
		FONT_ADD_RANGE(0xFFFD, 0xFFFD); // Invalid
		FONT_ADD_RANGE(0x4e00, 0x9FAF); // CJK Ideograms
	}

	if (ExtraFontRanges & ImExtraFontRanges_Japanese)
	{
		FONT_ADD_RANGE(0x3000, 0x30FF); // CJK Symbols and Punctuations, Hiragana, Katakana
		FONT_ADD_RANGE(0x31F0, 0x31FF); // Katakana Phonetic Extensions
		FONT_ADD_RANGE(0xFF00, 0xFFEF); // Half-width characters
	}

	if (ExtraFontRanges & ImExtraFontRanges_Greek)
	{
		FONT_ADD_RANGE(0x0370, 0x03FF); // Greek and Coptic
	}

	if (ExtraFontRanges & ImExtraFontRanges_Thai)
	{
		FONT_ADD_RANGE(0x2010, 0x205E); // Punctuations
		FONT_ADD_RANGE(0x0E00, 0x0E7F); // Thai
	}

	if (ExtraFontRanges & ImExtraFontRanges_Korean)
	{
		FONT_ADD_RANGE(0x3131, 0x3163); // Korean alphabets
		FONT_ADD_RANGE(0xAC00, 0xD7A3); // Korean characters
		FONT_ADD_RANGE(0xFFFD, 0xFFFD); // Invalid
	}

	if (ExtraFontRanges & ImExtraFontRanges_Vietnamese)
	{
		FONT_ADD_RANGE(0x0102, 0x0103);
		FONT_ADD_RANGE(0x0110, 0x0111);
		FONT_ADD_RANGE(0x0128, 0x0129);
		FONT_ADD_RANGE(0x0168, 0x0169);
		FONT_ADD_RANGE(0x01A0, 0x01A1);
		FONT_ADD_RANGE(0x01AF, 0x01B0);
		FONT_ADD_RANGE(0x1EA0, 0x1EF9);
	}

	// Terminator
	fontRanges.Add(0);

#undef FONT_ADD_RANGE
}

void rageam::ui::ImGlue::CreateDefaultFonts(const ImWchar* fontRanges, float fontScaleMultiplier) const
{
	float fontSize = static_cast<float>(FontSize);

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->FontBuilderIO = ImGuiFreeType::GetBuilderForFreeType();
	io.Fonts->FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint;

	ImFontConfig fontConfig = {};
	fontConfig.SizePixels = fontSize * fontScaleMultiplier;
	fontConfig.OversampleH = 1;
	fontConfig.OversampleV = 1;

	ImFontConfig iconConfig{};
	iconConfig.MergeMode = true;
	iconConfig.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor;

	static constexpr ImWchar iconRange[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

	file::U8Path regular = PATH_TO_UTF8(DataManager::GetFontsFolder() / L"NotoSans.ttf");
	file::U8Path medium = PATH_TO_UTF8(DataManager::GetFontsFolder() / L"NotoSansMedium.ttf");
	file::U8Path awesome = PATH_TO_UTF8(DataManager::GetFontsFolder() / L"Font Awesome 6.ttf");
	io.Fonts->AddFontFromFileTTF(regular, fontSize * fontScaleMultiplier, &fontConfig, fontRanges);		// Regular
	io.Fonts->AddFontFromFileTTF(awesome, fontSize * fontScaleMultiplier, &iconConfig, iconRange);		// Awesome icons (merged into regular)
	io.Fonts->AddFontFromFileTTF(regular, fontSize * fontScaleMultiplier - 2, &fontConfig, fontRanges);	// Small
	io.Fonts->AddFontFromFileTTF(medium, fontSize * fontScaleMultiplier, &fontConfig, fontRanges);		// Medium
}

void rageam::ui::ImGlue::CreateBankFont() const
{
	// Font has 8x8 127 characters, each byte represents one pixel line
	static u8 s_FontBitmap[] = // 1016 bytes = 8 * 127
	{
		0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x10, 0x38,
		0x7C, 0xFE, 0x38, 0x38, 0x38, 0x00, 0x38, 0x38, 0x38, 0xFE,
		0x7C, 0x38, 0x10, 0x00, 0x10, 0x18, 0xFC, 0xFF, 0xFC, 0x18,
		0x10, 0x00, 0x08, 0x10, 0x78, 0xCC, 0xFC, 0xC0, 0x7C, 0x00,
		0x30, 0x48, 0x78, 0xCC, 0xFC, 0xC0, 0x7C, 0x00, 0x76, 0xDC,
		0x00, 0xDC, 0x66, 0x66, 0x66, 0x00, 0x08, 0x10, 0x38, 0x0C,
		0x7C, 0xCC, 0x76, 0x00, 0x24, 0x00, 0x3C, 0x66, 0x66, 0x66,
		0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x24, 0x00, 0x38, 0x0C, 0x7C, 0xCC, 0x76, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03,
		0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x0C, 0x0C, 0x0C, 0x0C,
		0x0C, 0x0C, 0x0C, 0x0C, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
		0x0F, 0x0F, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
		0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3C, 0x3C,
		0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3F, 0x3F, 0x3F, 0x3F,
		0x3F, 0x3F, 0x3F, 0x3F, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
		0xC0, 0xC0, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3,
		0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCF, 0xCF,
		0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xCF, 0xF0, 0xF0, 0xF0, 0xF0,
		0xF0, 0xF0, 0xF0, 0xF0, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3, 0xF3,
		0xF3, 0xF3, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xFC,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18,
		0x18, 0x00, 0x18, 0x00, 0xCC, 0xCC, 0xCC, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x36, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0xD8, 0x00,
		0x18, 0x7E, 0xC0, 0x7C, 0x06, 0xFC, 0x30, 0x00, 0xC2, 0xC6,
		0x0C, 0x18, 0x30, 0x66, 0xC6, 0x00, 0x38, 0x6C, 0x38, 0x70,
		0xDE, 0xCC, 0x76, 0x00, 0x30, 0x30, 0x60, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00,
		0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00, 0x00, 0x6C,
		0x38, 0xFE, 0x38, 0x6C, 0x00, 0x00, 0x00, 0x18, 0x18, 0x7E,
		0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18,
		0x18, 0x30, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x02, 0x06,
		0x0C, 0x18, 0x30, 0x60, 0xC0, 0x00, 0x7C, 0xCE, 0xDE, 0xF6,
		0xE6, 0xC6, 0x7C, 0x00, 0x18, 0x38, 0x18, 0x18, 0x18, 0x18,
		0x7E, 0x00, 0x7C, 0xC6, 0x06, 0x1C, 0x70, 0xC6, 0xFE, 0x00,
		0x7C, 0xC6, 0x06, 0x1C, 0x06, 0xC6, 0x7C, 0x00, 0x1C, 0x3C,
		0x6C, 0xCC, 0xFE, 0x0C, 0x0C, 0x00, 0xFE, 0xC0, 0xFC, 0x06,
		0x06, 0xC6, 0x7C, 0x00, 0x3C, 0x60, 0xC0, 0xFC, 0xC6, 0xC6,
		0x7C, 0x00, 0xFE, 0xC6, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00,
		0x7C, 0xC6, 0xC6, 0x7C, 0xC6, 0xC6, 0x7C, 0x00, 0x7C, 0xC6,
		0xC6, 0x7E, 0x06, 0x0C, 0x78, 0x00, 0x00, 0x18, 0x18, 0x00,
		0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18,
		0x18, 0x30, 0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x00,
		0x00, 0x00, 0x7E, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x30, 0x18,
		0x0C, 0x06, 0x0C, 0x18, 0x30, 0x00, 0x3C, 0x66, 0x06, 0x0C,
		0x18, 0x00, 0x18, 0x00, 0x7C, 0xC6, 0xDE, 0xDE, 0xDE, 0xC0,
		0x7C, 0x00, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0xC6, 0xC6, 0x00,
		0xFC, 0x6E, 0x66, 0x7C, 0x66, 0x6E, 0xFC, 0x00, 0x3E, 0x62,
		0xC0, 0xC0, 0xC0, 0x62, 0x3E, 0x00, 0xF8, 0x6E, 0x66, 0x66,
		0x66, 0x6E, 0xF8, 0x00, 0xFE, 0x62, 0x60, 0x78, 0x60, 0x62,
		0xFE, 0x00, 0xFE, 0x62, 0x60, 0x78, 0x60, 0x60, 0xF0, 0x00,
		0x3E, 0x62, 0xC0, 0xC0, 0xCE, 0x66, 0x3E, 0x00, 0xC6, 0xC6,
		0xC6, 0xFE, 0xC6, 0xC6, 0xC6, 0x00, 0x3C, 0x18, 0x18, 0x18,
		0x18, 0x18, 0x3C, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0xCC,
		0x78, 0x00, 0xE6, 0x66, 0x6C, 0x78, 0x78, 0x6C, 0xE6, 0x00,
		0xF0, 0x60, 0x60, 0x60, 0x60, 0x66, 0xFE, 0x00, 0xC6, 0xEE,
		0xFE, 0xD6, 0xC6, 0xC6, 0xC6, 0x00, 0xC6, 0xE6, 0xF6, 0xFE,
		0xDE, 0xCE, 0xC6, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6,
		0x7C, 0x00, 0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xE0, 0x00,
		0x7C, 0xC6, 0xC6, 0xD6, 0xDE, 0x7C, 0x06, 0x00, 0xFC, 0x66,
		0x66, 0x7C, 0x78, 0x6C, 0xE6, 0x00, 0x7C, 0xC6, 0xE0, 0x38,
		0x0E, 0xC6, 0x7C, 0x00, 0x7E, 0x5A, 0x18, 0x18, 0x18, 0x18,
		0x3C, 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00,
		0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00, 0xC6, 0xC6,
		0xC6, 0xD6, 0xFE, 0xFE, 0xC6, 0x00, 0xC6, 0x6C, 0x38, 0x38,
		0x6C, 0xC6, 0xC6, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x18,
		0x3C, 0x00, 0xFE, 0xCC, 0x18, 0x30, 0x60, 0xC6, 0xFE, 0x00,
		0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00, 0x80, 0xC0,
		0x60, 0x30, 0x18, 0x0C, 0x06, 0x00, 0x3C, 0x0C, 0x0C, 0x0C,
		0x0C, 0x0C, 0x3C, 0x00, 0x18, 0x3C, 0x66, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0x18, 0x18, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x38, 0x0C, 0x7C, 0xCC, 0x76, 0x00, 0x60, 0x60, 0x60, 0x7C,
		0x66, 0x66, 0xDC, 0x00, 0x00, 0x00, 0x7C, 0xC4, 0xC0, 0xC4,
		0x7C, 0x00, 0x0C, 0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00,
		0x00, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x7C, 0x00, 0x38, 0x6C,
		0x60, 0xF8, 0x60, 0x60, 0xE0, 0x00, 0x00, 0x00, 0x76, 0xCC,
		0xCC, 0x7C, 0x0C, 0x7C, 0xE0, 0x60, 0x7C, 0x66, 0x66, 0x66,
		0x66, 0x00, 0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x38, 0x00,
		0x0C, 0x00, 0x1C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0xE0, 0x60,
		0x66, 0x6C, 0x78, 0x6C, 0x66, 0x00, 0x70, 0x30, 0x30, 0x30,
		0x30, 0x30, 0x38, 0x00, 0x00, 0x00, 0xCC, 0xFE, 0xD6, 0xC6,
		0xC6, 0x00, 0x00, 0x00, 0xDC, 0x66, 0x66, 0x66, 0x66, 0x00,
		0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00, 0x00, 0x00,
		0xDC, 0x66, 0x66, 0x7C, 0x60, 0xE0, 0x00, 0x00, 0x76, 0xCC,
		0xCC, 0x7C, 0x0C, 0x0E, 0x00, 0x00, 0xDC, 0x76, 0x60, 0x60,
		0x60, 0x00, 0x00, 0x00, 0x78, 0xC0, 0x78, 0x0C, 0x78, 0x00,
		0x10, 0x30, 0x78, 0x30, 0x30, 0x34, 0x18, 0x00, 0x00, 0x00,
		0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 0x00, 0x00, 0x66, 0x66,
		0x66, 0x3C, 0x18, 0x00, 0x00, 0x00, 0xC6, 0xC6, 0xD6, 0xFE,
		0x6C, 0x00, 0x00, 0x00, 0xCC, 0x78, 0x30, 0x78, 0xCC, 0x00,
		0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0x7C, 0x00, 0x00,
		0xFC, 0x18, 0x30, 0x60, 0xFC, 0x00, 0x0E, 0x18, 0x18, 0x70,
		0x18, 0x18, 0x0E, 0x00, 0x30, 0x30, 0x30, 0x00, 0x30, 0x30,
		0x30, 0x00, 0x70, 0x18, 0x18, 0x0E, 0x18, 0x18, 0x70, 0x00,
		0x00, 0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA,
		0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA
	};

	ImGuiIO& io = ImGui::GetIO();

	// We add this dummy font and then overwrite pixels with ours
	ImFontConfig fontConfig = {};
	fontConfig.SizePixels = static_cast<float>(FontSize);
	ImFont* font = io.Fonts->AddFontDefault(&fontConfig);

	// Build font atlas
	ImWchar fontIDs[127];
	for (int i = 0; i < 127; i++)
	{
		int id = i + 1; // Starts from 1 (null character is excluded...)
		fontIDs[i] = io.Fonts->AddCustomRectFontGlyph(font, id, FontSize, FontSize, static_cast<float>(FontSize));
	}
	io.Fonts->Build();

	graphics::PixelDataOwner resizedGlyph = graphics::PixelDataOwner::AllocateForImage(
		FontSize, FontSize, graphics::ImagePixelFormat_U32);

	// Convert bitmap encoded font to RGBA and write glyphs to font atlas
	unsigned char* atlasPixels = nullptr;
	int atlastWidth, atlasHeight;
	io.Fonts->GetTexDataAsRGBA32(&atlasPixels, &atlastWidth, &atlasHeight);
	for (int i = 0; i < 127; i++)
	{
		const ImFontAtlasCustomRect* rect = io.Fonts->GetCustomRectByIndex(fontIDs[i]);

		// Decode pixels in 8x8 U32 array
		ImU32 sourceGlyph[8][8];
		for (int y = 0; y < 8; y++)
		{
			u8 row = s_FontBitmap[i * 8 + y];
			for (int x = 0; x < 8; x++)
			{
				int bit = 7 - x; // Inverted
				// Each bit represents pixel in the row
				u32 color = row & 1 << bit ? 0xFFFFFFFF : 0;
				sourceGlyph[y][x] = color;
			}
		}

		// Resize glyph to font size
		ImageResize(resizedGlyph.Data(), sourceGlyph, graphics::ResizeFilter_Point, graphics::ImagePixelFormat_U32,
			8, 8, FontSize, FontSize, true);

		for (int y = 0; y < FontSize; y++)
		{
			ImU32* atlasRow = reinterpret_cast<ImU32*>(atlasPixels) + (rect->Y + y) * atlastWidth + rect->X;
			char* glyphRow = resizedGlyph.Data()->Bytes + y * FontSize * sizeof u32;
			memcpy(atlasRow, glyphRow, sizeof(u32) * FontSize);
		}
	}
}

void rageam::ui::ImGlue::CreateFonts() const
{
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Clear();

	HMONITOR monitor = MonitorFromWindow(graphics::WindowGetHWND(), MONITOR_DEFAULTTONEAREST);
	float fontScale = ImGui_ImplWin32_GetDpiScaleForMonitor(monitor);

	List<ImWchar> fontRanges;
	BuildFontRanges(fontRanges);
	CreateDefaultFonts(fontRanges.GetItems(), fontScale);
	CreateBankFont();
	EmbedFontIcons(ImFont_Regular);
	
	ImGui_ImplDX11_InvalidateDeviceObjects();
	ImGui_ImplDX11_CreateDeviceObjects();
}

void rageam::ui::ImGlue::RegisterSystemApps()
{
	Windows = new WindowManager();
	AddApp(Windows);

	AddApp(new Skeleton());

#ifdef AM_TESTBED
	AddApp(new TestbedApp());
#endif
	Windows->Add(new Explorer());

	AddApp(new AssetAsyncCompiler());
}

void rageam::ui::ImGlue::LoadIcons()
{
	file::Iterator it(DataManager::GetIconsFolder() / L"*.*");
	file::FindData entry;
	while (it.Next())
	{
		it.GetCurrent(entry);

		HashValue nameHash = Hash(entry.Path.GetFileNameWithoutExtension());

		ImImage& image = m_Icons.ConstructAt(nameHash);
		image.Load(entry.Path, UI_MAX_ICON_RESLUTION);
	}
}

void rageam::ui::ImGlue::StyleVS() const
{
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowMenuButtonPosition = ImGuiDir_Right;
	style.Colors[ImGuiCol_MenuBarBg] = { 0, 0, 0, 0 };
	style.Colors[ImGuiCol_ResizeGrip] = { 0, 0, 0, 0 };
	style.Colors[ImGuiCol_Button] = { 0, 0, 0, 0 };
	style.Colors[ImGuiCol_ButtonHovered] = ImGui::ColorConvertU32ToFloat4(0x783D3D3D);
	style.Colors[ImGuiCol_ButtonActive] = ImGui::ColorConvertU32ToFloat4(0x782E2E2E);
	style.Colors[ImGuiCol_Tab] = style.Colors[ImGuiCol_FrameBg]; // TODO: Move to SlGui
}

rageam::ui::ImGlue::ImGlue()
{
	System* sys = System::GetInstance();
	if (sys->HasData)
	{
		FontSize = sys->Data.UI.FontSize;
		m_OldFontSize = FontSize;
	}

	m_NotFoundImage.Set(graphics::ImageFactory::CreateChecker_NotFound(256, 4));

	CreateContext();
	CreateFonts();
	LoadIcons();

	RegisterSystemApps();
}

rageam::ui::ImGlue::~ImGlue()
{
	System* sys = System::GetInstance();
	sys->Data.UI.FontSize = FontSize;

	DestroyContext();
}

bool rageam::ui::ImGlue::BeginFrame()
{
	EASY_BLOCK("ImGlue::BeginFrame");
	std::unique_lock lock(m_Mutex);

#ifdef AM_INTEGRATED
	if (m_BeganFrame) // No response from render thread... skip frame
		return false;
#endif

	// This will destroy all pending textures
	m_TexturesToDestroy.Clear();

	// Don't process UI anymore... we've got assertion before
	if (ImGetAssertWasThrown())
		return false;

	// Font scale was changed during previous frame,
	// must be re-created before new frame
	u32 accentThemeColor = GetThemeAccentColor();
	if (m_OldFontSize != FontSize ||
		m_OldExtraFontRanges != ExtraFontRanges ||
		m_OldAccentColor != accentThemeColor)
	{
		m_OldFontSize = FontSize;
		m_OldExtraFontRanges = ExtraFontRanges;
		m_OldAccentColor = accentThemeColor;
		CreateFonts();
	}

	if (m_IsFirstFrame || m_OldStyle != Style)
	{
		switch (Style)
		{
		case ImStyle_VS:		StyleVS();						break;
		case ImStyle_Dark:		ImGui::StyleColorsDark();		break;
		case ImStyle_Light:		ImGui::StyleColorsLight();		break;
		case ImStyle_Classic:	ImGui::StyleColorsClassic();	break;
		default:
			AM_UNREACHABLE("Style '%s' is not implemented!", Enum::GetName(Style));
		}
		m_OldStyle = Style;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

#ifdef AM_INTEGRATED
	m_BeganFrame = true;
#endif

	return true;
}

bool rageam::ui::ImGlue::UpdateApps()
{
	EASY_BLOCK("ImGlue::UpdateApps");
	std::unique_lock lock(m_Mutex);

	Timer timer = Timer::StartNew();

	ImGuiStyle& style = ImGui::GetStyle();
	style.IndentSpacing = GImGui->FontSize * 0.9f;

	// Execute all apps
	bool onlyUpdate = !IsVisible;
	bool isDisabled = IsDisabled;
	if (isDisabled) ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); // TODO: Window still can be dragged
	AM_INTEGRATED_ONLY(scrBegin());
	// Place it before updating ui::WindowManager so new scene window can be added this frame
	Scene::TryOpenPendingSceneWindow();
	for (amUniquePtr<App>& app : m_Apps)
	{
		m_LastUpdatedApp = app.get();
		EASY_BLOCK(app->GetDebugName(), profiler::colors::color(0, 126, 52));
		app->Tick(onlyUpdate);
	}
	AM_INTEGRATED_ONLY(scrEnd());
	if (isDisabled) ImGui::PopItemFlag();

	// We really shouldn't render this
	if (ImGetAssertWasThrown())
	{
		AM_ERRF("ImGlue::BuildDrawList() -> UI crashed and will stop updating!");
		return false;
	}

	timer.Stop();

	m_LastUpdatedApp = nullptr;
	m_IsFirstFrame = false;
	LastUpdateTimer = timer;

	return true;
}

void rageam::ui::ImGlue::EndFrame() AM_STANDALONE_ONLY(const)
{
	EASY_BLOCK("ImGlue::EndFrame");
	std::unique_lock lock(m_Mutex);

	if (ImGetAssertWasThrown())
		return;

	ImGuiIO& io = ImGui::GetIO();

#ifdef AM_INTEGRATED
	// Render existing frame...
	if (!m_BeganFrame)
	{
		ImDrawData* drawData = ImGui::GetDrawData();
		if (drawData)
		{
			ImGui_ImplDX11_RenderDrawData(drawData);
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				ImGui::RenderPlatformWindowsDefault();
		}
		return;
	}

	m_BeganFrame = false;
#endif

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void rageam::ui::ImGlue::AddNoLongerNeededTexture(const amComPtr<ID3D11ShaderResourceView>& view)
{
	m_TexturesToDestroy.Add(view);
}

void rageam::ui::ImGlue::AddApp(App* app)
{
	std::unique_lock lock(m_Mutex);
	m_Apps.Construct(app);
}

void rageam::ui::ImGlue::KillAllApps()
{
	std::unique_lock lock(m_Mutex);
	m_Apps.Destroy();
}

rageam::ui::ImImage* rageam::ui::ImGlue::GetIcon(ConstString name) const
{
	ImImage* image = m_Icons.TryGetAt(Hash(name));
	if (image)
		return image;

	return nullptr;
}
