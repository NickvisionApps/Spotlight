using Microsoft.UI;
using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Input;
using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Media.Imaging;
using Microsoft.Windows.AppNotifications;
using Microsoft.Windows.AppNotifications.Builder;
using NickvisionSpotlight.Shared.Controllers;
using NickvisionSpotlight.Shared.Events;
using NickvisionSpotlight.WinUI.Controls;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Vanara.PInvoke;
using Windows.Graphics;
using Windows.Storage;
using Windows.Storage.Pickers;
using Windows.System;
using WinRT.Interop;

namespace NickvisionSpotlight.WinUI.Views;

/// <summary>
/// The MainWindow for the application
/// </summary>
public sealed partial class MainWindow : Window
{
    private readonly MainWindowController _controller;
    private readonly IntPtr _hwnd;
    private bool _isActived;
    private RoutedEventHandler? _notificationButtonClickEvent;

    private enum Monitor_DPI_Type : int
    {
        MDT_Effective_DPI = 0,
        MDT_Angular_DPI = 1,
        MDT_Raw_DPI = 2,
        MDT_Default = MDT_Effective_DPI
    }

    [DllImport("Shcore.dll", SetLastError = true)]
    private static extern int GetDpiForMonitor(IntPtr hmonitor, Monitor_DPI_Type dpiType, out uint dpiX, out uint dpiY);

    /// <summary>
    /// Constructs a MainWindow
    /// </summary>
    /// <param name="controller">The MainWindowController</param>
    public MainWindow(MainWindowController controller)
    {
        InitializeComponent();
        //Initialize Vars
        _controller = controller;
        _hwnd = WindowNative.GetWindowHandle(this);
        _isActived = true;
        //Register Events
        AppWindow.Closing += Window_Closing;
        _controller.NotificationSent += NotificationSent;
        //Set TitleBar
        TitleBarTitle.Text = _controller.AppInfo.ShortName;
        AppWindow.TitleBar.ExtendsContentIntoTitleBar = true;
        AppWindow.TitleBar.PreferredHeightOption = TitleBarHeightOption.Tall;
        AppWindow.TitleBar.ButtonBackgroundColor = Colors.Transparent;
        AppWindow.TitleBar.ButtonInactiveBackgroundColor = Colors.Transparent;
        TitlePreview.Text = _controller.IsDevVersion ? _controller.Localizer["Preview", "WinUI"] : "";
        AppWindow.Title = TitleBarTitle.Text;
        AppWindow.SetIcon(@"Assets\org.nickvision.spotlight.ico");
        SystemBackdrop = new MicaBackdrop();
        //Window Sizing
        AppWindow.Resize(new SizeInt32(900, 700));
        User32.ShowWindow(_hwnd, ShowWindowCommand.SW_SHOWMAXIMIZED);
        //Localize Strings
        MenuFile.Title = _controller.Localizer["File"];
        MenuSyncSpotlightImages.Text = _controller.Localizer["SyncSpotlightImages"];
        MenuExit.Text = _controller.Localizer["Exit"];
        MenuEdit.Title = _controller.Localizer["Edit"];
        MenuSettings.Text = _controller.Localizer["Settings"];
        MenuImages.Title = _controller.Localizer["Images"];
        MenuExportImage.Text = _controller.Localizer["ExportImage"];
        MenuExportAllImages.Text = _controller.Localizer["ExportAllImages"];
        MenuSetAsBackground.Text = _controller.Localizer["SetAsBackground"];
        MenuAbout.Text = string.Format(_controller.Localizer["About"], _controller.AppInfo.ShortName);
        MenuHelp.Title = _controller.Localizer["Help"];
        LblStatus.Text = _controller.Localizer["StatusReady", "WinUI"];
        StatusPageHome.Glyph = _controller.ShowSun ? "\xE706" : "\xE708";
        StatusPageHome.Title = _controller.Greeting;
        StatusPageHome.Description = _controller.Localizer["Start"];
        ToolTipService.SetToolTip(BtnSyncSpotlightImages, _controller.Localizer["SyncSpotlightImages", "Tooltip"]);
        LblBtnSyncSpotlightImages.Text = _controller.Localizer["SyncSpotlightImages"];
        BtnExportImage.Label = _controller.Localizer["ExportImage"];
        ToolTipService.SetToolTip(BtnExportImage, _controller.Localizer["ExportImage", "Tooltip"]);
        ToolTipService.SetToolTip(BtnExportAllImages, _controller.Localizer["ExportAllImages", "Tooltip"]);
        BtnSetAsBackground.Label = _controller.Localizer["SetAsBackground"];
        ToolTipService.SetToolTip(BtnSetAsBackground, _controller.Localizer["SetAsBackground", "Tooltip"]);
        //Pages
        ViewStack.ChangePage("Home");
    }

