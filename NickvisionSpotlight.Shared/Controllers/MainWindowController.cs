using NickvisionSpotlight.Models;
using NickvisionSpotlight.Shared.Events;
using NickvisionSpotlight.Shared.Helpers;
using NickvisionSpotlight.Shared.Models;
using System;
using System.Threading.Tasks;

namespace NickvisionSpotlight.Shared.Controllers;

/// <summary>
/// A controller for a MainWindow
/// </summary>
public class MainWindowController : IDisposable
{
    private bool _disposed;
    private SpotlightManager _spotlight;

    /// <summary>
    /// The localizer to get translated strings from
    /// </summary>
    public Localizer Localizer { get; init; }
    /// <summary>
    /// The path of the folder opened
    /// </summary>
    public string FolderPath { get; private set; }

    /// <summary>
    /// Gets the AppInfo object
    /// </summary>
    public AppInfo AppInfo => AppInfo.Current;
    /// <summary>
    /// Whether or not the version is a development version or not
    /// </summary>
    public bool IsDevVersion => AppInfo.Current.Version.IndexOf('-') != -1;
    /// <summary>
    /// The preferred theme of the application
    /// </summary>
    public Theme Theme => Configuration.Current.Theme;
    /// <summary>
    /// The number of spotlight images synced
    /// </summary>
    public int SpotlightImagesCount => _spotlight.SpotlightImages.Count;

    /// <summary>
    /// Occurs when a notification is sent
    /// </summary>
    public event EventHandler<NotificationSentEventArgs>? NotificationSent;

    /// <summary>
    /// Constructs a MainWindowController
    /// </summary>
    public MainWindowController()
    {
        _disposed = false;
        _spotlight = new SpotlightManager();
        Localizer = new Localizer();
        FolderPath = "No Folder Opened";
    }

    /// <summary>
    /// Whether or not to show a sun icon on the home page
    /// </summary>
    public bool ShowSun
    {
        get
        {
            var timeNowHours = DateTime.Now.Hour;
            return timeNowHours >= 6 && timeNowHours < 18;
        }
    }

    /// <summary>
    /// The string for greeting on the home page
    /// </summary>
    public string Greeting
    {
        get
        {
            var greeting = DateTime.Now.Hour switch
            {
                >= 0 and < 6 => "Night",
                < 12 => "Morning",
                < 18 => "Afternoon",
                < 24 => "Evening",
                _ => "Generic"
            };
            return Localizer["Greeting", greeting];
        }
    }

    /// <summary>
    /// Occurs when the spotlight images are changed
    /// </summary>
    public event EventHandler<EventArgs>? SpotlightImagesChanged
    {
        add
        {
            _spotlight.ImagesChanged += value;
        }

        remove
        {
            _spotlight.ImagesChanged -= value;
        }
    }

    /// <summary>
    /// Frees resources used by the MainWindowController object
    /// </summary>
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>
    /// Frees resources used by the MainWindowController object
    /// </summary>
    protected virtual void Dispose(bool disposing)
    {
        if (_disposed)
        {
            return;
        }
        if (disposing)
        {
            Localizer.Dispose();
        }
        _disposed = true;
    }

    /// <summary>
    /// Creates a new PreferencesViewController
    /// </summary>
    /// <returns>The PreferencesViewController</returns>
    public PreferencesViewController CreatePreferencesViewController() => new PreferencesViewController(Localizer);

    /// <summary>
    /// Scans the Windows Spotlight folder for images and adds them to the Nickvision Spotlight cached images folder, while populating the SpotlightImages list
    /// </summary>
    public async Task SyncSpotlightImagesAsync() => await _spotlight.SyncSpotlightImagesAsync();

    /// <summary>
    /// Exports a spotlight image
    /// </summary>
    /// <param name="index">The index of the image to export</param>
    /// <param name="path">The path of where to export the image to</param>
    public void ExportImage(int index, string path) => _spotlight.ExportImage(index, path);

    /// <summary>
    /// Exports all spotlight images to a directory
    /// </summary>
    /// <param name="path">The directory to export all images</param>
    public async Task ExportAllImagesAsync(string path) => await _spotlight.ExportAllImagesAsync(path);

    /// <summary>
    /// Sets a spotlight image as the desktop background
    /// </summary>
    /// <param name="index">The index of the image to use as the desktop background</param>
    /// <returns>True, unless there is an error with SystemParametersInfo</returns>
    public bool SetAsBackground(int index) => _spotlight.SetImageAsDesktopBackground(index);
}
