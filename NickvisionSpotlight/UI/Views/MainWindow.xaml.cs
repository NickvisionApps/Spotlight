using Microsoft.UI;
using Microsoft.UI.Composition;
using Microsoft.UI.Composition.SystemBackdrops;
using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media;
using NickvisionSpotlight.Models;
using NickvisionSpotlight.UI.Controls;
using System;
using System.Threading.Tasks;
using Vanara.PInvoke;
using Windows.Storage.Pickers;
using Windows.System;
using Windows.UI.ApplicationSettings;
using WinRT;
using WinRT.Interop;

namespace NickvisionSpotlight.UI.Views
{
    /// <summary>
    /// The MainWindow for the application
    /// </summary>
    public sealed partial class MainWindow : Window
    {
        private readonly IntPtr _hwnd;
        private readonly AppWindow _appWindow;
        private bool _isActived;
        private readonly SystemBackdropConfiguration _backdropConfiguration;
        private readonly MicaController? _micaController;
        private readonly DesktopAcrylicController? _acrylicController;
        private readonly SpotlightManager _spotlightManager;
        private readonly SpotlightPage _spotlightPage;

        /// <summary>
        /// Constructs a MainWindow
        /// </summary>
        public MainWindow()
        {
            InitializeComponent();
            //Initialize Vars
            _hwnd = WindowNative.GetWindowHandle(this);
            _appWindow = AppWindow.GetFromWindowId(Win32Interop.GetWindowIdFromWindow(_hwnd));
            _isActived = true;
            _spotlightManager = new SpotlightManager();
            //Register Events
            _appWindow.Closing += Window_Closing;
            //Set TitleBar
            TitleBarTitle.Text = AppInfo.Current.Name;
            _appWindow.Title = TitleBarTitle.Text;
            if (AppWindowTitleBar.IsCustomizationSupported())
            {
                _appWindow.TitleBar.ExtendsContentIntoTitleBar = true;
                TitleBarLeftPaddingColumn.Width = new GridLength(_appWindow.TitleBar.LeftInset);
                TitleBarRightPaddingColumn.Width = new GridLength(_appWindow.TitleBar.RightInset);
                _appWindow.TitleBar.ButtonBackgroundColor = Colors.Transparent;
                _appWindow.TitleBar.ButtonInactiveBackgroundColor = Colors.Transparent;
            }
            else
            {
                TitleBar.Visibility = Visibility.Collapsed;
                ToolBar.Margin = new Thickness(6, 0, 6, 0);
                NavView.Margin = new Thickness(0, 50, 0, 0);
            }
            //Setup Backdrop
            new WindowsSystemDispatcherQueueHelper().EnsureWindowsSystemDispatcherQueueController();
            _backdropConfiguration = new SystemBackdropConfiguration()
            {
                IsInputActive = true,
                Theme = ((FrameworkElement)Content).ActualTheme switch
                {
                    ElementTheme.Default => SystemBackdropTheme.Default,
                    ElementTheme.Light => SystemBackdropTheme.Light,
                    ElementTheme.Dark => SystemBackdropTheme.Dark,
                    _ => SystemBackdropTheme.Default
                }
            };
            if (MicaController.IsSupported())
            {
                _micaController = new MicaController();
                _micaController.AddSystemBackdropTarget(this.As<ICompositionSupportsSystemBackdrop>());
                _micaController.SetSystemBackdropConfiguration(_backdropConfiguration);
            }
            else if (DesktopAcrylicController.IsSupported())
            {
                _acrylicController = new DesktopAcrylicController();
                _acrylicController.AddSystemBackdropTarget(this.As<ICompositionSupportsSystemBackdrop>());
                _acrylicController.SetSystemBackdropConfiguration(_backdropConfiguration);
            }
            //Maximize
            User32.ShowWindow(_hwnd, ShowWindowCommand.SW_SHOWMAXIMIZED);
            //Pages
            _spotlightPage = new SpotlightPage(_spotlightManager);
            ChangePage(Pages.Spotlight);
            //Messages
            Messenger.Current.Register("MainWindow.ChangePage", (object? parameter) =>
            {
                if (parameter is Pages page)
                {
                    ChangePage(page);
                }
            });
            Messenger.Current.Register("MainWindow.ShowInfoBarMessage", (object? parameter) =>
            {
                if (parameter is InfoBarMessageInfo info)
                {
                    ShowInfoBarMessage(info);
                }
            });
            Messenger.Current.Register("MainWindow.InitializeWithWindow", (object? parameter) =>
            {
                if (parameter != null)
                {
                    InitializeWithWindow.Initialize(parameter, _hwnd);
                }
            });
            Messenger.Current.Register("MainWindow.SyncSpotlightImagesAsync", async (object? parameter) => await SyncSpotlightImagesAsync());
        }

