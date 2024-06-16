#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include <format>
#include <libnick/app/aura.h>
#include <libnick/helpers/codehelpers.h>
#include <libnick/helpers/stringhelpers.h>
#include <libnick/notifications/shellnotification.h>
#include <libnick/localization/gettext.h>
#include "SettingsPage.xaml.h"

using namespace ::Nickvision;
using namespace ::Nickvision::App;
using namespace ::Nickvision::Events;
using namespace ::Nickvision::Helpers;
using namespace ::Nickvision::Notifications;
using namespace ::Nickvision::Spotlight::Shared::Controllers;
using namespace ::Nickvision::Spotlight::Shared::Models;
using namespace winrt::Microsoft::UI;
using namespace winrt::Microsoft::UI::Dispatching;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Controls::Primitives;
using namespace winrt::Microsoft::UI::Xaml::Input;
using namespace winrt::Microsoft::UI::Xaml::Media;
using namespace winrt::Microsoft::UI::Xaml::Media::Imaging;
using namespace winrt::Microsoft::UI::Windowing;
using namespace winrt::Windows::ApplicationModel::DataTransfer;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Pickers;
using namespace winrt::Windows::System;

namespace winrt::Nickvision::Spotlight::WinUI::implementation 
{
    static std::vector<std::string> keys(const std::unordered_map<std::string, std::string>& m)
    {
        std::vector<std::string> k;
        for(std::unordered_map<std::string, std::string>::const_iterator it = m.begin(); it != m.end(); it++)
        {
            k.push_back(it->first);
        }
        return k;
    }

    MainWindow::MainWindow()
        : m_opened{ false },
        m_notificationClickToken{ 0 }
    {
        InitializeComponent();
        this->m_inner.as<::IWindowNative>()->get_WindowHandle(&m_hwnd);
        //Set TitleBar
        TitleBar().AppWindow(AppWindow());
        //Localize Strings
        NavViewImages().Content(winrt::box_value(winrt::to_hstring(_("Images"))));
        NavViewHelp().Content(winrt::box_value(winrt::to_hstring(_("Help"))));
        ToolTipService::SetToolTip(BtnCheckForUpdates(), winrt::box_value(winrt::to_hstring(_("Check for Updates"))));
        ToolTipService::SetToolTip(BtnCredits(), winrt::box_value(winrt::to_hstring(_("Credits"))));
        ToolTipService::SetToolTip(BtnCopyDebugInfo(), winrt::box_value(winrt::to_hstring(_("Copy Debug Information"))));
        LblChangelog().Text(winrt::to_hstring(_("Changelog")));
        BtnGitHubRepo().Content(winrt::box_value(winrt::to_hstring(_("GitHub Repo"))));
        BtnReportABug().Content(winrt::box_value(winrt::to_hstring(_("Report a Bug"))));
        BtnDiscussions().Content(winrt::box_value(winrt::to_hstring(_("Discussions"))));
        NavViewSettings().Content(winrt::box_value(winrt::to_hstring(_("Settings"))));
        StatusPageNoImages().Title(winrt::to_hstring(_("No Spotlight Images")));
        StatusPageNoImages().Description(winrt::to_hstring(_("Ensure Windows Spotlight is enabled and come back later to try again")));
        LblImagesTitle().Text(winrt::to_hstring(_("Images")));
        LblExport().Text(winrt::to_hstring(_("Export")));
        ToolTipService::SetToolTip(BtnExport(), winrt::box_value(winrt::to_hstring(_("Export (Ctrl+S)"))));
        LblSetAsBackground().Text(winrt::to_hstring(_("Set as Background")));
        ToolTipService::SetToolTip(BtnSetAsBackground(), winrt::box_value(winrt::to_hstring(_("Set as Background (Ctrl+B)"))));
        BtnView().Label(winrt::to_hstring(_("View")));
        MenuViewGrid().Text(winrt::to_hstring(_("Grid")));
        MenuViewFlip().Text(winrt::to_hstring(_("Flip")));
        BtnExportAll().Label(winrt::to_hstring(_("Export All")));
        ToolTipService::SetToolTip(BtnExportAll(), winrt::box_value(winrt::to_hstring(_("Export All (Ctrl+Shift+S)"))));

    }

