#include "Application.h"
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
		wxInitAllImageHandlers();
		m_mainWindow = new MainWindow();
		m_taskBarIcon = new TaskBarIcon(m_mainWindow);
		SplashScreen splash(m_mainWindow);
		splash.ShowModal();
		m_mainWindow->Show();
		return true;
	}
}