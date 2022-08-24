using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using NickvisionSpotlight.Models;

namespace NickvisionSpotlight.UI.Views;

/// <summary>
/// The settings page for the application
/// </summary>
public sealed partial class SettingsPage : Page
{
    /// <summary>
    /// Constructs a SettingsPage
    /// </summary>
    public SettingsPage()
    {
        InitializeComponent();
        //Load Config
        if (Configuration.Current.Theme == Theme.Light)
        {
            BtnLight.IsChecked = true;
        }
        else if (Configuration.Current.Theme == Theme.Dark)
        {
            BtnDark.IsChecked = true;
        }
        else if (Configuration.Current.Theme == Theme.System)
        {
            BtnSystem.IsChecked = true;
        }
    }

    /// <summary>
    /// Occurs when a theme button is clicked
    /// </summary>
    /// <param name="sender">object</param>
    /// <param name="e">RoutedEventArgs</param>
    private void BtnTheme_Click(object sender, RoutedEventArgs e)
    {
        if (BtnLight.IsChecked!.Value)
        {
            Configuration.Current.Theme = Theme.Light;
        }
        else if (BtnDark.IsChecked!.Value)
        {
            Configuration.Current.Theme = Theme.Dark;
        }
        else if (BtnSystem.IsChecked!.Value)
        {
            Configuration.Current.Theme = Theme.System;
        }
        Configuration.Current.Save();
    }
}