    void MainWindow::SetController(const std::shared_ptr<MainWindowController>& controller, ElementTheme systemTheme)
    {
        m_controller = controller;
        m_systemTheme = systemTheme;
        //Register Events
        AppWindow().Closing({ this, &MainWindow::OnClosing });
        m_controller->configurationSaved() += [&](const EventArgs& args) { OnConfigurationSaved(args); };
        m_controller->notificationSent() += [&](const NotificationSentEventArgs& args) { OnNotificationSent(args); };
        m_controller->shellNotificationSent() += [&](const ShellNotificationSentEventArgs& args) { OnShellNotificationSent(args); };
        m_controller->imagesSynced() += [&](const EventArgs& args) { OnImagesSynced(args); };
        //Localize Strings
        TitleBar().Title(winrt::to_hstring(m_controller->getAppInfo().getShortName()));
        TitleBar().Subtitle(m_controller->isDevVersion() ? winrt::to_hstring(_("Preview")) : L"");
        LblAppName().Text(winrt::to_hstring(m_controller->getAppInfo().getShortName()));
        LblAppDescription().Text(winrt::to_hstring(m_controller->getAppInfo().getDescription()));
        LblAppVersion().Text(winrt::to_hstring(m_controller->getAppInfo().getVersion().str()));
        LblAppChangelog().Text(winrt::to_hstring(m_controller->getAppInfo().getChangelog()));
    }

    void MainWindow::OnLoaded(const IInspectable& sender, const RoutedEventArgs& args)
    {
        if (!m_controller)
        {
            throw std::logic_error("MainWindow::SetController() must be called before using the window.");
        }
        if (m_opened)
        {
            return;
        }
        NavView().IsEnabled(false);
        ViewStack().CurrentPage(L"Spinner");
        m_controller->startup();
        m_controller->connectTaskbar(m_hwnd);
        m_controller->getWindowGeometry().apply(m_hwnd);
        m_opened = true;
    }

    void MainWindow::OnClosing(const Microsoft::UI::Windowing::AppWindow& sender, const AppWindowClosingEventArgs& args)
    {
        m_controller->shutdown({ m_hwnd }, MenuViewFlip().IsChecked() ? ViewMode::Flip : ViewMode::Grid);
    }

    void MainWindow::OnActivated(const IInspectable& sender, const WindowActivatedEventArgs& args)
    {
        if(args.WindowActivationState() != WindowActivationState::Deactivated)
        {
            switch(MainGrid().ActualTheme())
            {
            case ElementTheme::Light:
                TitleBar().TitleForeground(SolidColorBrush(Colors::Black()));
                break;
            case ElementTheme::Dark:
                TitleBar().TitleForeground(SolidColorBrush(Colors::White()));
                break;
            default:
                break;
            }
        }
        else
        {
            TitleBar().TitleForeground(SolidColorBrush(Colors::Gray()));
        }
    }

    void MainWindow::OnConfigurationSaved(const EventArgs& args)
    {
        switch (m_controller->getTheme())
        {
        case Theme::Light:
            MainGrid().RequestedTheme(ElementTheme::Light);
            break;
        case Theme::Dark:
            MainGrid().RequestedTheme(ElementTheme::Dark);
            break;
        default:
            MainGrid().RequestedTheme(m_systemTheme);
            break;
        }
    }

    void MainWindow::OnNotificationSent(const NotificationSentEventArgs& args)
    {
        DispatcherQueue().TryEnqueue([this, args]()
        {
            InfoBar().Message(winrt::to_hstring(args.getMessage()));
            switch(args.getSeverity())
            {
            case NotificationSeverity::Success:
                InfoBar().Severity(InfoBarSeverity::Success);
                break;
            case NotificationSeverity::Warning:
                InfoBar().Severity(InfoBarSeverity::Warning);
                break;
            case NotificationSeverity::Error:
                InfoBar().Severity(InfoBarSeverity::Error);
                break;
            default:
                InfoBar().Severity(InfoBarSeverity::Informational);
                break;
            }
            if(m_notificationClickToken)
            {
                BtnInfoBar().Click(m_notificationClickToken);
            }
            if(args.getAction() == "error")
            {
                NavView().SelectedItem(nullptr);
                NavViewImages().IsSelected(true);
            }
            else if(args.getAction() == "update")
            {
                BtnInfoBar().Content(winrt::box_value(winrt::to_hstring(_("Update"))));
                m_notificationClickToken = BtnInfoBar().Click({ this, &MainWindow::WindowsUpdate });
            }
            BtnInfoBar().Visibility(!args.getAction().empty() ? Visibility::Visible : Visibility::Collapsed);
            InfoBar().IsOpen(true);
        });
    }

    void MainWindow::OnShellNotificationSent(const ShellNotificationSentEventArgs& args)
    {
        Aura::getActive().getLogger().log(Logging::LogLevel::Debug, "ShellNotification sent. (" + args.getMessage() + ")");
        ShellNotification::send(args, m_hwnd);
    }

