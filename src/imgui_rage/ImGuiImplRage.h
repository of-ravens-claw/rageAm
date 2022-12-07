// dear imgui: Platform Backend for Rage 5 Rage

#pragma once
#include "imgui.h"      // IMGUI_IMPL_API

IMGUI_IMPL_API bool     ImGui_ImplRage_Init(void* hwnd);
IMGUI_IMPL_API void     ImGui_ImplRage_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplRage_NewFrame();

// Rage message handler your application need to call.
// - Intentionally commented out in a '#if 0' block to avoid dragging dependencies on <windows.h> from this helper.
// - You should COPY the line below into your .cpp code to forward declare the function and then you can call it.
// - Call from your application's message handler. Keep calling your message handler unless this function returns TRUE.

#if 0
extern IMGUI_IMPL_API LRESULT ImGui_ImplRage_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

// DPI-related helpers (optional)
// - Use to enable DPI awareness without having to create an application manifest.
// - Your own app may already do this via a manifest or explicit calls. This is mostly useful for our examples/ apps.
// - In theory we could call simple functions from Windows SDK such as SetProcessDPIAware(), SetProcessDpiAwareness(), etc.
//   but most of the functions provided by Microsoft require Windows 8.1/10+ SDK at compile time and Windows 8/10+ at runtime,
//   neither we want to require the user to have. So we dynamically select and load those functions to avoid dependencies.
IMGUI_IMPL_API void     ImGui_ImplRage_EnableDpiAwareness();
IMGUI_IMPL_API float    ImGui_ImplRage_GetDpiScaleForHwnd(void* hwnd);       // HWND hwnd
IMGUI_IMPL_API float    ImGui_ImplRage_GetDpiScaleForMonitor(void* monitor); // HMONITOR monitor