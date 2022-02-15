#include "MainWindow.h"
#include <fstream>
#include <wx/busyinfo.h>
#include "../Models/Configuration.h"
#include "../Helpers/ThemeHelpers.h"
#include "SettingsDialog.h"

namespace NickvisionSpotlight::Views
{
	using namespace NickvisionSpotlight::Models;
	using namespace NickvisionSpotlight::Models::Update;
	using namespace NickvisionSpotlight::Helpers;
	using namespace NickvisionSpotlight::Controls;

	MainWindow::MainWindow() : wxFrame(nullptr, IDs::WINDOW, "Nickvision Spotlight", wxDefaultPosition, wxSize(800, 600)), m_isLightTheme(false), m_updater("https://raw.githubusercontent.com/nlogozzo/NickvisionSpotlight/main/UpdateConfig.json", { "2022.2.0" })
	{
		//==Window Settings==//
		SetIcon(wxICON(APP_ICON));
		Maximize();
		Connect(IDs::WINDOW, wxEVT_CLOSE_WINDOW, wxCloseEventHandler(MainWindow::OnClose));
		//==Menu==//
		m_menuBar = new wxMenuBar();
		//File
		m_menuFile = new wxMenu();
		m_menuFile->Append(IDs::MENU_SAVE_IMAGE, _("&Save Image\tCtrl+S"));
		Connect(IDs::MENU_SAVE_IMAGE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::SaveImage));
		m_menuFile->Append(IDs::MENU_SAVE_ALL_IMAGES, _("Save &All Images\tCtrl+Shift+S"));
		Connect(IDs::MENU_SAVE_ALL_IMAGES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::SaveAllImages));
		m_menuFile->AppendSeparator();
		m_menuFile->Append(IDs::MENU_EXIT, _("E&xit\tAlt+F4"));
		Connect(IDs::MENU_EXIT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::Exit));
		m_menuBar->Append(m_menuFile, _("&File"));
		//Edit
		m_menuEdit = new wxMenu();
		m_menuEdit->Append(IDs::MENU_SETTINGS, _("&Settings\tCtrl+."));
		Connect(IDs::MENU_SETTINGS, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::Settings));
		m_menuBar->Append(m_menuEdit, _("&Edit"));
		//Spotlight
		m_menuSpotlight = new wxMenu();
		m_menuSpotlight->Append(IDs::MENU_SYNC_SPOTLIGHT_IMAGES, _("&Sync Spotlight Images\tCtrl+Shift+D"));
		Connect(IDs::MENU_SYNC_SPOTLIGHT_IMAGES, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::SyncSpotlightImages));
		m_menuSpotlight->Append(IDs::MENU_SET_AS_BACKGROUND, _("Set as &Background\tCtrl+Shift+B"));
		Connect(IDs::MENU_SET_AS_BACKGROUND, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::SetAsBackground));
		m_menuBar->Append(m_menuSpotlight, _("&Spotlight"));
		//Help
		m_menuHelp = new wxMenu();
		m_menuHelp->Append(IDs::MENU_UPDATE, _("&Update"));
		Connect(IDs::MENU_UPDATE, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::Update));
		m_menuHelp->AppendSeparator();
		m_menuHelp->Append(IDs::MENU_GITHUB_REPO, _("&GitHub Repo"));
		Connect(IDs::MENU_GITHUB_REPO, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::GitHubRepo));
		m_menuHelp->Append(IDs::MENU_REPORT_A_BUG, _("&Report a Bug"));
		Connect(IDs::MENU_REPORT_A_BUG, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::ReportABug));
		m_menuHelp->AppendSeparator();
		m_menuHelp->Append(IDs::MENU_CHANGELOG, _("&Changelog"));
		Connect(IDs::MENU_CHANGELOG, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::Changelog));
		m_menuHelp->Append(IDs::MENU_ABOUT, _("&About Spotlight\tF1"));
		Connect(IDs::MENU_ABOUT, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::About));
		m_menuBar->Append(m_menuHelp, _("&Help"));
		SetMenuBar(m_menuBar);
		//==ToolBar==//
		m_toolBar = new wxToolBar(this, IDs::TOOLBAR, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL);
		m_toolBar->AddTool(IDs::TOOL_SAVE_IMAGE, "", wxBitmap("SAVE", wxBITMAP_TYPE_PNG_RESOURCE))->SetShortHelp(_("Save Image"));
		Connect(IDs::TOOL_SAVE_IMAGE, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindow::SaveImage));
		m_toolBar->AddSeparator();
		m_toolBar->AddTool(IDs::TOOL_SET_AS_BACKGROUND, "", wxBitmap("MONITOR", wxBITMAP_TYPE_PNG_RESOURCE))->SetShortHelp(_("Set as Background"));
		Connect(IDs::TOOL_SET_AS_BACKGROUND, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindow::SetAsBackground));
		m_toolBar->AddStretchableSpace();
		m_toolBar->AddTool(IDs::TOOL_SETTINGS, "", wxBitmap("SETTINGS", wxBITMAP_TYPE_PNG_RESOURCE))->SetShortHelp(_("Settings"));
		Connect(IDs::TOOL_SETTINGS, wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(MainWindow::Settings));
		m_toolBar->Realize();
		SetToolBar(m_toolBar);
		//==StatusBar==//
		m_statusBar = new StatusBar(this, IDs::STATUSBAR);
		SetStatusBar(m_statusBar);
		//==InfoBar==//
		m_infoBar = new InfoBar(this, IDs::INFOBAR);
		//==List Images==//
		m_listImages = new wxListBox(this, IDs::LIST_IMAGES, wxDefaultPosition, wxSize(300, -1), { }, wxBORDER_NONE | wxLB_SINGLE | wxLB_NEEDED_SB | wxLB_HSCROLL);
		Connect(IDs::LIST_IMAGES, wxEVT_LISTBOX, wxCommandEventHandler(MainWindow::ListImages_SelectionChanged));
		//==Bitmap Image==//
		m_scrollImage = new wxScrolledWindow(this, IDs::SCROLL_IMAGE);
		m_btmpImage = new wxStaticBitmap(m_scrollImage, IDs::BTMP_IMAGE, wxBitmap(1, 1));
		m_scrollImage->SetScrollRate(4, 4);
		//==Box Image==//
		m_boxImage = new wxBoxSizer(wxHORIZONTAL);
		m_boxImage->Add(m_listImages, 0, wxEXPAND | wxALL, 6);
		m_boxImage->Add(m_scrollImage, 1, wxEXPAND | wxALL, 6);
		//==Layout==//
		m_mainBox = new wxBoxSizer(wxVERTICAL);
		m_mainBox->Add(m_infoBar, 0, wxEXPAND);
		m_mainBox->Add(m_boxImage, 1, wxEXPAND | wxALL, 6);
		SetSizer(m_mainBox);
	}

	void MainWindow::SetIsLightTheme(bool isLightTheme)
	{
		m_isLightTheme = isLightTheme;
		if (m_isLightTheme)
		{
			//Win32
			ThemeHelpers::ApplyWin32LightMode(this);
			ThemeHelpers::ApplyWin32LightMode(m_listImages);
			ThemeHelpers::ApplyWin32LightMode(m_scrollImage);
			//Window
			SetBackgroundColour(ThemeHelpers::GetMainLightColor());
			//ToolBar
			m_toolBar->SetBackgroundColour(ThemeHelpers::GetSecondaryLightColor());
			m_toolBar->SetForegroundColour(*wxBLACK);
			//StatusBar
			m_statusBar->SetIsLightTheme(true);
			//InfoBar
			m_infoBar->SetIsLightTheme(true);
			//Images
			m_listImages->SetBackgroundColour(ThemeHelpers::GetSecondaryLightColor());
			m_listImages->SetForegroundColour(*wxBLACK);
			m_scrollImage->SetBackgroundColour(ThemeHelpers::GetSecondaryLightColor());
		}
		else
		{
			//Win32
			ThemeHelpers::ApplyWin32DarkMode(this);
			ThemeHelpers::ApplyWin32DarkMode(m_listImages);
			ThemeHelpers::ApplyWin32DarkMode(m_scrollImage);
			//Window
			SetBackgroundColour(ThemeHelpers::GetMainDarkColor());
			//ToolBar
			m_toolBar->SetBackgroundColour(ThemeHelpers::GetSecondaryDarkColor());
			m_toolBar->SetForegroundColour(*wxWHITE);
			//StatusBar
			m_statusBar->SetIsLightTheme(false);
			//InfoBar
			m_infoBar->SetIsLightTheme(false);
			//Images
			m_listImages->SetBackgroundColour(ThemeHelpers::GetSecondaryDarkColor());
			m_listImages->SetForegroundColour(*wxWHITE);
			m_scrollImage->SetBackgroundColour(ThemeHelpers::GetSecondaryDarkColor());
		}
		m_infoBar->Dismiss();
		Refresh();
	}

	void MainWindow::LoadConfig()
	{
		Configuration configuration;
		SetIsLightTheme(configuration.PreferLightTheme());
	}

	void MainWindow::SyncSpotlightImages()
	{
		m_spotlightManager.SyncSpotlightImages();
		m_listImages->Clear();
		m_btmpImage->SetBitmap({ 1, 1 });
		for (const std::filesystem::path& path : m_spotlightManager.GetSpotlightImages())
		{
			m_listImages->AppendString(path.filename().string());
		}
		m_statusBar->SetMessage(_("Total Number of Images: " + std::to_string(m_spotlightManager.GetSpotlightImages().size())));
	}

	void MainWindow::OnClose(wxCloseEvent& WXUNUSED(event))
	{
		//==Save Config==//
		Configuration configuration;
		configuration.Save();
		//==Hide Instead==//
		Hide();
	}

	void MainWindow::SaveImage(wxCommandEvent& WXUNUSED(event))
	{
		if (m_listImages->GetSelection() != -1)
		{
			wxFileDialog saveImageDialog(this, _("Save Image"), "", "", "JPEG (*.jpg)|*.jpg", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			if (saveImageDialog.ShowModal() == wxID_OK)
			{
				m_spotlightManager.ExportImage(m_listImages->GetSelection(), saveImageDialog.GetPath().ToStdString());
				m_infoBar->ShowMessage(_("Image Saved To: " + saveImageDialog.GetPath()), wxICON_INFORMATION);
			}
		}
	}

	void MainWindow::SaveAllImages(wxCommandEvent& WXUNUSED(event))
	{
		wxDirDialog selectFolderDialog(this, _("Select Folder"), "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
		if (selectFolderDialog.ShowModal() == wxID_OK)
		{
			wxBusyInfo busyExporting(wxBusyInfoFlags()
				.Parent(this)
				.Title(_("<b>Please Wait</b>"))
				.Text(_("Exporting all images..."))
				.Background(m_isLightTheme ? *wxWHITE : *wxBLACK)
				.Foreground(m_isLightTheme ? *wxBLACK : *wxWHITE)
				.Transparency(4 * wxALPHA_OPAQUE / 5));
			m_spotlightManager.ExportAllImages(selectFolderDialog.GetPath().ToStdString() + "\\");
			busyExporting.~wxBusyInfo();
			m_infoBar->ShowMessage(_("Images Saved To: " + selectFolderDialog.GetPath()), wxICON_INFORMATION);
		}
	}

	void MainWindow::Exit(wxCommandEvent& WXUNUSED(event))
	{
		Close();
	}

	void MainWindow::Settings(wxCommandEvent& WXUNUSED(event))
	{
		SettingsDialog settingsDialog(this, m_isLightTheme);
		settingsDialog.ShowModal();
		Configuration configuration;
		if (configuration.PreferLightTheme() != m_isLightTheme)
		{
			SetIsLightTheme(configuration.PreferLightTheme());
		}
	}

	void MainWindow::SyncSpotlightImages(wxCommandEvent& WXUNUSED(event))
	{
		wxBusyInfo busySyncing(wxBusyInfoFlags()
			.Parent(this)
			.Title(_("<b>Please Wait</b>"))
			.Text(_("Syncing spotlight images..."))
			.Background(m_isLightTheme ? *wxWHITE : *wxBLACK)
			.Foreground(m_isLightTheme ? *wxBLACK : *wxWHITE)
			.Transparency(4 * wxALPHA_OPAQUE / 5));
		SyncSpotlightImages();
		busySyncing.~wxBusyInfo();
	}

	void MainWindow::SetAsBackground(wxCommandEvent& WXUNUSED(event))
	{
		if (m_listImages->GetSelection() != -1)
		{
			m_spotlightManager.SetImageAsBackground(m_listImages->GetSelection());
		}
	}

	void MainWindow::CheckForUpdates()
	{
		m_updater.CheckForUpdates();
		if (m_updater.UpdateAvailable())
		{
			m_infoBar->ShowMessage(_("There is an update available. Please run the update command in the help menu to download and install the update."), wxICON_INFORMATION);
		}
	}

	void MainWindow::Update(wxCommandEvent& WXUNUSED(event))
	{
		if (m_updater.UpdateAvailable())
		{
			int result = wxMessageBox(_("Update Available\n\n===V" + m_updater.GetLatestVersion()->ToString() + " Changelog===\n" + m_updater.GetChangelog() + "\n\nNickvisionSpotlight will automatically download and install the update. Please save all work before continuing. Are you ready to update?"), _("Update"), wxICON_INFORMATION | wxYES_NO, this);
			if (result == wxYES)
			{
				bool updateSuccess = false;
				wxBusyInfo busyUpdating(wxBusyInfoFlags()
					.Parent(this)
					.Title(_("<b>Please Wait</b>"))
					.Text(_("Downloading and installing the update..."))
					.Background(m_isLightTheme ? *wxWHITE : *wxBLACK)
					.Foreground(m_isLightTheme ? *wxBLACK : *wxWHITE)
					.Transparency(4 * wxALPHA_OPAQUE / 5));
				updateSuccess = m_updater.Update();
				busyUpdating.~wxBusyInfo();
				if (!updateSuccess)
				{
					m_infoBar->ShowMessage(_("There was an error downloading and install the update. Please try again and if the error continues, file a bug report."), wxICON_ERROR);
				}
			}
		}
		else
		{
			m_infoBar->ShowMessage(_("There is no update available at this time. Please try again later."), wxICON_ERROR);
		}
	}

	void MainWindow::GitHubRepo(wxCommandEvent& WXUNUSED(event))
	{
		wxLaunchDefaultBrowser("https://github.com/nlogozzo/NickvisionSpotlight");
	}

	void MainWindow::ReportABug(wxCommandEvent& WXUNUSED(event))
	{
		wxLaunchDefaultBrowser("https://github.com/nlogozzo/NickvisionSpotlight/issues/new");
	}

	void MainWindow::Changelog(wxCommandEvent& WXUNUSED(event))
	{
		wxMessageBox(_("What's New?\n\n- Application rewrite with C++ and wxWidgets\n- Added the ability for Spotlight to run in the background to automatically set the desktop background to a random image, if the user chooses to do so\n- Spotlight only displays horizontal images instead of both horizontal and vertical wallpapers\n- Fixed an issue where not all Spotlight images were being shown\n\nNew in Alpha 4:\n- Wrapped spotlight image in ScrolledWindow\n- Added About entry to TaskBarIcon menu\n- Shortened already running message"), _("Changelog"), wxICON_INFORMATION, this);
	}

	void MainWindow::About(wxCommandEvent& WXUNUSED(event))
	{
		wxMessageBox(_("About Nickvision Spotlight\n\nVersion 2022.2.0-alpha4\nA utility for working with Windows Spotlight images.\n\nBuilt with C++, wxWidgets, and Icons8\n(C) Nickvision 2021-2022"), _("About"), wxICON_INFORMATION, this);
	}

	void MainWindow::ListImages_SelectionChanged(wxCommandEvent& WXUNUSED(event))
	{
		if (m_listImages->GetSelection() != -1)
		{
			m_btmpImage->SetBitmap(wxBitmap(m_spotlightManager.GetSpotlightImages()[m_listImages->GetSelection()].string(), wxBITMAP_TYPE_ANY));
			m_btmpImage->SetSize(m_btmpImage->GetBitmap().GetSize());
			m_scrollImage->SetVirtualSize(m_btmpImage->GetSize());
		}
	}
}