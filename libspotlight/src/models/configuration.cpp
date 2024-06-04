#include "models/configuration.h"

using namespace Nickvision::App;

namespace Nickvision::Spotlight::Shared::Models
{
    Configuration::Configuration(const std::string& key)
        : ConfigurationBase{ key }
    {
    }

    Theme Configuration::getTheme() const
    {
        return static_cast<Theme>(m_json.get("Theme", static_cast<int>(Theme::System)).asInt());
    }

    void Configuration::setTheme(Theme theme)
    {
        m_json["Theme"] = static_cast<int>(theme);
    }

    bool Configuration::getAutomaticallyCheckForUpdates() const
    {
#ifdef _WIN32
        bool def{ true };
#elif defined(__linux__)
        bool def{ false };
#endif
        return m_json.get("AutomaticallyCheckForUpdates", def).asBool();
    }

    WindowGeometry Configuration::getWindowGeometry() const
    {
        WindowGeometry geometry;
        const Json::Value json{ m_json["WindowGeometry"] };
        geometry.setWidth(json.get("Width", 900).asInt64());
        geometry.setHeight(json.get("Height", 700).asInt64());
        geometry.setIsMaximized(json.get("IsMaximized", false).asBool());
        return geometry;
    }

    void Configuration::setWindowGeometry(const WindowGeometry& geometry)
    {
        m_json["WindowGeometry"]["Width"] = static_cast<Json::Int64>(geometry.getWidth());
        m_json["WindowGeometry"]["Height"] = static_cast<Json::Int64>(geometry.getHeight());
        m_json["WindowGeometry"]["IsMaximized"] = geometry.isMaximized();
    }

    void Configuration::setAutomaticallyCheckForUpdates(bool check)
    {
        m_json["AutomaticallyCheckForUpdates"] = check;
    }

    ViewMode Configuration::getViewMode() const
    {
        return static_cast<ViewMode>(m_json.get("ViewMode", static_cast<int>(ViewMode::Grid)).asInt());
    }

    void Configuration::setViewMode(ViewMode mode)
    {
        m_json["ViewMode"] = static_cast<int>(mode);
    }
}