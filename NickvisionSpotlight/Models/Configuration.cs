﻿using System;
using System.IO;
using System.Text.Json;

namespace NickvisionSpotlight.Models
{
    /// <summary>
    /// A model for the configuration of the application
    /// </summary>
    public class Configuration
    {
        private static readonly string ConfigDir = $"{Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)}{Path.DirectorySeparatorChar}Nickvision{Path.DirectorySeparatorChar}{AppInfo.Current.Name}";
        private static readonly string ConfigPath = $"{ConfigDir}{Path.DirectorySeparatorChar}config.json";
        private static Configuration? _instance;

        public Theme Theme { get; set; }

        /// <summary>
        /// Constructs a Configuration
        /// </summary>
        public Configuration()
        {
            if (!Directory.Exists(ConfigDir))
            {
                Directory.CreateDirectory(ConfigDir);
            }
            Theme = Theme.System;
        }

        public static Configuration Current
        {
            get
            {
                if (_instance == null)
                {
                    try
                    {
                        _instance = JsonSerializer.Deserialize<Configuration>(File.ReadAllText(ConfigPath)) ?? new Configuration();
                    }
                    catch
                    {
                        _instance = new Configuration();
                    }
                }
                return _instance;
            }
        }

        /// <summary>
        /// Saves the configuration to disk
        /// </summary>
        public void Save() => File.WriteAllText(ConfigPath, JsonSerializer.Serialize(this));
    }
}