    /// <summary>
    /// Calls InitializeWithWindow.Initialize on the target object with the MainWindow's hwnd
    /// </summary>
    /// <param name="target">The target object to initialize</param>
    public void InitializeWithWindow(object target) => WinRT.Interop.InitializeWithWindow.Initialize(target, _hwnd);

    /// <summary>
    /// Occurs when the window is activated
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">WindowActivatedEventArgs</param>
    private void Window_Activated(object sender, WindowActivatedEventArgs e)
    {
        _isActived = e.WindowActivationState != WindowActivationState.Deactivated;
        //Update TitleBar
        TitleBarTitle.Foreground = (SolidColorBrush)Application.Current.Resources[_isActived ? "WindowCaptionForeground" : "WindowCaptionForegroundDisabled"];
        AppWindow.TitleBar.ButtonForegroundColor = ((SolidColorBrush)Application.Current.Resources[_isActived ? "WindowCaptionForeground" : "WindowCaptionForegroundDisabled"]).Color;
    }

    /// <summary>
    /// Occurs when the window is closing
    /// </summary>
    /// <param name="sender">AppWindow</param>
    /// <param name="e">AppWindowClosingEventArgs</param>
    private void Window_Closing(AppWindow sender, AppWindowClosingEventArgs e)
    {
        _controller.Dispose();
    }

    /// <summary>
    /// Occurs when the window's theme is changed
    /// </summary>
    /// <param name="sender">FrameworkElement</param>
    /// <param name="e">object</param>
    private void Window_ActualThemeChanged(FrameworkElement sender, object e)
    {
        //Update TitleBar
        TitleBarTitle.Foreground = (SolidColorBrush)Application.Current.Resources[_isActived ? "WindowCaptionForeground" : "WindowCaptionForegroundDisabled"];
        MenuFile.Foreground = (SolidColorBrush)Application.Current.Resources[_isActived ? "WindowCaptionForeground" : "WindowCaptionForegroundDisabled"];
        MenuEdit.Foreground = (SolidColorBrush)Application.Current.Resources[_isActived ? "WindowCaptionForeground" : "WindowCaptionForegroundDisabled"];
        MenuHelp.Foreground = (SolidColorBrush)Application.Current.Resources[_isActived ? "WindowCaptionForeground" : "WindowCaptionForegroundDisabled"];
        AppWindow.TitleBar.ButtonForegroundColor = ((SolidColorBrush)Application.Current.Resources[_isActived ? "WindowCaptionForeground" : "WindowCaptionForegroundDisabled"]).Color;
    }

    /// <summary>
    /// Occurs when the TitleBar is loaded
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">RoutedEventArgs</param>
    private void TitleBar_Loaded(object sender, RoutedEventArgs e) => SetDragRegionForCustomTitleBar();

    /// <summary>
    /// Occurs when the TitleBar's size is changed
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">RoutedEventArgs</param>
    private void TitleBar_SizeChanged(object sender, SizeChangedEventArgs e) => SetDragRegionForCustomTitleBar();

