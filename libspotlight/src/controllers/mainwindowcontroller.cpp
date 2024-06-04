#include "controllers/mainwindowcontroller.h"
#include <ctime>
#include <format>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <libnick/app/aura.h>
#include <libnick/helpers/codehelpers.h>
#include <libnick/helpers/stringhelpers.h>
#include <libnick/localization/gettext.h>
#include <windows.h>
#include "models/configuration.h"

using namespace Nickvision::App;
using namespace Nickvision::Events;
using namespace Nickvision::Notifications;
using namespace Nickvision::Spotlight::Shared::Models;
using namespace Nickvision::Update;

namespace Nickvision::Spotlight::Shared::Controllers
{
    MainWindowController::MainWindowController()
        : m_started{ false }
    {
#ifdef DEBUG
        Aura::getActive().init("org.nickvision.spotlight", "Nickvision Spotlight", "Spotlight", Logging::LogLevel::Debug);
#else
        Aura::getActive().init("org.nickvision.spotlight", "Nickvision Spotlight", "Spotlight", Logging::LogLevel::Info);
#endif
        AppInfo& appInfo{ Aura::getActive().getAppInfo() };
        appInfo.setVersion({ "2024.6.0-next" });
        appInfo.setShortName(_("Spotlight"));
        appInfo.setDescription(_("Find your favorite Windows spotlight images"));
        appInfo.setSourceRepo("https://github.com/NickvisionApps/Spotlight");
        appInfo.setIssueTracker("https://github.com/NickvisionApps/Spotlight/issues/new");
        appInfo.setSupportUrl("https://github.com/NickvisionApps/Spotlight/discussions");
        appInfo.getExtraLinks()[_("Matrix Chat")] = "https://matrix.to/#/#nickvision:matrix.org";
        appInfo.getDevelopers()["Nicholas Logozzo"] = "https://github.com/nlogozzo";
        appInfo.getDevelopers()[_("Contributors on GitHub")] = "https://github.com/NickvisionApps/Spotlight/graphs/contributors";
        appInfo.getDesigners()["Nicholas Logozzo"] = "https://github.com/nlogozzo";
        appInfo.getDesigners()[_("Fyodor Sobolev")] = "https://github.com/fsobolev";
        appInfo.getDesigners()["DaPigGuy"] = "https://github.com/DaPigGuy";
        appInfo.getArtists()[_("David Lapshin")] = "https://github.com/daudix";
        appInfo.setTranslatorCredits(_("translator-credits"));
    }

    AppInfo& MainWindowController::getAppInfo() const
    {
        return Aura::getActive().getAppInfo();
    }

    bool MainWindowController::isDevVersion() const
    {
        return Aura::getActive().getAppInfo().getVersion().getVersionType() == VersionType::Preview;
    }

    Theme MainWindowController::getTheme() const
    {
        return Aura::getActive().getConfig<Configuration>("config").getTheme();
    }

    WindowGeometry MainWindowController::getWindowGeometry() const
    {
        return Aura::getActive().getConfig<Configuration>("config").getWindowGeometry();
    }

    const std::vector<std::filesystem::path>& MainWindowController::getSpotlightImages() const
    {
        return m_spotlightManager->getImages();
    }

    Event<EventArgs>& MainWindowController::configurationSaved()
    {
        return Aura::getActive().getConfig<Configuration>("config").saved();
    }

    Event<NotificationSentEventArgs>& MainWindowController::notificationSent()
    {
        return m_notificationSent;
    }

    Event<ShellNotificationSentEventArgs>& MainWindowController::shellNotificationSent()
    {
        return m_shellNotificationSent;
    }

    std::string MainWindowController::getDebugInformation(const std::string& extraInformation) const
    {
        std::stringstream builder;
        builder << Aura::getActive().getAppInfo().getId();
        builder << ".winui" << std::endl;
        builder << Aura::getActive().getAppInfo().getVersion().toString() << std::endl << std::endl;
        if(Aura::getActive().isRunningViaFlatpak())
        {
            builder << "Running under Flatpak" << std::endl;
        }
        else if(Aura::getActive().isRunningViaSnap())
        {
            builder << "Running under Snap" << std::endl;
        }
        else
        {
            builder << "Running locally" << std::endl;
        }
        LCID lcid = GetThreadLocale();
        wchar_t name[LOCALE_NAME_MAX_LENGTH];
        if(LCIDToLocaleName(lcid, name, LOCALE_NAME_MAX_LENGTH, 0) > 0)
        {
            builder << StringHelpers::toString(name) << std::endl;
        }
        if (!extraInformation.empty())
        {
            builder << extraInformation << std::endl;
        }
        return builder.str();
    }

