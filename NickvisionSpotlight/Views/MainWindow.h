#pragma once

#include <wx/wx.h>
#include "../Models/Update/Updater.h"
#include "../Models/SpotlightManager.h"
#include "../Controls/StatusBar.h"
#include "../Controls/InfoBar.h"

namespace NickvisionSpotlight::Views
{
	class MainWindow : public wxFrame
	{
	public:
		MainWindow();
		void CheckForUpdates();
		void SyncSpotlightImages();

	private:
		enum IDs
		{
			WINDOW = 100,
			MENU_SAVE_IMAGE,
			MENU_SAVE_ALL_IMAGES,
			MENU_EXIT,
			MENU_SETTINGS,
			MENU_SYNC_SPOTLIGHT_IMAGES,
			MENU_SET_AS_BACKGROUND,
			MENU_UPDATE,
			MENU_GITHUB_REPO,
			MENU_REPORT_A_BUG,
			MENU_CHANGELOG,
			MENU_ABOUT,
			TOOLBAR,
			STATUSBAR,
			INFOBAR,
			TOOL_SAVE_IMAGE,
			TOOL_SET_AS_BACKGROUND,
			TOOL_SETTINGS,
			LIST_IMAGES,
			IMG_SELECTED
		};
		bool m_isLightTheme;
		NickvisionSpotlight::Models::Update::Updater m_updater;
		NickvisionSpotlight::Models::SpotlightManager m_spotlightManager;
		//==Menu==//
		wxMenuBar* m_menuBar = nullptr;
		wxMenu* m_menuFile = nullptr;
		wxMenu* m_menuEdit = nullptr;
		wxMenu* m_menuSpotlight = nullptr;
		wxMenu* m_menuHelp = nullptr;
		//==ToolBar==//
		wxToolBar* m_toolBar = nullptr;
		//==StatusBar==//
		NickvisionSpotlight::Controls::StatusBar* m_statusBar = nullptr;
		//==Layout==//
		wxBoxSizer* m_mainBox = nullptr;
		NickvisionSpotlight::Controls::InfoBar* m_infoBar = nullptr;
		wxBoxSizer* m_boxImage = nullptr;
		wxListBox* m_listImages = nullptr;
		wxStaticBitmap* m_imgSelected = nullptr;
		//==Slots==//
		void OnClose(wxCloseEvent& event);
		void SaveImage(wxCommandEvent& event);
		void SaveAllImages(wxCommandEvent& event);
		void Exit(wxCommandEvent& event);
		void Settings(wxCommandEvent& event);
		void SyncSpotlightImages(wxCommandEvent& event);
		void SetAsBackground(wxCommandEvent& event);
		void Update(wxCommandEvent& event);
		void GitHubRepo(wxCommandEvent& event);
		void ReportABug(wxCommandEvent& event);
		void Changelog(wxCommandEvent& event);
		void About(wxCommandEvent& event);
		void ListImages_SelectionChanged(wxCommandEvent& event);
	};
}