        /// <summary>
        /// Occurs when the window is activated
        /// </summary>
        /// <param name="sender">object</param>
        /// <param name="e">WindowActivatedEventArgs</param>
        private void Window_Activated(object sender, WindowActivatedEventArgs e)
        {
            _isActived = e.WindowActivationState != WindowActivationState.Deactivated;
            //Update TitleBar
            TitleBarTitle.Foreground = (SolidColorBrush)App.Current.Resources[_isActived ? "WindowCaptionForeground" : "WindowCaptionForegroundDisabled"];
            _appWindow.TitleBar.ButtonForegroundColor = ((SolidColorBrush)App.Current.Resources[_isActived ? "WindowCaptionForeground" : "WindowCaptionForegroundDisabled"]).Color;
            //Update Backdrop
            _backdropConfiguration.IsInputActive = _isActived;
        }

        /// <summary>
        /// Occurs when the window is closing
        /// </summary>
        /// <param name="sender">AppWindow</param>
        /// <param name="e">AppWindowClosingEventArgs</param>
        private void Window_Closing(AppWindow sender, AppWindowClosingEventArgs e)
        {
            _micaController?.Dispose();
            _acrylicController?.Dispose();
        }

        /// <summary>
        /// Occurs when the window's theme is changed
        /// </summary>
        /// <param name="sender">FrameworkElement</param>
        /// <param name="e">object</param>
        private void Window_ActualThemeChanged(FrameworkElement sender, object e)
        {
            //Update TitleBar
            TitleBarTitle.Foreground = (SolidColorBrush)App.Current.Resources[_isActived ? "WindowCaptionForeground" : "WindowCaptionForegroundDisabled"];
            _appWindow.TitleBar.ButtonForegroundColor = ((SolidColorBrush)App.Current.Resources[_isActived ? "WindowCaptionForeground" : "WindowCaptionForegroundDisabled"]).Color;
            //Update Backdrop
            _backdropConfiguration.Theme = sender.ActualTheme switch
            {
                ElementTheme.Default => SystemBackdropTheme.Default,
                ElementTheme.Light => SystemBackdropTheme.Light,
                ElementTheme.Dark => SystemBackdropTheme.Dark,
                _ => SystemBackdropTheme.Default
            };
        }

        /// <summary>
        /// Occurs when the sync spotlight images button is clicked
        /// </summary>
        /// <param name="sender">object</param>
        /// <param name="e">RoutedEventArgs</param>
        private async void BtnSyncSpotlightImages_Click(object sender, RoutedEventArgs e)
        {
            var syncingDialog = new ProgressDialog("Syncing spotlight images...", async () => await SyncSpotlightImagesAsync())
            {
                XamlRoot = Content.XamlRoot
            };
            await syncingDialog.ShowAsync();
        }

        /// <summary>
        /// Occurs when the report a bug menu item is clicked
        /// </summary>
        /// <param name="sender">object</param>
        /// <param name="e">RoutedEventArgs</param>
        private async void MenuReportABug_Click(object sender, RoutedEventArgs e) => await Launcher.LaunchUriAsync(AppInfo.Current.IssueTracker);

