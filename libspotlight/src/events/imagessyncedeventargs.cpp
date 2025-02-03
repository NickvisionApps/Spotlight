#include "events/imagessyncedeventargs.h"

using namespace Nickvision::Spotlight::Shared::Models;

namespace Nickvision::Spotlight::Shared::Events
{
    ImagesSyncedEventArgs::ImagesSyncedEventArgs(const std::vector<std::filesystem::path>& images, ViewMode viewMode)
        : m_images{ images },
        m_viewMode{ viewMode }
    {
    }

    const std::vector<std::filesystem::path>& ImagesSyncedEventArgs::getImages() const
    {
        return m_images;
    }

    ViewMode ImagesSyncedEventArgs::getViewMode() const
    {
        return m_viewMode;
    }
}