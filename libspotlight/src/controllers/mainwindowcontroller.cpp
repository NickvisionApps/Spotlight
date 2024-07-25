#include "controllers/mainwindowcontroller.h"
#include <algorithm>
#include <ctime>
#include <format>
#include <locale>
#include <sstream>
#include <thread>
#include <libnick/filesystem/userdirectories.h>
#include <libnick/helpers/codehelpers.h>
#include <libnick/helpers/stringhelpers.h>
#include <libnick/localization/gettext.h>
#include <libnick/system/environment.h>
#include <windows.h>
#include "models/configuration.h"

using namespace Nickvision::App;
using namespace Nickvision::Events;
using namespace Nickvision::Filesystem;
using namespace Nickvision::Helpers;
using namespace Nickvision::Notifications;
using namespace Nickvision::Spotlight::Shared::Models;
using namespace Nickvision::System;
using namespace Nickvision::Update;

namespace Nickvision::Spotlight::Shared::Controllers
{
    MainWindowController::MainWindowController(const std::vector<std::string>& args)
        : m_started{ false },
        m_args{ args },
        m_appInfo{ "org.nickvision.spotlight", "Nickvision Spotlight", "Spotlight" },
        m_dataFileManager{ m_appInfo.getName() },
        m_logger{ UserDirectories::get(ApplicationUserDirectory::LocalData, m_appInfo.getName()) / "log.txt", Logging::LogLevel::Info, false },
        m_spotlightManager{ m_appInfo.getName(), m_logger }
    {
        m_appInfo.setVersion({ "2024.7.0-next" });
        m_appInfo.setShortName(_("Spotlight"));
        m_appInfo.setDescription(_("Find your favorite Windows spotlight images"));
        m_appInfo.setChangelog("- Updated dependencies");
        m_appInfo.setSourceRepo("https://github.com/NickvisionApps/Spotlight");
        m_appInfo.setIssueTracker("https://github.com/NickvisionApps/Spotlight/issues/new");
        m_appInfo.setSupportUrl("https://github.com/NickvisionApps/Spotlight/discussions");
        m_appInfo.getExtraLinks()[_("Matrix Chat")] = "https://matrix.to/#/#nickvision:matrix.org";
        m_appInfo.getDevelopers()["Nicholas Logozzo"] = "https://github.com/nlogozzo";
        m_appInfo.getDevelopers()[_("Contributors on GitHub")] = "https://github.com/NickvisionApps/Spotlight/graphs/contributors";
        m_appInfo.getDesigners()["Nicholas Logozzo"] = "https://github.com/nlogozzo";
        m_appInfo.getDesigners()[_("Fyodor Sobolev")] = "https://github.com/fsobolev";
        m_appInfo.getDesigners()["DaPigGuy"] = "https://github.com/DaPigGuy";
        m_appInfo.getArtists()[_("David Lapshin")] = "https://github.com/daudix";
        m_appInfo.setTranslatorCredits(_("translator-credits"));
        m_updater = std::make_shared<Updater>(m_appInfo.getSourceRepo());
        m_dataFileManager.get<Configuration>("config").saved() += [this](const EventArgs&)
        {
            m_logger.log(Logging::LogLevel::Debug, "Configuration saved.");
        };
    }

    Event<EventArgs>& MainWindowController::configurationSaved()
    {
        return m_dataFileManager.get<Configuration>("config").saved();
    }

    Event<NotificationSentEventArgs>& MainWindowController::notificationSent()
    {
        return m_notificationSent;
    }

    Event<ShellNotificationSentEventArgs>& MainWindowController::shellNotificationSent()
    {
        return m_shellNotificationSent;
    }

    Event<EventArgs>& MainWindowController::imagesSynced()
    {
        return m_imagesSynced;
    }

    const AppInfo& MainWindowController::getAppInfo() const
    {
        return m_appInfo;
    }

    Theme MainWindowController::getTheme()
    {
        return m_dataFileManager.get<Configuration>("config").getTheme();
    }

    ViewMode MainWindowController::getViewMode()
    {
        return m_dataFileManager.get<Configuration>("config").getViewMode();
    }

    std::string MainWindowController::getDebugInformation(const std::string& extraInformation) const
    {
        std::stringstream builder;
        switch(m_spotlightManager.getSupportLevel())
        {
        case SpotlightSupport::Full:
            builder << "Spotlight Support Level: Full" << std::endl;
            break;
        case SpotlightSupport::LockScreenOnly:
            builder << "Spotlight Support Level: Lock Screen Only" << std::endl;
            break;
        case SpotlightSupport::DesktopOnly:
            builder << "Spotlight Support Level: Desktop Only" << std::endl;
            break;
        default:
            builder << "Spotlight Support Level: None" << std::endl;
            break;
        }
        if (!extraInformation.empty())
        {
            builder << std::endl << extraInformation << std::endl;
        }
        return Environment::getDebugInformation(m_appInfo, builder.str());
    }

