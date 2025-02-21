#if (defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS))
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef MAINWINDOWCONTROLLER_H
#define MAINWINDOWCONTROLLER_H

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <libnick/app/appinfo.h>
#include <libnick/app/datafilemanager.h>
#include <libnick/app/windowgeometry.h>
#include <libnick/events/event.h>
#include <libnick/notifications/notificationsenteventargs.h>
#include <libnick/notifications/shellnotificationsenteventargs.h>
#include <libnick/taskbar/taskbaritem.h>
#include <libnick/update/updater.h>
#include "controllers/preferencesviewcontroller.h"
#include "events/imagessyncedeventargs.h"
#include "models/spotlightmanager.h"
#include "models/startupinformation.h"
#include "models/theme.h"
#include "models/viewmode.h"

namespace Nickvision::Spotlight::Shared::Controllers
{
    /**
     * @brief A controller for a MainWindow.
     */
    class MainWindowController
    {
    public:
        /**
         * @brief Constructs a MainWindowController.
         * @param args A list of argument strings for the application
         */
        MainWindowController(const std::vector<std::string>& args);
        /**
         * @brief Gets the Saved event for the application's configuration.
         * @return The configuration Saved event
         */
        Nickvision::Events::Event<Nickvision::Events::EventArgs>& configurationSaved();
        /**
         * @brief Gets the event for when a notification is sent.
         * @return The notification sent event
         */
        Nickvision::Events::Event<Nickvision::Notifications::NotificationSentEventArgs>& notificationSent();
        /**
         * @brief Gets the event for when a shell notification is sent.
         * @return The shell notification sent event
         */
        Nickvision::Events::Event<Nickvision::Notifications::ShellNotificationSentEventArgs>& shellNotificationSent();
        /**
         * @brief Gets the event for when images are synced.
         * @return The images synced event
         */
        Nickvision::Events::Event<Events::ImagesSyncedEventArgs>& imagesSynced();
        /**
         * @brief Gets the AppInfo object for the application
         * @return The current AppInfo object
         */
        const Nickvision::App::AppInfo& getAppInfo() const;
        /**
         * @brief Gets the preferred theme for the application.
         * @return The preferred theme
         */
        Models::Theme getTheme();
        /**
         * @brief Gets the debugging information for the application.
         * @param extraInformation Extra, ui-specific, information to include in the debug info statement
         * @return The application's debug information
         */
        std::string getDebugInformation(const std::string& extraInformation = "") const;
        /**
         * @brief Gets the number of spotlight images synced.
         * @return The number of spotlight images synced
         */
        size_t getSpotlightImageCount() const;
        /**
         * @brief Gets the path of a spotlight image.
         * @param index The index of the image
         * @return The path of the spotlight image
         */
        const std::filesystem::path& getSpotlightImagePath(int index) const;
        /**
         * @brief Gets whether or not the application can be shut down.
         * @return True if can shut down, else false
         */
        bool canShutdown() const;
        /**
         * @brief Gets a PreferencesViewController.
         * @return The PreferencesViewController
         */
        std::shared_ptr<PreferencesViewController> createPreferencesViewController();
        /**
         * @brief Starts the application.
         * @brief Will only have an effect on the first time called.
         * @return The WindowGeometry to use for the application window at startup
         */
        const Models::StartupInformation& startup(HWND hwnd);
        /**
         * @brief Shuts down the application.
         * @param geometry The window geometry to save
         * @param viewMode The view mode to save
         */
        void shutdown(const Nickvision::App::WindowGeometry& geometry, Models::ViewMode viewMode);
        /**
         * @brief Checks for an application update and sends a notification if one is available.
         */
        void checkForUpdates();
        /**
         * @brief Downloads and installs the latest application update in the background.
         * @brief Will send a notification if the update fails.
         * @brief MainWindowController::checkForUpdates() must be called before this method.
         */
        void windowsUpdate();
        /**
         * @brief Connects the main window to the taskbar interface.
         * @param hwnd The main window handle
         */
        void connectTaskbar(HWND hwnd);
        /**
         * @brief Clears and syncs the spotlight images.
         */
        void clearAndSync();
        /**
         * @brief Sets a spotlight image as the desktop background.
         * @param index The index of the image to set as the desktop background
         */
        void setImageAsDesktopBackground(int index);
        /**
         * @brief Exports a spotlight image.
         * @param index The index of the image to export
         * @param path The path to export the image to
         */
        void exportImage(int index, const std::filesystem::path& path);
        /**
         * @brief Exports all spotlight images.
         * @param path The path to export the images to
         */
        void exportAllImages(const std::filesystem::path& path);

    private:
        bool m_started;
        std::vector<std::string> m_args;
        Nickvision::App::AppInfo m_appInfo;
        Nickvision::App::DataFileManager m_dataFileManager;
        std::shared_ptr<Nickvision::Update::Updater> m_updater;
        Nickvision::Taskbar::TaskbarItem m_taskbar;
        Nickvision::Events::Event<Nickvision::Notifications::NotificationSentEventArgs> m_notificationSent;
        Nickvision::Events::Event<Nickvision::Notifications::ShellNotificationSentEventArgs> m_shellNotificationSent;
        Models::SpotlightManager m_spotlightManager;
        Nickvision::Events::Event<Events::ImagesSyncedEventArgs> m_imagesSynced;
    };
}

#endif //MAINWINDOWCONTROLLER_H
