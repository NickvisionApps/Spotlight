using System;

namespace NickvisionSpotlight.Models
{
    /// <summary>
    /// A model for the information about the application
    /// </summary>
    public class AppInfo
    {
        private static AppInfo? _instance;

        public static AppInfo Current => _instance ??= new AppInfo();

        public string Name { get; set; }
        public string Description { get; set; }
        public Version Version { get; set; }
        public string Changelog { get; set; }
        public Uri GitHubRepo { get; set; }
        public Uri IssueTracker { get; set; }

        /// <summary>
        /// Constructs an AppInfo
        /// </summary>
        private AppInfo()
        {
            Name = "";
            Description = "";
            Version = new Version("0.0.0");
            Changelog = "";
            GitHubRepo = new Uri("about:blank");
            IssueTracker = new Uri("about:blank");
        }
    }
}