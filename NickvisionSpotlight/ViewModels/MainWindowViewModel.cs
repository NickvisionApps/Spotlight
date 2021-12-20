using Avalonia.Controls;
using Avalonia.Media.Imaging;
using FluentAvalonia.UI.Controls;
using Nickvision.Avalonia.Extensions;
using Nickvision.Avalonia.Models;
using Nickvision.Avalonia.MVVM;
using Nickvision.Avalonia.MVVM.Commands;
using Nickvision.Avalonia.MVVM.Services;
using Nickvision.Avalonia.Update;
using NickvisionSpotlight.Models;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Net.Http;
using System.Threading.Tasks;

namespace NickvisionSpotlight.ViewModels
{
    public class MainWindowViewModel : ViewModelBase
    {
        private ServiceCollection _serviceCollection;
        private HttpClient _httpClient;
        private bool _opened;
        private SpotlightManager _spotlightManager;
        private string _status;
        private string _selectedImage;
        private Bitmap _selectedImageSource;

        public bool IsImageNotSelected => string.IsNullOrEmpty(SelectedImage);
        public ObservableCollection<string> SpotlightImages { get; init; }
        public DelegateAsyncCommand<ICloseable> OpenedCommand { get; init; }
        public DelegateAsyncCommand<object> SaveCommand { get; init; }
        public DelegateAsyncCommand<object> SaveAllCommand { get; init; }
        public DelegateCommand<ICloseable> ExitCommand { get; init; }
        public DelegateAsyncCommand<object> SettingsCommand { get; init; }
        public DelegateAsyncCommand<object> SyncSpotlightImagesCommand { get; init; }
        public DelegateAsyncCommand<object> SetAsBackgroundCommand { get; init; }
        public DelegateAsyncCommand<ICloseable> CheckForUpdatesCommand { get; init; }
        public DelegateCommand<object> GitHubRepoCommand { get; init; }
        public DelegateCommand<object> ReportABugCommand { get; init; }
        public DelegateAsyncCommand<object> ChangelogCommand { get; init; }
        public DelegateAsyncCommand<object> AboutCommand { get; init; }

        public MainWindowViewModel(ServiceCollection serviceCollection)
        {
            Title = "Nickvision Spotlight";
            _serviceCollection = serviceCollection;
            _httpClient = new HttpClient();
            _opened = false;
            _spotlightManager = new SpotlightManager();
            SpotlightImages = new ObservableCollection<string>();
            SaveCommand = new DelegateAsyncCommand<object>(Save, () => !string.IsNullOrEmpty(SelectedImage));
            SaveAllCommand = new DelegateAsyncCommand<object>(SaveAll, () => SpotlightImages.Count > 0);
            OpenedCommand = new DelegateAsyncCommand<ICloseable>(Opened);
            ExitCommand = new DelegateCommand<ICloseable>(Exit);
            SettingsCommand = new DelegateAsyncCommand<object>(Settings);
            SyncSpotlightImagesCommand = new DelegateAsyncCommand<object>(SyncSpotlightImages);
            SetAsBackgroundCommand = new DelegateAsyncCommand<object>(SetAsBackground, () => !string.IsNullOrEmpty(SelectedImage));
            CheckForUpdatesCommand = new DelegateAsyncCommand<ICloseable>(CheckForUpdates);
            GitHubRepoCommand = new DelegateCommand<object>(GitHubRepo);
            ReportABugCommand = new DelegateCommand<object>(ReportABug);
            ChangelogCommand = new DelegateAsyncCommand<object>(Changelog);
            AboutCommand = new DelegateAsyncCommand<object>(About);
        }

        public string Status
        {
            get => _status;

            set => SetProperty(ref _status, value);
        }

        public string SelectedImage
        {
            get => _selectedImage;

            set
            {
                SetProperty(ref _selectedImage, value);
                OnPropertyChanged("IsImageNotSelected");
                foreach(var image in _spotlightManager.SpotlightImages)
                {
                    if(image.Filename == value)
                    {
                        SelectedImageSource = new Bitmap(image.Path);
                    }
                }
                SetAsBackgroundCommand.RaiseCanExecuteChanged();
                SaveCommand.RaiseCanExecuteChanged();
            }
        }

        public Bitmap SelectedImageSource
        {
            get => _selectedImageSource;

            set => SetProperty(ref _selectedImageSource, value);
        }

        private async Task Opened(ICloseable window)
        {
            if (!_opened)
            {
                if(Environment.OSVersion.Platform != PlatformID.Win32NT)
                {
                    await _serviceCollection.GetService<IContentDialogService>().ShowMessageAsync(new ContentDialogInfo()
                    {
                        Title = "Platform Not Supported",
                        Description = "This application can only run on Windows.",
                        CloseButtonText = "OK",
                        DefaultButton = ContentDialogButton.Close
                    });
                    window.Close();
                }
                var configuration = await Configuration.LoadAsync();
                try
                {
                    _serviceCollection.GetService<IThemeService>().ForceNativeTitleBarTheme();
                }
                catch { }
                _serviceCollection.GetService<IThemeService>().ChangeTheme(configuration.Theme);
                _serviceCollection.GetService<IThemeService>().ChangeAccentColor(configuration.AccentColor);
                Status = "Total Number of Images: 0";
                _opened = true;
            }
        }

