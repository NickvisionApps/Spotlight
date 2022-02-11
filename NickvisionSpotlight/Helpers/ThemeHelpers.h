#pragma once

#include <wx/wx.h>

namespace NickvisionSpotlight::Helpers::ThemeHelpers
{
	wxColor GetMainDarkColor();
	wxColor GetSecondaryDarkColor();
	wxColor GetMainLightColor();
	wxColor GetSecondaryLightColor();
	void ApplyWin32DarkMode(wxWindow* window);
}

