#ifndef IMAGESSYNCEDEVENTARGS_H
#define IMAGESSYNCEDEVENTARGS_H

#include <filesystem>
#include <vector>
#include <libnick/events/eventargs.h>
#include "models/viewmode.h"

namespace Nickvision::Spotlight::Shared::Events
{
    /**
     * @brief Event arguments for when images are synced.
     */
    class ImagesSyncedEventArgs : public Nickvision::Events::EventArgs
    {
    public:
        /**
         * @brief Constructs an ImagesSyncedEventArgs.
         * @param images The images that were synced
         * @param viewMode The view mode to use
         */
        ImagesSyncedEventArgs(const std::vector<std::filesystem::path>& images, Models::ViewMode viewMode);
        /**
         * @brief Gets the images that were synced.
         * @return The images that were synced
         */
        const std::vector<std::filesystem::path>& getImages() const;
        /**
         * @brief Gets the view mode to use.
         * @return The view mode to use
         */
        Models::ViewMode getViewMode() const;

    private:
        std::vector<std::filesystem::path> m_images;
        Models::ViewMode m_viewMode;
    };
}

#endif //IMAGESYNCEDEVENTARGS_H