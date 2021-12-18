using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using FluentAvalonia.UI.Controls;
using Nickvision.Avalonia.MVVM;
using Nickvision.Avalonia.MVVM.Services;
using NickvisionSpotlight.ViewModels;

namespace NickvisionSpotlight.Views
{
    public partial class MainWindowView : Window, ICloseable
    {
        public MainWindowView()
        {
            AvaloniaXamlLoader.Load(this);
            var serviceCollection = new ServiceCollection();
            serviceCollection.AddService(new ThemeService(this));
            serviceCollection.AddService(new IOService(this));
            serviceCollection.AddService(new ContentDialogService());
            serviceCollection.AddService(new ProgressDialogService());
            serviceCollection.AddService(new InfoBarService(this.FindControl<InfoBar>("InfoBar")));
            var viewModel = new MainWindowViewModel(serviceCollection);
            DataContext = viewModel;
        }
    }
}