    std::shared_ptr<PreferencesViewController> MainWindowController::createPreferencesViewController() const
    {
        return std::make_shared<PreferencesViewController>();
    }

    void MainWindowController::startup()
    {
        if (m_started)
        {
            return;
        }
        try
        {
            m_updater = std::make_shared<Updater>(Aura::getActive().getAppInfo().getSourceRepo());
        }
        catch(...)
        {
            m_updater = nullptr;
        }
        if (Aura::getActive().getConfig<Configuration>("config").getAutomaticallyCheckForUpdates())
        {
            checkForUpdates();
        }
        m_spotlightManager = std::make_shared<SpotlightManager>();
        if(!*m_spotlightManager)
        {
            Aura::getActive().getLogger().log(Logging::LogLevel::Critical, "Unable to find spotlight images directory.");
            throw std::runtime_error("Unable to find spotlight images directory.");
        }
        m_spotlightManager->sync();
        m_started = true;
        Aura::getActive().getLogger().log(Logging::LogLevel::Debug, "MainWindow started.");
    }

    void MainWindowController::shutdown(const WindowGeometry& geometry)
    {
        Aura::getActive().getConfig<Configuration>("config").setWindowGeometry(geometry);
        Aura::getActive().getConfig<Configuration>("config").save();
        Aura::getActive().getLogger().log(Logging::LogLevel::Debug, "MainWindow shutdown.");
    }

    void MainWindowController::checkForUpdates()
    {
        if(!m_updater)
        {
            return;
        }
        Aura::getActive().getLogger().log(Logging::LogLevel::Debug, "Checking for updates...");
        std::thread worker{ [&]()
        {
            Version latest{ m_updater->fetchCurrentStableVersion() };
            if (!latest.empty())
            {
                if (latest > Aura::getActive().getAppInfo().getVersion())
                {
                    Aura::getActive().getLogger().log(Logging::LogLevel::Debug, "Update found: " + latest.toString());
                    m_notificationSent.invoke({ _("New update available"), NotificationSeverity::Success, "update" });
                }
                else
                {
                    Aura::getActive().getLogger().log(Logging::LogLevel::Debug, "No updates found.");
                }
            }
            else
            {
                Aura::getActive().getLogger().log(Logging::LogLevel::Warning, "Unable to fetch latest app version.");
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
        Aura::getActive().getLogger().log(Logging::LogLevel::Debug, "Fetching Windows app update...");
        std::thread worker{ [&]()
        {
            bool res{ m_updater->windowsUpdate(VersionType::Stable) };
            if (!res)
            {
                Aura::getActive().getLogger().log(Logging::LogLevel::Error, "Unable to fetch Windows app update.");
                m_notificationSent.invoke({ _("Unable to download and install update"), NotificationSeverity::Error, "error" });
            }
        } };
        worker.detach();
    }

    void MainWindowController::connectTaskbar(HWND hwnd)
    {
        if(m_taskbar.connect(hwnd))
        {
            Aura::getActive().getLogger().log(Logging::LogLevel::Debug, "Connected to Windows taskbar.");
        }
        else
        {
            Aura::getActive().getLogger().log(Logging::LogLevel::Error, "Unable to connect to Windows taskbar.");
        }
    }

    void MainWindowController::setImageAsDesktopBackground(int index)
    {
        if(m_spotlightManager->setAsDesktopBackground(static_cast<size_t>(index)))
        {
            m_notificationSent.invoke({ _("Image set as desktop background"), NotificationSeverity::Success });
        }
        else
        {
            m_notificationSent.invoke({ _("Unable to set image as desktop background"), NotificationSeverity::Error });
        }
    }
}
