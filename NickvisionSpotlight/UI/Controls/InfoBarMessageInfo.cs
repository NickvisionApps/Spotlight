﻿using Microsoft.UI.Xaml.Controls;

namespace NickvisionSpotlight.UI.Controls;

/// <summary>
/// A model of the properties to show in an InfoBar
/// </summary>
public class InfoBarMessageInfo
{
    /// <summary>
    /// The title of the InfoBar
    /// </summary>
    public string Title { get; set; }
    /// <summary>
    /// The message of the InfoBar
    /// </summary>
    public string Message { get; set; }
    /// <summary>
    /// The severity of the InfoBar
    /// </summary>
    public InfoBarSeverity Severity { get; set; }

    /// <summary>
    /// Constructs an InfoBarMessageInfo
    /// </summary>
    /// <param name="title">The title of the InfoBar</param>
    /// <param name="message">The message of the InfoBar</param>
    /// <param name="severity">The severity of the InfoBar</param>
    public InfoBarMessageInfo(string title = "", string message = "", InfoBarSeverity severity = InfoBarSeverity.Informational)
    {
        Title = title;
        Message = message;
        Severity = severity;
    }
}