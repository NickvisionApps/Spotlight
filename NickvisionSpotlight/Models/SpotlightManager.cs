using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace NickvisionSpotlight.Models
{
    public class SpotlightManager
    {
        [DllImport("user32.dll", CharSet = CharSet.Auto)]
        static extern int SystemParametersInfo(int uAction, int uParam, string lpvParam, int fuWinIni);

        private string _dataDir;
        private string _spotlightDir;

        public List<(string Path, string Filename)> SpotlightImages { get; init; }

        public SpotlightManager()
        {
            _dataDir = $"{Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)}\\Nickvision\\NickvisionSpotlight\\Images";
            _spotlightDir = $"{Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData)}\\Packages\\Microsoft.Windows.ContentDeliveryManager_cw5n1h2txyewy\\LocalState\\Assets";
            SpotlightImages = new List<(string Path, string Filename)>();
            if (!Directory.Exists(_spotlightDir))
            {
                throw new ApplicationException("Unable to access Windows Spotlight images directory.");
            }
            if (!Directory.Exists(_dataDir))
            {
                Directory.CreateDirectory(_dataDir);
            }
        }

        public async Task SyncSpotlightImagesAsync()
        {
            SpotlightImages.Clear();
            await Task.Run(() =>
            {
                foreach (var file in Directory.EnumerateFiles(_spotlightDir, "*.*", SearchOption.TopDirectoryOnly))
                {
                    var fileInfo = new FileInfo(file);
                    if (fileInfo.Length / 1000 >= 200) //actual backgrounds are always > 200 KB
                    {
                        var newPath = $"{_dataDir}\\{Path.GetFileName(file)}.jpg";
                        File.Copy(file, newPath, true);
                        SpotlightImages.Add((newPath, Path.GetFileName(newPath)));
                    }
                }
            });
        }

        public async Task ExportImageAsync(string selectedFilename, string savePath)
        {
            await Task.Run(() =>
            {
                foreach (var image in SpotlightImages)
                {
                    if (image.Filename == selectedFilename)
                    {
                        File.Copy(image.Path, savePath, true);
                        break;
                    }
                }
            });
        }

        public async Task ExportAllImagesAsync(string directory)
        {
            await Task.Run(() =>
            {
                foreach (var image in SpotlightImages)
                {
                    File.Copy(image.Path, $"{directory}\\{image.Filename}", true);
                }
            });
        }

        public async Task SetImageAsBackgroundAsync(string selectedFilename)
        {
            var SPI_SETDESKWALLPAPER = 20;
            var SPIF_UPDATEINIFILE = 0x01;
            var SPIF_SENDWININICHANGE = 0x02;
            await Task.Run(() =>
            {
                foreach (var image in SpotlightImages)
                {
                    if (image.Filename == selectedFilename)
                    {
                        var backgroundPath = $"{Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)}\\Nickvision\\NickvisionSpotlight\\background.jpg";
                        File.Copy(image.Path, backgroundPath, true);
                        var key = Registry.CurrentUser.OpenSubKey(@"Control Panel\Desktop", true);
                        key.SetValue(@"WallpaperStyle", 2.ToString());
                        key.SetValue(@"TileWallpaper", 0.ToString());
                        SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, backgroundPath, SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
                        break;
                    }
                }
            });
        }
    }
}
