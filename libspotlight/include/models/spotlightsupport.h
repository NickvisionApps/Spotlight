#ifndef SPOTLIGHTSUPPORT_H
#define SPOTLIGHTSUPPORT_H

namespace Nickvision::Spotlight::Shared::Models
{
    /**
     * @brief Levels of support for spotlight images.
     */
    enum class SpotlightSupport
    {
        None = 0,
        LockScreenOnly,
        DesktopOnly,
        Full
    };
}

#endif //SPOTLIGHTSUPPORT_H