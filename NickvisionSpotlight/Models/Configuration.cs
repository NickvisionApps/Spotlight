using Nickvision.Avalonia.Models;
using System;
using System.IO;
using System.Text.Json;
using System.Threading.Tasks;

namespace NickvisionSpotlight.Models
{
    public class Configuration
    {
        private static readonly string _configDir = $"{Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)}{Path.DirectorySeparatorChar}Nickvision{Path.DirectorySeparatorChar}NickvisionSpotlight";
        private static readonly string _configPath = $"{_configDir}{Path.DirectorySeparatorChar}config.json";

        public Theme Theme { get; set; }
        public AccentColor AccentColor { get; set; }

        public Configuration()
        {
            Theme = Environment.OSVersion.Platform == PlatformID.Win32NT ? Theme.System : Theme.Dark;
            AccentColor = Environment.OSVersion.Platform == PlatformID.Win32NT ? AccentColor.System : AccentColor.Blue;
        }

        public static Configuration Load()
        {
            try
            {
                var json = File.ReadAllText(_configPath);
                return JsonSerializer.Deserialize<Configuration>(json);
            }
            catch
            {
                return new Configuration();
            }
        }

        public static async Task<Configuration> LoadAsync()
        {
            try
            {
                var json = await File.ReadAllTextAsync(_configPath);
                return JsonSerializer.Deserialize<Configuration>(json);
            }
            catch
            {
                return new Configuration();
            }
        }

        public void Save()
        {
            var json = JsonSerializer.Serialize(this);
            if (!Directory.Exists(_configDir))
            {
                Directory.CreateDirectory(_configDir);
            }
            File.WriteAllText(_configPath, json);
        }

        public async Task SaveAsync()
        {
            var json = JsonSerializer.Serialize(this);
            if (!Directory.Exists(_configDir))
            {
                Directory.CreateDirectory(_configDir);
            }
            await File.WriteAllTextAsync(_configPath, json);
        }
    }
}
