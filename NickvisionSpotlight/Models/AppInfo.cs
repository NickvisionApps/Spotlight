using System;

namespace NickvisionSpotlight.Models;

/// <summary>
/// A model for the information about the application
/// </summary>
public class AppInfo
{
    private static AppInfo? _instance;

    /// <summary>
    /// Gets the singleton object
    /// </summary>
    public static AppInfo Current => _instance ??= new AppInfo();

    /// <summary>
    /// The name of the application
    /// </summary>
    public string Name { get; set; }
    /// <summary>
    /// The description of the application
    /// </summary>
    public string Description { get; set; }
    /// <summary>
    /// The running version of the application
    /// </summary>
    public Version Version { get; set; }
    /// <summary>
    /// The changelog for the running version of the application
    /// </summary>
    public string Changelog { get; set; }
    /// <summary>
    /// The GitHub repo for the application
    /// </summary>
    public Uri GitHubRepo { get; set; }
    /// <summary>
    /// The issue tracker for the application
    /// </summary>
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