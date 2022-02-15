#pragma once

#include <wx/wx.h>
#include <wx/taskbar.h>
#include "../Views/MainWindow.h"

namespace NickvisionSpotlight::Controls
{
	class TaskBarIcon : public wxTaskBarIcon
	{
	public:
		TaskBarIcon(NickvisionSpotlight::Views::MainWindow* mainWindow);

	private:
		enum IDs
		{
			MENU_OPEN_WINDOW = 700,
			MENU_ABOUT,
			MENU_QUIT
		};
		//==Menu==//
		NickvisionSpotlight::Views::MainWindow* m_mainWindow;
		wxMenu* m_menu = nullptr;
		//==Slots==//
		void Clicked(wxTaskBarIconEvent& event);
		void OpenWindow(wxCommandEvent& event);
		void About(wxCommandEvent& event);
		void Quit(wxCommandEvent& event);
	};
}

