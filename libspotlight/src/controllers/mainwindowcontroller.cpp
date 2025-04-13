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
#include <libnick/notifications/appnotification.h>
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
        m_spotlightManager{ m_appInfo.getName() }
    {
        m_appInfo.setVersion({ "2025.4.0-next" });
        m_appInfo.setShortName(_("Spotlight"));
        m_appInfo.setDescription(_("Find your favorite Windows spotlight images"));
        m_appInfo.setChangelog("- Added the ability to clear the spotlight cache and resync images\n- Fixed an issue where the application could not update itself\n- Improved the design of the application");
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
        Localization::Gettext::init(m_appInfo.getEnglishShortName());
        m_updater = std::make_shared<Updater>(m_appInfo.getSourceRepo());
    }

    Event<EventArgs>& MainWindowController::configurationSaved()
    {
        return m_dataFileManager.get<Configuration>("config").saved();
    }

    Event<NotificationSentEventArgs>& MainWindowController::notificationSent()
    {
        return AppNotification::sent();
    }
    Event<ParamEventArgs<std::vector<std::filesystem::path>>>& MainWindowController::imagesSynced()
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

    size_t MainWindowController::getSpotlightImageCount() const
    {
        return m_spotlightManager.getImages().size();
    }

    const std::filesystem::path& MainWindowController::getSpotlightImagePath(int index) const
    {
        static std::filesystem::path empty;
        if(index < 0 || index > m_spotlightManager.getImages().size())
        {
            return empty;
        }
        return m_spotlightManager.getImages()[index];
    }

    bool MainWindowController::canShutdown() const
    {
        return true;
    }

    std::shared_ptr<PreferencesViewController> MainWindowController::createPreferencesViewController()
    {
        return std::make_shared<PreferencesViewController>(m_dataFileManager.get<Configuration>("config"));
    }

    const StartupInformation& MainWindowController::startup(HWND hwnd)
    {
        static StartupInformation info;
        if (m_started)
        {
            return info;
        }
        //Load configuration
        info.setWindowGeometry(m_dataFileManager.get<Configuration>("config").getWindowGeometry());
        //Load taskbar item
        m_taskbar.connect(hwnd);
        if (m_dataFileManager.get<Configuration>("config").getAutomaticallyCheckForUpdates())
        {
            checkForUpdates();
        }
        //Load images
        std::thread syncWorker{ [this]()
        {
            m_imagesSynced.invoke({ m_spotlightManager.sync() });
        } };
        syncWorker.detach();
        m_started = true;
        return info;
    }

    void MainWindowController::shutdown(const WindowGeometry& geometry)
    {
        Configuration& config{ m_dataFileManager.get<Configuration>("config") };
        config.setWindowGeometry(geometry);
        config.save();
    }

    void MainWindowController::checkForUpdates()
    {
        if(!m_updater)
        {
            return;
        }
        std::thread worker{ [this]()
        {
            Version latest{ m_updater->fetchCurrentVersion(VersionType::Stable) };
            if (!latest.empty())
            {
                if (latest > m_appInfo.getVersion())
                {
                    m_notificationSent.invoke({ _("New update available"), NotificationSeverity::Success, "update" });
                }
            }
        } };
        worker.detach();
    }

    void MainWindowController::windowsUpdate()
    {
        if(!m_updater)
        {
            return;
        }
        m_notificationSent.invoke({ _("The update is downloading in the background and will start once it finishes"), NotificationSeverity::Informational });
        std::thread worker{ [this]()
        {
            if(!m_updater->windowsUpdate(VersionType::Stable))
            {
                m_notificationSent.invoke({ _("Unable to download and install update"), NotificationSeverity::Error });
            }
        } };
        worker.detach();
    }

    void MainWindowController::clearAndSync()
    {
        //Load images
        std::thread syncWorker{ [this]()
        {
            m_imagesSynced.invoke({ m_spotlightManager.clearAndSync() });
        } };
        syncWorker.detach();
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