    const std::vector<std::filesystem::path>& MainWindowController::getSpotlightImages() const
    {
        return m_spotlightManager.getImages();
    }

    bool MainWindowController::canShutdown() const
    {
        return true;
    }

    std::shared_ptr<PreferencesViewController> MainWindowController::createPreferencesViewController()
    {
        return std::make_shared<PreferencesViewController>(m_dataFileManager.get<Configuration>("config"));
    }

    Nickvision::App::WindowGeometry MainWindowController::startup(HWND hwnd)
    {
        if (m_started)
        {
            return m_dataFileManager.get<Configuration>("config").getWindowGeometry();
        }
        if(m_taskbar.connect(hwnd))
        {
            m_logger.log(Logging::LogLevel::Debug, "Connected to Windows taskbar.");
        }
        else
        {
            m_logger.log(Logging::LogLevel::Error, "Unable to connect to Windows taskbar.");
        }
        if (m_dataFileManager.get<Configuration>("config").getAutomaticallyCheckForUpdates())
        {
            checkForUpdates();
        }
        std::thread syncWorker{ [this]()
        {
            m_spotlightManager.sync();
            m_imagesSynced.invoke({});
        } };
        syncWorker.detach();
        m_started = true;
        m_logger.log(Logging::LogLevel::Debug, "MainWindow started.");
        return m_dataFileManager.get<Configuration>("config").getWindowGeometry();
    }

    void MainWindowController::shutdown(const WindowGeometry& geometry, ViewMode viewMode)
    {
        Configuration& config{ m_dataFileManager.get<Configuration>("config") };
        config.setWindowGeometry(geometry);
        config.setViewMode(viewMode);
        config.save();
        m_logger.log(Logging::LogLevel::Debug, "MainWindow shutdown.");
    }

    void MainWindowController::checkForUpdates()
    {
        if(!m_updater)
        {
            return;
        }
        m_logger.log(Logging::LogLevel::Debug, "Checking for updates...");
        std::thread worker{ [this]()
        {
            Version latest{ m_updater->fetchCurrentVersion(VersionType::Stable) };
            if (!latest.empty())
            {
                if (latest > m_appInfo.getVersion())
                {
                    m_logger.log(Logging::LogLevel::Info, "Update found: " + latest.str());
                    m_notificationSent.invoke({ _("New update available"), NotificationSeverity::Success, "update" });
                }
                else
                {
                    m_logger.log(Logging::LogLevel::Debug, "No updates found.");
                }
            }
            else
            {
                m_logger.log(Logging::LogLevel::Warning, "Unable to fetch latest app version.");
            }
        } };
        worker.detach();
    }

    void MainWindowController::windowsUpdate()
    {
        if(m_updater)
        {
            return;
        }
        m_logger.log(Logging::LogLevel::Debug, "Fetching Windows app update...");
        std::thread worker{ [this]()
        {
            if (m_updater->windowsUpdate(VersionType::Stable))
            {
                m_logger.log(Logging::LogLevel::Info, "Windows app update started.");
            }
            else
            {
                m_logger.log(Logging::LogLevel::Error, "Unable to fetch Windows app update.");
                m_notificationSent.invoke({ _("Unable to download and install update"), NotificationSeverity::Error, "error" });
            }
        } };
        worker.detach();
    }

    void MainWindowController::log(Logging::LogLevel level, const std::string& message, const std::source_location& source)
    {
        m_logger.log(level, message, source);
    }

    void MainWindowController::setImageAsDesktopBackground(int index)
    {
        if(m_spotlightManager.setAsDesktopBackground(static_cast<size_t>(index)))
        {
            m_notificationSent.invoke({ _("Image set as desktop background"), NotificationSeverity::Success });
        }
        else
        {
            m_notificationSent.invoke({ _("Unable to set image as desktop background"), NotificationSeverity::Error });
        }
    }

    void MainWindowController::exportImage(int index, const std::filesystem::path& path)
    {
        if(m_spotlightManager.exportImage(static_cast<size_t>(index), path))
        {
            m_notificationSent.invoke({ std::vformat(_("Image exported to {}"), std::make_format_args(CodeHelpers::unmove(path.string()))), NotificationSeverity::Success });
        }
        else
        {
            m_notificationSent.invoke({ _("Unable to export image"), NotificationSeverity::Error });
        }
    }

    void MainWindowController::exportAllImages(const std::filesystem::path& path)
    {
        if(m_spotlightManager.exportAllImages(path))
        {
            m_notificationSent.invoke({ std::vformat(_("Images exported to {}"), std::make_format_args(CodeHelpers::unmove(path.string()))), NotificationSeverity::Success });
        }
        else
        {
            m_notificationSent.invoke({ _("Unable to export images"), NotificationSeverity::Error });
        }
    }
}