    /// <summary>
    /// Sets the drag region for the TitleBar
    /// </summary>
    /// <exception cref="Exception"></exception>
    private void SetDragRegionForCustomTitleBar()
    {
        var hMonitor = Win32Interop.GetMonitorFromDisplayId(DisplayArea.GetFromWindowId(Win32Interop.GetWindowIdFromWindow(_hwnd), DisplayAreaFallback.Primary).DisplayId);
        var result = GetDpiForMonitor(hMonitor, Monitor_DPI_Type.MDT_Default, out uint dpiX, out uint _);
        if (result != 0)
        {
            throw new Exception("Could not get DPI for monitor.");
        }
        var scaleFactorPercent = (uint)(((long)dpiX * 100 + (96 >> 1)) / 96);
        var scaleAdjustment = scaleFactorPercent / 100.0;
        RightPaddingColumn.Width = new GridLength(AppWindow.TitleBar.RightInset / scaleAdjustment);
        LeftPaddingColumn.Width = new GridLength(AppWindow.TitleBar.LeftInset / scaleAdjustment);
        var dragRectsList = new List<RectInt32>();
        RectInt32 dragRectL;
        dragRectL.X = (int)((LeftPaddingColumn.ActualWidth) * scaleAdjustment);
        dragRectL.Y = 0;
        dragRectL.Height = (int)(TitleBar.ActualHeight * scaleAdjustment);
        dragRectL.Width = (int)((IconColumn.ActualWidth
                                + TitleColumn.ActualWidth
                                + LeftDragColumn.ActualWidth) * scaleAdjustment);
        dragRectsList.Add(dragRectL);
        RectInt32 dragRectR;
        dragRectR.X = (int)((LeftPaddingColumn.ActualWidth
                            + IconColumn.ActualWidth
                            + TitleBarTitle.ActualWidth
                            + LeftDragColumn.ActualWidth
                            + MainMenu.ActualWidth) * scaleAdjustment);
        dragRectR.Y = 0;
        dragRectR.Height = (int)(TitleBar.ActualHeight * scaleAdjustment);
        dragRectR.Width = (int)(RightDragColumn.ActualWidth * scaleAdjustment);
        dragRectsList.Add(dragRectR);
        RectInt32[] dragRects = dragRectsList.ToArray();
        AppWindow.TitleBar.SetDragRectangles(dragRects);
    }

    /// <summary>
    /// Occurs when a notification is sent from the controller
    /// </summary>
    /// <param name="sender">object?</param>
    /// <param name="e">NotificationSentEventArgs</param>
    private void NotificationSent(object? sender, NotificationSentEventArgs e)
    {
        //InfoBar
        InfoBar.Message = e.Message;
        InfoBar.Severity = e.Severity switch
        {
            NotificationSeverity.Informational => InfoBarSeverity.Informational,
            NotificationSeverity.Success => InfoBarSeverity.Success,
            NotificationSeverity.Warning => InfoBarSeverity.Warning,
            NotificationSeverity.Error => InfoBarSeverity.Error,
            _ => InfoBarSeverity.Informational
        };
        if (_notificationButtonClickEvent != null)
        {
            BtnInfoBar.Click -= _notificationButtonClickEvent;
        }
        BtnInfoBar.Visibility = !string.IsNullOrEmpty(e.Action) ? Visibility.Visible : Visibility.Collapsed;
        InfoBar.IsOpen = true;
    }

    /// <summary>
    /// Sends a shell notification
    /// </summary>
    /// <param name="e">ShellNotificationSentEventArgs</param>
    private void SendShellNotification(ShellNotificationSentEventArgs e)
    {
        var notificationBuilder = new AppNotificationBuilder().AddText(e.Title, new AppNotificationTextProperties().SetMaxLines(1)).AddText(e.Message);
        AppNotificationManager.Default.Show(notificationBuilder.BuildNotification());
    }

    /// <summary>
    /// Occurs when the sync spotlight images menu item is clicked
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">RoutedEventArgs</param>
    private async void SyncSpotlightImages(object sender, RoutedEventArgs e)
    {
        LblLoading.Text = _controller.Localizer["Syncing"];
        Loading.IsLoading = true;
        await _controller.SyncSpotlightImagesAsync();
        Loading.IsLoading = false;
        ViewStack.ChangePage("Spotlight");
        IconStatus.Glyph = "\uE8B9";
        LblStatus.Text = string.Format(_controller.Localizer["TotalSpotlightImages"], _controller.SpotlightImagesCount);
        MenuExportAllImages.IsEnabled = _controller.SpotlightImagesCount > 0;
        BtnExportAllImages.IsEnabled = _controller.SpotlightImagesCount > 0;
        ListSpotlight.Items.Clear();
        for (var i = 0; i < _controller.SpotlightImagesCount; i++)
        {
            ListSpotlight.Items.Add(new Image()
            {
                Width = 400,
                Height = 300,
                Source = new BitmapImage(new Uri(_controller.GetSpotlightImagePathByIndex(i))),
                Stretch = Stretch.UniformToFill
            });
        }
    }

