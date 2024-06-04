#ifndef SPOTLIGHTMANAGER_H
#define SPOTLIGHTMANAGER_H

#include <filesystem>
#include <vector>
#include "spotlightsupport.h"

namespace Nickvision::Spotlight::Shared::Models
{
    /**
     * @brief A model that manages spotlight images. 
     */
    class SpotlightManager
    {
    public:
        /**
         * @brief Construct a SpotlightManager. 
         */
        SpotlightManager();
        /**
         * @brief Gets the list of paths to synced spotlight images.
         * @return The list of paths to synced spotlight images
         */
        const std::vector<std::filesystem::path>& getImages() const;
        /**
         * @brief Gets the level of support for spotlight images.
         * @return The level of support for spotlight images
         */
        SpotlightSupport getSupportLevel() const;
        /**
         * @brief Syncs spotlight images.
         * @return The list of paths to synced spotlight images
         */
        const std::vector<std::filesystem::path>& sync();
        /**
         * @brief Exports a spotlight image.
         * @param index The index of the image to export
         * @param path The path to export the image to
         * @return True if successful, else false
         */
        bool exportImage(size_t index, const std::filesystem::path& path) const;
        /**
         * @brief Exports a spotlight images.
         * @param image The path of the image to export
         * @param path The path of the directory to export the images to
         * @return True if successful, else false
         */
        bool exportImage(const std::filesystem::path& image, const std::filesystem::path& path) const;
        /**
         * @brief Exports all spotlight images.
         * @param path The path of the directory to export the images to
         * @return True if successful, else false
         */
        bool exportAllImages(const std::filesystem::path& path) const;
        /**
         * @brief Sets a spotlight image as the desktop background.
         * @param index The index of the image to set as the desktop background
         * @return True if successful, else false
         */
        bool setAsDesktopBackground(size_t index) const;
        /**
         * @brief Sets a spotlight image as the desktop background.
         * @param image The path of the image to set as the desktop background
         * @return True if successful, else false
         */
        bool setAsDesktopBackground(const std::filesystem::path& image) const;

    private:
        std::filesystem::path m_spotlightLockScreenDir;
        std::filesystem::path m_spotlightDesktopDir;
        std::filesystem::path m_dataDir;
        std::vector<std::filesystem::path> m_images;
    };
}

#endif //SPOTLIGHTMANAGER_H