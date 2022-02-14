#pragma once

#include <wx/wx.h>
#include <wx/snglinst.h>
#include "Views/MainWindow.h"
#include "Controls/TaskBarIcon.h"

namespace NickvisionSpotlight
{
	class Application : public wxApp
	{
	public:
		Application();
		~Application();
		virtual bool OnInit();
		virtual int OnExit();

	private:
		wxSingleInstanceChecker* m_singleInstance = nullptr;
		Views::MainWindow* m_mainWindow = nullptr;
		Controls::TaskBarIcon* m_taskBarIcon = nullptr;
	};
}