    /// <summary>
    /// Occurs when the exit menu item is clicked
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">RoutedEventArgs</param>
    private void Exit(object sender, RoutedEventArgs e) => Close();

    /// <summary>
    /// Occurs when the settings menu item is clicked
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">RoutedEventArgs</param>
    private async void Settings(object sender, RoutedEventArgs e)
    {
        var preferencesDialog = new SettingsDialog(_controller.CreatePreferencesViewController())
        {
            XamlRoot = Content.XamlRoot,
            RequestedTheme = MainMenu.RequestedTheme
        };
        await preferencesDialog.ShowAsync();
    }

    /// <summary>
    /// Occurs when the export image menu item is clicked
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">RoutedEventArgs</param>
    private async void ExportImage(object sender, RoutedEventArgs e)
    {
        var fileSavePicker = new FileSavePicker();
        fileSavePicker.FileTypeChoices.Add("Images", new List<string>() { ".jpg" });
        InitializeWithWindow(fileSavePicker);
        var file = await fileSavePicker.PickSaveFileAsync();
        if (file != null)
        {
            _controller.ExportImage(ListSpotlight.SelectedIndex, file.Path);
        }
    }

    /// <summary>
    /// Occurs when the export all images menu item is clicked
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">RoutedEventArgs</param>
    private async void ExportAllImages(object sender, RoutedEventArgs e)
    {
        var folderPicker = new FolderPicker();
        folderPicker.FileTypeFilter.Add("*");
        InitializeWithWindow(folderPicker);
        var folder = await folderPicker.PickSingleFolderAsync();
        if (folder != null)
        {
            LblLoading.Text = _controller.Localizer["Exporting"];
            Loading.IsLoading = true;
            await _controller.ExportAllImagesAsync(folder.Path);
            Loading.IsLoading = false;
        }
    }

    /// <summary>
    /// Occurs when the set as background menu item is clicked
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">RoutedEventArgs</param>
    private void SetAsBackground(object sender, RoutedEventArgs e) => _controller.SetAsBackground(ListSpotlight.SelectedIndex);

    /// <summary>
    /// Occurs when the about menu item is clicked
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">RoutedEventArgs</param>
    private async void About(object sender, RoutedEventArgs e)
    {
        var aboutDialog = new AboutDialog(_controller.AppInfo, _controller.Localizer)
        {
            XamlRoot = Content.XamlRoot,
            RequestedTheme = MainMenu.RequestedTheme
        };
        await aboutDialog.ShowAsync();
    }

    /// <summary>
    /// Occurs when the ListSpotlight's selection changed
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">SelectionChangedEventArgs</param>
    private void ListSpotlight_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        MenuExportImage.IsEnabled = ListSpotlight.SelectedIndex != -1;
        MenuSetAsBackground.IsEnabled = ListSpotlight.SelectedIndex != -1;
        BtnExportImage.IsEnabled = ListSpotlight.SelectedIndex != -1;
        BtnSetAsBackground.IsEnabled = ListSpotlight.SelectedIndex != -1;
    }

    /// <summary>
    /// Occurs when the ListSpotlight is double tapped
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">DoubleTappedRoutedEventArgs</param>
    private async void ListSpotlight_DoubleTapped(object sender, DoubleTappedRoutedEventArgs e)
    {
        var contentDialog = new ContentDialog()
        {
            Content = new Image()
            {
                Source = new BitmapImage(new Uri(_controller.GetSpotlightImagePathByIndex(ListSpotlight.SelectedIndex))),
                Stretch = Stretch.Fill
            },
            CloseButtonText = _controller.Localizer["OK"],
            DefaultButton = ContentDialogButton.Close,
            XamlRoot = Content.XamlRoot
        };
        await contentDialog.ShowAsync();
    }
}
