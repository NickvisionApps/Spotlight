using Microsoft.UI.Xaml;
using NickvisionSpotlight.Models;
using NickvisionSpotlight.UI;
using NickvisionSpotlight.UI.Controls;
using NickvisionSpotlight.UI.Views;
using System;
using System.Threading.Tasks;

namespace NickvisionSpotlight
{
    /// <summary>
    /// The Application
    /// </summary>
    public partial class App : Application
    {
        private MainWindow? _window;

        /// <summary>
        /// Constructs an App
        /// </summary>
        public App()
        {
            InitializeComponent();
            //AppInfo
            AppInfo.Current.Name = "Nickvision Spotlight";
            AppInfo.Current.Description = "A utility for working with Windows Spotlight images.";
            AppInfo.Current.Version = new Version("2022.8.0");
            AppInfo.Current.Changelog = "- Rewrite with Windows App SDK and C#";
            AppInfo.Current.GitHubRepo = new Uri("https://github.com/nlogozzo/NickvisionSpotlight");
            AppInfo.Current.IssueTracker = new Uri("https://github.com/nlogozzo/NickvisionSpotlight/issues/new");
            //Load Config
            if (Configuration.Current.Theme == Theme.Light)
            {
                RequestedTheme = ApplicationTheme.Light;
            }
            else if (Configuration.Current.Theme == Theme.Dark)
            {
                RequestedTheme = ApplicationTheme.Dark;
            }
        }

        /// <summary>
        /// Occurs when the application is launched
        /// </summary>
        /// <param name="args">LaunchActivatedEventArgs</param>
        protected override async void OnLaunched(LaunchActivatedEventArgs args)
        {
            _window = new MainWindow();
            var windowContent = _window.Content;
            var splashScreen = new SplashScreen();
            _window.Content = splashScreen;
            _window.Activate();
            await StartupAsync(splashScreen);
            _window.Content = windowContent;
        }

        private async Task StartupAsync(SplashScreen splashScreen)
        {
            splashScreen.Description = "Loading application...";
            await Task.Delay(500);
            splashScreen.Description = "Syncing spotlight images...";
            await Messenger.Current.SendAsync("MainWindow.SyncSpotlightImagesAsync");
        }
    }
}