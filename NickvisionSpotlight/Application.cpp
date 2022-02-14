#include "Application.h"
#include <wx/notifmsg.h>
#include "Views/SplashScreen.h"

namespace NickvisionSpotlight
{
	wxIMPLEMENT_APP(Application);

	using namespace NickvisionSpotlight::Views;
	using namespace NickvisionSpotlight::Controls;

	Application::Application()
	{
		
	}

	Application::~Application()
	{

	}

	bool Application::OnInit()
	{
		m_singleInstance = new wxSingleInstanceChecker();
		if (m_singleInstance->IsAnotherRunning())
		{
			wxNotificationMessage notifAlreadyRunning("Spotlight Already Running!", "Nickvision Spotlight is already running in the background.\nPlease right click the icon in the system tray to open the main window.", nullptr, wxICON_WARNING);
			notifAlreadyRunning.Show();
			delete m_singleInstance;
			m_singleInstance = nullptr;
			return false;
		}
		wxInitAllImageHandlers();
		m_mainWindow = new MainWindow();
		m_taskBarIcon = new TaskBarIcon(m_mainWindow);
		SplashScreen splash(m_mainWindow);
		splash.ShowModal();
		m_mainWindow->Show();
		return true;
	}

	int Application::OnExit()
	{
		delete m_singleInstance;
		return 0;
	}
}