    void MainWindow::OnNavSelectionChanged(const NavigationView& sender, const NavigationViewSelectionChangedEventArgs& args)
    {
        winrt::hstring tag{ NavView().SelectedItem().as<NavigationViewItem>().Tag().as<winrt::hstring>() };
        if(tag == L"Settings")
        {
            WinUI::SettingsPage page{ winrt::make<SettingsPage>() };
            page.as<SettingsPage>()->SetController(m_controller->createPreferencesViewController());
            ViewStack().CurrentPage(L"Custom");
            FrameCustom().Content(winrt::box_value(page));
        }
        else if(tag == L"Images")
        {
            ViewStack().CurrentPage(m_controller->getSpotlightImages().size() > 0 ? L"Images" : L"NoImages");
        }
        else
        {
            ViewStack().CurrentPage(tag);
        }
    }

    void MainWindow::OnNavViewItemTapped(const IInspectable& sender, const TappedRoutedEventArgs& args)
    {
        FlyoutBase::ShowAttachedFlyout(sender.as<FrameworkElement>());
    }

    void MainWindow::CheckForUpdates(const IInspectable& sender, const RoutedEventArgs& args)
    {
        FlyoutBase::GetAttachedFlyout(NavViewHelp().as<FrameworkElement>()).Hide();
        m_controller->checkForUpdates();
    }

    void MainWindow::WindowsUpdate(const IInspectable& sender, const RoutedEventArgs& args)
    {
        TitleBar().SearchVisibility(Visibility::Collapsed);
        InfoBar().IsOpen(false);
        NavView().IsEnabled(false);
        ViewStack().CurrentPage(L"Spinner");
        m_controller->windowsUpdate();
    }

    void MainWindow::CopyDebugInformation(const IInspectable& sender, const RoutedEventArgs& args)
    {
        FlyoutBase::GetAttachedFlyout(NavViewHelp().as<FrameworkElement>()).Hide();
        DataPackage dataPackage;
        dataPackage.SetText(winrt::to_hstring(m_controller->getDebugInformation()));
        Clipboard::SetContent(dataPackage);
        OnNotificationSent({ _("Debug information copied to clipboard."), NotificationSeverity::Success });
    }

    Windows::Foundation::IAsyncAction MainWindow::GitHubRepo(const IInspectable& sender, const RoutedEventArgs& args)
    {
        co_await Launcher::LaunchUriAsync(Windows::Foundation::Uri{ winrt::to_hstring(m_controller->getAppInfo().getSourceRepo()) });
    }

    Windows::Foundation::IAsyncAction MainWindow::ReportABug(const IInspectable& sender, const RoutedEventArgs& args)
    {
        co_await Launcher::LaunchUriAsync(Windows::Foundation::Uri{ winrt::to_hstring(m_controller->getAppInfo().getIssueTracker()) });
    }

    Windows::Foundation::IAsyncAction MainWindow::Discussions(const IInspectable& sender, const RoutedEventArgs& args)
    {
        co_await Launcher::LaunchUriAsync(Windows::Foundation::Uri{ winrt::to_hstring(m_controller->getAppInfo().getSupportUrl()) });
    }

    Windows::Foundation::IAsyncAction MainWindow::Credits(const IInspectable& sender, const RoutedEventArgs& args)
    {
        FlyoutBase::GetAttachedFlyout(NavViewHelp().as<FrameworkElement>()).Hide();
        ContentDialog dialog;
        dialog.Title(winrt::box_value(winrt::to_hstring(_("Credits"))));
        if(m_controller->getAppInfo().getTranslatorNames().size() == 1 && m_controller->getAppInfo().getTranslatorNames()[0] == "translator-credits")
        {
            dialog.Content(winrt::box_value(winrt::to_hstring(std::vformat(_("Developers:\n{}\nDesigners:\n{}\nArtists:\n{}"), std::make_format_args(CodeHelpers::unmove(StringHelpers::join(keys(m_controller->getAppInfo().getDevelopers()), "\n")), CodeHelpers::unmove(StringHelpers::join(keys(m_controller->getAppInfo().getDesigners()), "\n")), CodeHelpers::unmove(StringHelpers::join(keys(m_controller->getAppInfo().getArtists()), "\n", false)))))));
        }
        else
        {
            dialog.Content(winrt::box_value(winrt::to_hstring(std::vformat(_("Developers:\n{}\nDesigners:\n{}\nArtists:\n{}\nTranslators:\n{}"), std::make_format_args(CodeHelpers::unmove(StringHelpers::join(keys(m_controller->getAppInfo().getDevelopers()), "\n")), CodeHelpers::unmove(StringHelpers::join(keys(m_controller->getAppInfo().getDesigners()), "\n")), CodeHelpers::unmove(StringHelpers::join(keys(m_controller->getAppInfo().getArtists()), "\n")), CodeHelpers::unmove(StringHelpers::join(m_controller->getAppInfo().getTranslatorNames(), "\n", false)))))));
        }
        dialog.CloseButtonText(winrt::to_hstring(_("Close")));
        dialog.DefaultButton(ContentDialogButton::Close);
        dialog.RequestedTheme(MainGrid().ActualTheme());
        dialog.XamlRoot(MainGrid().XamlRoot());
        co_await dialog.ShowAsync();
    }