        /// <summary>
        /// Occurs when the github repo menu item is clicked
        /// </summary>
        /// <param name="sender">object</param>
        /// <param name="e">RoutedEventArgs</param>
        private async void MenuGitHubRepo_Click(object sender, RoutedEventArgs e) => await Launcher.LaunchUriAsync(AppInfo.Current.GitHubRepo);

        /// <summary>
        /// Occurs when the changelog menu item is clicked
        /// </summary>
        /// <param name="sender">object</param>
        /// <param name="e">RoutedEventArgs</param>
        private async void MenuChangelog_Click(object sender, RoutedEventArgs e)
        {
            var changelogDialog = new ContentDialog()
            {
                Title = "What's New?",
                Content = AppInfo.Current.Changelog,
                CloseButtonText = "OK",
                DefaultButton = ContentDialogButton.Close,
                XamlRoot = Content.XamlRoot
            };
            await changelogDialog.ShowAsync();
        }

        /// <summary>
        /// Occurs when the about menu item is clicked
        /// </summary>
        /// <param name="sender">object</param>
        /// <param name="e">RoutedEventArgs</param>
        private async void MenuAbout_Click(object sender, RoutedEventArgs e)
        {
            var aboutDialog = new ContentDialog()
            {
                Title = $"About {AppInfo.Current.Name}",
                Content = $"{AppInfo.Current.Description}\nVersion {AppInfo.Current.Version}\n\nCopyright (C) 2021-2022\nAll rights reserved.\nNicholas Logozzo\nNickvision\n\nBuilt with C# and Windows App SDK",
                CloseButtonText = "OK",
                DefaultButton = ContentDialogButton.Close,
                XamlRoot = Content.XamlRoot
            };
            await aboutDialog.ShowAsync();
        }

        /// <summary>
        /// Occurs when the navigation view selection changes
        /// </summary>
        /// <param name="sender">NavigationView</param>
        /// <param name="args">NavigationViewSelectionChangedEventArgs</param>
        private void NavView_SelectionChanged(NavigationView sender, NavigationViewSelectionChangedEventArgs args)
        {
            var tag = (string)((NavigationViewItem)NavView.SelectedItem).Tag;
            if (tag == "Spotlight")
            {
                Frame.Content = _spotlightPage;
            }
            else if (tag == "Settings")
            {
                Frame.Content = new SettingsPage();
            }
            else
            {
                Frame.Content = null;
            }
        }

        /// <summary>
        /// Changes the page in the navigation view
        /// </summary>
        /// <param name="page">Pages</param>
        private void ChangePage(Pages page)
        {
            if (page == Pages.Spotlight)
            {
                NavItemSpotlight.IsSelected = true;
            }
            else if (page == Pages.Settings)
            {
                NavItemSettings.IsSelected = true;
            }
        }

        /// <summary>
        /// Shows a message in the InfoBar
        /// </summary>
        /// <param name="info">InfoBarMessageInfo</param>
        private void ShowInfoBarMessage(InfoBarMessageInfo info)
        {
            InfoBar.Title = info.Title;
            InfoBar.Message = info.Message;
            InfoBar.Severity = info.Severity;
            InfoBar.IsOpen = true;
        }

        /// <summary>
        /// Syncs spotlight images and updates the UI
        /// </summary>
        /// <returns></returns>
        private async Task SyncSpotlightImagesAsync()
        {
            await _spotlightManager.SyncSpotlightImagesAsync();
            TxtTotalImages.Text = _spotlightManager.SpotlightImages.Count > 0 ? $"Total Images Synced: {_spotlightManager.SpotlightImages.Count}" : "";
            Messenger.Current.Send("SpotlightPage.UpdateSpotlightList");
        }
    }
}