        private async Task Save(object parameter)
        {
            var fileTypeFilters = new List<FileDialogFilter>();
            fileTypeFilters.Add(new FileDialogFilter()
            {
                Name = "JPEG",
                Extensions = new List<string>() { ".jpg" }
            });
            var result = await _serviceCollection.GetService<IIOService>().ShowSaveFileDialogAsync("Save Image", fileTypeFilters, ".jpg");
            if(result != null)
            {
                await _spotlightManager.ExportImageAsync(SelectedImage, result);
            }
        }

        private async Task SaveAll(object parameter)
        {
            var result = await _serviceCollection.GetService<IIOService>().ShowOpenFolderDialogAsync("Select Image Folder");
            if (result != null)
            {
                await _serviceCollection.GetService<IProgressDialogService>().ShowAsync("Saving all images...", async () => await _spotlightManager.ExportAllImagesAsync(result));
                _serviceCollection.GetService<IInfoBarService>().ShowCloseableNotification("Export Successful", $"Images saved to {result}.", InfoBarSeverity.Success);
            }
        }

        private void Exit(ICloseable window) => window.Close();

        private async Task Settings(object parameter) => await _serviceCollection.GetService<IContentDialogService>().ShowCustomAsync(new SettingsDialogViewModel(_serviceCollection));

        private async Task SyncSpotlightImages(object parameter)
        {
            SpotlightImages.Clear();
            SelectedImageSource = null;
            await _serviceCollection.GetService<IProgressDialogService>().ShowAsync("Syncing spotlight images...", async () => await _spotlightManager.SyncSpotlightImagesAsync());
            foreach(var image in _spotlightManager.SpotlightImages)
            {
                SpotlightImages.Add(image.Filename);
            }
            Status = $"Total Number of Images: {SpotlightImages.Count}";
            SaveAllCommand.RaiseCanExecuteChanged();
        }

        private async Task SetAsBackground(object parameter) => await _spotlightManager.SetImageAsBackgroundAsync(SelectedImage);

        private async Task CheckForUpdates(ICloseable window)
        {
            var updater = new Updater(_httpClient, new Uri("https://raw.githubusercontent.com/nlogozzo/NickvisionSpotlight/main/UpdateConfig.json"), new Version("2021.12.1"));
            await _serviceCollection.GetService<IProgressDialogService>().ShowAsync("Checking for updates...", async () => await updater.CheckForUpdatesAsync());
            if (updater.UpdateAvailable)
            {
                if (Environment.OSVersion.Platform == PlatformID.Win32NT)
                {
                    var result = await _serviceCollection.GetService<IContentDialogService>().ShowMessageAsync(new ContentDialogInfo()
                    {
                        Title = "Update Available",
                        Description = $"===V{updater.LatestVersion} Changelog===\n{updater.Changelog}\n\nNickvision Spotlight will automatically download and install the update, please save all work before continuing. Are you ready to update?",
                        PrimaryButtonText = "Yes",
                        CloseButtonText = "No",
                        DefaultButton = ContentDialogButton.Close
                    });
                    if (result == ContentDialogResult.Primary)
                    {
                        await _serviceCollection.GetService<IProgressDialogService>().ShowAsync("Downloading and installing the update...", async () => await updater.WindowsUpdateAsync(window));
                    }
                }
                else
                {
                    await _serviceCollection.GetService<IContentDialogService>().ShowMessageAsync(new ContentDialogInfo()
                    {
                        Title = "Update Available",
                        Description = $"===V{updater.LatestVersion} Changelog===\n{updater.Changelog}\n\nPlease visit the GitHub repo to download the latest release.",
                        CloseButtonText = "OK",
                        DefaultButton = ContentDialogButton.Close
                    });
                }
            }
            else
            {
                _serviceCollection.GetService<IInfoBarService>().ShowCloseableNotification("No Update Available", "There is no update at this time. Please try again later.", InfoBarSeverity.Error);
            }
        }

        private void GitHubRepo(object parameter) => new Uri("https://github.com/nlogozzo/NickvisionSpotlight").OpenInBrowser();

        private void ReportABug(object parameter) => new Uri("https://github.com/nlogozzo/NickvisionSpotlight/issues/new").OpenInBrowser();

        private async Task Changelog(object parameter)
        {
            await _serviceCollection.GetService<IContentDialogService>().ShowMessageAsync(new ContentDialogInfo()
            {
                Title = "What's New?",
                Description  = "- Design Tweaks\n- Fixed an issue where the old image stayed selected after a re-sync",
                CloseButtonText = "OK",
                DefaultButton = ContentDialogButton.Close
            });
        }

        private async Task About(object parameter)
        {
            await _serviceCollection.GetService<IContentDialogService>().ShowMessageAsync(new ContentDialogInfo()
            {
                Title = "About",
                Description  = "Nickvision Spotlight Version 2021.12.1\nA utility for working with Windows Spotlight images.\n\nUsing Avalonia and .NET 6",
                CloseButtonText = "OK",
                DefaultButton = ContentDialogButton.Close
            });
        }
    }
}
