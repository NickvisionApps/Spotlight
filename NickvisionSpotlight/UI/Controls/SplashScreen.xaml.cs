using Microsoft.UI.Xaml.Controls;

namespace NickvisionSpotlight.UI.Controls;

/// <summary>
/// A control for displaying a splash screen
/// </summary>
public sealed partial class SplashScreen : UserControl
{
    /// <summary>
    /// Constructs a SplashScreen
    /// </summary>
    public SplashScreen()
    {
        InitializeComponent();
    }

    /// <summary>
    /// Text to show on the splash screen
    /// </summary>
    public string Description
    {
        get => LblDescription.Text;

        set => LblDescription.Text = value;
    }
}