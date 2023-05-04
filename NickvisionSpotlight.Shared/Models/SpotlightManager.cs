using NickvisionSpotlight.Shared.Models;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Threading.Tasks;
using Vanara.PInvoke;

namespace NickvisionSpotlight.Models;

/// <summary>
/// A class for managing Windows Spotlight images
/// </summary>
public class SpotlightManager
{
    private string _spotlightDir;
    private string _dataDir;

    /// <summary>
    /// A list of paths of cached spotlight images
    /// </summary>
    public List<string> SpotlightImages { get; init; }

    /// <summary>
    /// Occurs when the list of spotlight images is changed
    /// </summary>
    public event EventHandler<EventArgs>? ImagesChanged;

    /// <summary>
    /// Constructs a SpotlightManager
    /// </summary>
    /// <exception cref="ApplicationException">Thrown if the Windows Spotlight folder does not exist</exception>
    public SpotlightManager()
    {
        _spotlightDir = $"{Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)}\\Packages\\Microsoft.Windows.ContentDeliveryManager_cw5n1h2txyewy\\LocalState\\Assets\\";
        _dataDir = $"{Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)}{Path.DirectorySeparatorChar}Nick vision{Path.DirectorySeparatorChar}{AppInfo.Current.Name}{Path.DirectorySeparatorChar}Images{Path.DirectorySeparatorChar}";
        SpotlightImages = new List<string>();
        if (!Directory.Exists(_spotlightDir))
        {
            throw new ApplicationException("Unable to find Windows Spotlight images directory.");
        }
        if (!Directory.Exists(_dataDir))
        {
            Directory.CreateDirectory(_dataDir);
        }
    }

    /// <summary>
    /// Scans the Windows Spotlight folder for images and adds them to the Nickvision Spotlight cached images folder, while populating the SpotlightImages list
    /// </summary>
    public async Task SyncSpotlightImagesAsync()
    {
        SpotlightImages.Clear();
        await Task.Run(() =>
        {
            foreach (var path in Directory.EnumerateFiles(_spotlightDir))
            {
                if (new FileInfo(path).Length / 1000 >= 200)
                {
                    var newPath = $"{_dataDir}{Path.GetFileNameWithoutExtension(path)}.jpg";
                    File.Copy(path, newPath, true);
                }
            }
            foreach (var path in Directory.EnumerateFiles(_dataDir))
            {
                using var image = Image.FromFile(path);
                if (image.Width > image.Height)
                {
                    SpotlightImages.Add(path);
                }
                else
                {
                    image.Dispose();
                    File.Delete(path);
                }
            }
        });
        ImagesChanged?.Invoke(this, EventArgs.Empty);
    }

    /// <summary>
    /// Exports a spotlight image
    /// </summary>
    /// <param name="index">The index of the image to export</param>
    /// <param name="savePath">The path of where to export the image to</param>
    /// <exception cref="IndexOutOfRangeException">Thrown if index is invalid</exception>
    public void ExportImage(int index, string savePath)
    {
        if (index >= 0 && index < SpotlightImages.Count)
        {
            File.Copy(SpotlightImages[index], savePath, true);
        }
        else
        {
            throw new IndexOutOfRangeException("The index provided is out of bounds of the SpotlightImages list.");
        }
    }

    /// <summary>
    /// Exports all spotlight images to a directory
    /// </summary>
    /// <param name="saveDir">The directory to export all images</param>
    public async Task ExportAllImagesAsync(string saveDir)
    {
        await Task.Run(() =>
        {
            foreach (var path in SpotlightImages)
            {
                var copyPath = $"{saveDir}{Path.DirectorySeparatorChar}{Path.GetFileName(path)}";
                File.Copy(path, copyPath, true);
            }
        });
    }

    /// <summary>
    /// Sets a spotlight image as the desktop background
    /// </summary>
    /// <param name="index">The index of the image to use as the desktop background</param>
    /// <returns>True, unless there is an error with SystemParametersInfo</returns>
    /// <exception cref="IndexOutOfRangeException">Thrown if index is invalid</exception>
    public bool SetImageAsDesktopBackground(int index)
    {
        if (index >= 0 && index < SpotlightImages.Count)
        {
            var backgroundPath = $"{Environment.GetFolderPath(Environment.SpecialFolder.MyPictures)}{Path.DirectorySeparatorChar}background.jpg";
            File.Copy(SpotlightImages[index], backgroundPath, true);
            return User32.SystemParametersInfo(User32.SPI.SPI_SETDESKWALLPAPER, 0, backgroundPath, User32.SPIF.SPIF_UPDATEINIFILE | User32.SPIF.SPIF_SENDWININICHANGE);
        }
        else
        {
            throw new IndexOutOfRangeException("The index provided is out of bounds of the SpotlightImages list.");
        }
    }
}