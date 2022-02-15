#include "TaskBarIcon.h"

namespace NickvisionSpotlight::Controls
{
	using namespace NickvisionSpotlight::Views;

	TaskBarIcon::TaskBarIcon(MainWindow* mainWindow) : m_mainWindow(mainWindow)
	{
		//==Settings==//
		SetIcon(wxICON(APP_ICON), "Nickvision Spotlight");
		Connect(wxEVT_TASKBAR_CLICK, wxTaskBarIconEventHandler(TaskBarIcon::Clicked));
		//==Menu==//
		m_menu = new wxMenu();
		m_menu->Append(IDs::MENU_OPEN_WINDOW, _("Open Window"));
		Connect(IDs::MENU_OPEN_WINDOW, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(TaskBarIcon::OpenWindow));
		m_menu->AppendSeparator();
		m_menu->Append(IDs::MENU_ABOUT, _("About"));
		Connect(IDs::MENU_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(TaskBarIcon::About));
		m_menu->Append(IDs::MENU_QUIT, _("Quit"));
		Connect(IDs::MENU_QUIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(TaskBarIcon::Quit));
	}

	void TaskBarIcon::Clicked(wxTaskBarIconEvent& WXUNUSED(event))
	{
		PopupMenu(m_menu);
	}

	void TaskBarIcon::OpenWindow(wxCommandEvent& event)
	{
		m_mainWindow->Show();
	}

	void TaskBarIcon::About(wxCommandEvent& event)
	{
		m_mainWindow->About(event);
	}

	void TaskBarIcon::Quit(wxCommandEvent& event)
	{
		Destroy();
		wxExit();
	}
}