    void MainWindow::OnImagesSynced(const EventArgs& args)
    {
        DispatcherQueue().TryEnqueue([this]()
        {
            NavView().IsEnabled(true);
            NavViewImages().IsSelected(true);
            LblImagesCount().Text(winrt::to_hstring(std::vformat(_("Total Number of Images: {}"), std::make_format_args(CodeHelpers::unmove(m_controller->getSpotlightImages().size())))));
            for(const std::filesystem::path& image : m_controller->getSpotlightImages())
            {
                Image gridControl;
                gridControl.Stretch(Stretch::Fill);
                gridControl.Source(BitmapImage(Windows::Foundation::Uri(winrt::to_hstring(image.string()))));
                GridImages().Items().Append(gridControl);
                Image flipControl;
                flipControl.Source(BitmapImage(Windows::Foundation::Uri(winrt::to_hstring(image.string()))));
                flipControl.Stretch(Stretch::UniformToFill);
                FlipImages().Items().Append(flipControl);
            }
            if(m_controller->getViewMode() == ViewMode::Flip)
            {
                MenuViewFlip().IsChecked(true);
            }
            else
            {
                MenuViewGrid().IsChecked(true);
            }
            ChangeImageViewMode(nullptr, nullptr);
            GridImages().SelectedIndex(0);
        });
    }

    void MainWindow::OnImageSelectionChanged(const IInspectable& sender, const SelectionChangedEventArgs& args)
    {
        if(MenuViewGrid().IsChecked())
        {
            BtnSetAsBackground().Visibility(GridImages().SelectedIndex() != -1 ? Visibility::Visible : Visibility::Collapsed);
            BtnExport().Visibility(GridImages().SelectedIndex() != -1 ? Visibility::Visible : Visibility::Collapsed);
        }
        else if(MenuViewFlip().IsChecked())
        {
            BtnSetAsBackground().Visibility(FlipImages().SelectedIndex() != -1 ? Visibility::Visible : Visibility::Collapsed);
            BtnExport().Visibility(FlipImages().SelectedIndex() != -1 ? Visibility::Visible : Visibility::Collapsed);
        }
    }

    Windows::Foundation::IAsyncAction MainWindow::OnImageDoubleTapped(const IInspectable& sender, const DoubleTappedRoutedEventArgs& args)
    {
        const std::filesystem::path& image{ m_controller->getSpotlightImages()[GridImages().SelectedIndex()] };
        co_await Launcher::LaunchFileAsync(co_await StorageFile::GetFileFromPathAsync(winrt::to_hstring(image.string())));
    }

    void MainWindow::ChangeImageViewMode(const IInspectable& sender, const RoutedEventArgs& args)
    {
        if(MenuViewGrid().IsChecked())
        {
            ScrollImages().Visibility(Visibility::Visible);
            FlipImages().Visibility(Visibility::Collapsed);
            MenuViewFlip().IsChecked(false);
        }
        else if(MenuViewFlip().IsChecked())
        {
            ScrollImages().Visibility(Visibility::Collapsed);
            FlipImages().Visibility(Visibility::Visible);
            MenuViewGrid().IsChecked(false);
        }
        OnImageSelectionChanged(sender, nullptr);
    }

    Windows::Foundation::IAsyncAction MainWindow::Export(const IInspectable& sender, const RoutedEventArgs& args)
    {
        int selectedIndex{ MenuViewGrid().IsChecked() ? GridImages().SelectedIndex() : FlipImages().SelectedIndex() };
        FileSavePicker picker;
        picker.as<::IInitializeWithWindow>()->Initialize(m_hwnd);
        picker.FileTypeChoices().Insert(L"JPG", winrt::single_threaded_vector<winrt::hstring>({ L".jpg" }));
        StorageFile file{ co_await picker.PickSaveFileAsync() };
        if(file)
        {
            m_controller->exportImage(selectedIndex, winrt::to_string(file.Path()));
        }
    }

    Windows::Foundation::IAsyncAction MainWindow::ExportAll(const IInspectable& sender, const RoutedEventArgs& args)
    {
        FolderPicker picker;
        picker.as<::IInitializeWithWindow>()->Initialize(m_hwnd);
        picker.FileTypeFilter().Append(L"*");
        StorageFolder folder{ co_await picker.PickSingleFolderAsync() };
        if(folder)
        {
            m_controller->exportAllImages(winrt::to_string(folder.Path()));
        }
    }

    void MainWindow::SetAsBackground(const IInspectable& sender, const RoutedEventArgs& args)
    {
        int selectedIndex{ MenuViewGrid().IsChecked() ? GridImages().SelectedIndex() : FlipImages().SelectedIndex() };
        m_controller->setImageAsDesktopBackground(selectedIndex);
    }
}
