#pragma once

#include <wx/wx.h>
#include <wx/treectrl.h>
#include "../Models/Configuration.h"

namespace NickvisionSpotlight::Views
{
	class SettingsDialog : public wxDialog
	{
	public:
		SettingsDialog(wxWindow* parent, bool isLightTheme);

	private:
		enum IDs
		{
			DIALOG = 300,
			TREE,
			PAGE_ROOT,
			LBL_ROOT,
			PAGE_UI,
			CHK_PREFER_LIGHT_THEME,
			PAGE_APP,
			BTN_SAVE
		};
		NickvisionSpotlight::Models::Configuration m_configuration;
		//==UI==//
		wxBoxSizer* m_mainBox = nullptr;
		wxBoxSizer* m_settingsBox = nullptr;
		//Tree
		wxTreeCtrl* m_mainTree = nullptr;
		wxTreeItemId m_treeNodeRoot;
		wxTreeItemId m_treeNodeUI;
		wxTreeItemId m_treeNodeApp;
		//Root Page
		wxPanel* m_pageRoot = nullptr;
		wxBoxSizer* m_boxRoot = nullptr;
		wxStaticText* m_lblRoot = nullptr;
		//UI Page
		wxPanel* m_pageUI = nullptr;
		wxBoxSizer* m_boxUI = nullptr;
		wxCheckBox* m_chkPreferLightTheme = nullptr;
		//App Page
		wxPanel* m_pageApp = nullptr;
		wxBoxSizer* m_boxApp = nullptr;
		//Save Button
		wxButton* m_btnSave = nullptr;
		//==Slots==//
		void OnClose(wxCloseEvent& event);
		void Save(wxCommandEvent& event);
		void TreeSelectionChanged(wxTreeEvent& event);
	};
}

