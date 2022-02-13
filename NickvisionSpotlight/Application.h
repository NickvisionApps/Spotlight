#pragma once

#include <wx/wx.h>
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

	private:
		Views::MainWindow* m_mainWindow = nullptr;
		Controls::TaskBarIcon* m_taskBarIcon = nullptr;
	};
}

