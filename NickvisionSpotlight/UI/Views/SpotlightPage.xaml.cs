using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Media.Imaging;
using NickvisionSpotlight.Models;
using NickvisionSpotlight.UI.Controls;
using System;
using System.Collections.Generic;
using Windows.Storage.Pickers;

namespace NickvisionSpotlight.UI.Views;

/// <summary>
/// The spotlight page for the application
/// </summary>
public sealed partial class SpotlightPage : Page
{
    private readonly MainWindow _mainWindow;
    private readonly SpotlightManager _spotlightManager;

    /// <summary>
    /// Constructs a SpotlightPage
    /// </summary>
    /// <param name="mainWindow">The MainWindow</param>
    /// <param name="spotlightManager">The SpotlightManager</param>
    public SpotlightPage(MainWindow mainWindow, SpotlightManager spotlightManager)
    {
        InitializeComponent();
        _mainWindow = mainWindow;
        _spotlightManager = spotlightManager;
        _spotlightManager.ImagesChanged += SpotlightManager_ImagesChanged;
    }

    /// <summary>
    /// Updates ListSpotlight with the number of synced images
    /// </summary>
    private void SpotlightManager_ImagesChanged(object? sender, EventArgs e)
    {
        ListSpotlight.Items.Clear();
        for (int i = 0; i < _spotlightManager.SpotlightImages.Count; i++)
        {
            ListSpotlight.Items.Add(i + 1);
        }
        BtnExportAllImages.IsEnabled = _spotlightManager.SpotlightImages.Count > 0;
        if (_spotlightManager.SpotlightImages.Count == 0)
        {
            _mainWindow.ShowInfoBarMessage(new InfoBarMessageInfo("No Spotlight Images?", "For Nickvision Spotlight to find spotlight images, please make sure you have your lockscreen is set to Windows Spotlight for Windows to download spotlight images and check back later.", InfoBarSeverity.Warning));
        }
    }

    /// <summary>
    /// Occurs when the export image button is clicked
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">RoutedEventArgs</param>
    private async void BtnExportImage_Click(object sender, RoutedEventArgs e)
    {
        if(ListSpotlight.SelectedIndex != -1)
        {
            var fileSavePicker = new FileSavePicker();
            fileSavePicker.FileTypeChoices.Add("Images", new List<string>() { ".jpg" });
            _mainWindow.InitializeWithWindow(fileSavePicker);
            var file = await fileSavePicker.PickSaveFileAsync();
            if (file != null)
            {
                _spotlightManager.ExportImage(ListSpotlight.SelectedIndex, file.Path);
                _mainWindow.ShowInfoBarMessage(new InfoBarMessageInfo("Export Successful", $"Image saved to: {file.Path}", InfoBarSeverity.Success));
            }
        }
    }

    /// <summary>
    /// Occurs when the export all images button is clicked
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">RoutedEventArgs</param>
    private async void BtnExportAllImages_Click(object sender, RoutedEventArgs e)
    {
        if(ListSpotlight.Items.Count > 0)
        {
            var folderPicker = new FolderPicker();
            folderPicker.FileTypeFilter.Add("*");
            _mainWindow.InitializeWithWindow(folderPicker);
            var folder = await folderPicker.PickSingleFolderAsync();
            if (folder != null)
            {
                var exportingDialog = new ProgressDialog("Exporting all images...", async () => await _spotlightManager.ExportAllImagesAsync(folder.Path))
                {
                    XamlRoot = Content.XamlRoot
                };
                await exportingDialog.ShowAsync();
                _mainWindow.ShowInfoBarMessage(new InfoBarMessageInfo("Export Successful", $"Images saved to: {folder.Path}", InfoBarSeverity.Success));
            }
        }
    }

    /// <summary>
    /// Occurs when the set as background button is clicked
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">RoutedEventArgs</param>
    private void BtnSetAsBackground_Click(object sender, RoutedEventArgs e)
    {
        if(ListSpotlight.SelectedIndex != -1)
        {
            var result = _spotlightManager.SetImageAsDesktopBackground(ListSpotlight.SelectedIndex);
            var infoBarMessage = new InfoBarMessageInfo();
            if(result)
            {
                infoBarMessage.Title = "Desktop Background Set";
                infoBarMessage.Message = "Enjoy your new wallpaper!";
                infoBarMessage.Severity = InfoBarSeverity.Success;
            }
            else
            {
                infoBarMessage.Title = "Error";
                infoBarMessage.Message = "Unable to set the desktop background. Please try again. If the issue continues, file a bug report.";
                infoBarMessage.Severity = InfoBarSeverity.Error;
            }
            _mainWindow.ShowInfoBarMessage(infoBarMessage);
        }
    }

    /// <summary>
    /// Occurs when the selection of ListSpotlight changes
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">SelectionChangedEventArgs</param>
    private void ListSpotlight_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        BtnExportImage.IsEnabled = ListSpotlight.SelectedIndex != -1;
        BtnSetAsBackground.IsEnabled = ListSpotlight.SelectedIndex != -1;
        ImgSelected.Source = ListSpotlight.SelectedIndex != -1 ? new BitmapImage(new Uri(_spotlightManager.SpotlightImages[ListSpotlight.SelectedIndex])) : null;
    }
}
