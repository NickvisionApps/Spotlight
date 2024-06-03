#include "models/spotlightmanager.h"
#include <fstream>
#include <boost/gil/extension/io/jpeg.hpp>
#include <libnick/filesystem/userdirectories.h>
#include <windows.h>

using namespace Nickvision::Filesystem;

namespace Nickvision::Spotlight::Shared::Models
{
    SpotlightManager::SpotlightManager()
        : m_spotlightDir{ UserDirectories::getCache() / "Packages/Microsoft.Windows.ContentDeliveryManager_cw5n1h2txyewy/LocalState/Assets" },
        m_dataDir{ UserDirectories::getApplicationConfig() / "Images" }
    {
        if(!std::filesystem::exists(m_dataDir))
        {
            std::filesystem::create_directories(m_dataDir);
        }
    }

    const std::vector<std::filesystem::path>& SpotlightManager::getImages() const
    {
        return m_images;
    }

    const std::vector<std::filesystem::path>& SpotlightManager::sync()
    {
        m_images.clear();
        for(const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(m_spotlightDir))
        {
            if(entry.file_size() / 1000 >= 200)
            {
                std::filesystem::path newPath{ m_dataDir / (entry.path().stem().string() + ".jpg") };
                std::filesystem::copy_file(entry.path(), newPath, std::filesystem::copy_options::overwrite_existing);
            }
        }
        for(const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(m_dataDir))
        {
            if(entry.path().extension() == ".jpg")
            {
                boost::gil::rgb8_image_t img;
                boost::gil::read_image(entry.path().string(), img, boost::gil::jpeg_tag());
                if(img.width() > img.height())
                {
                    m_images.push_back(entry.path());
                }
            }
        }
        return m_images;
    }

    bool SpotlightManager::isValid() const
    {
        return std::filesystem::exists(m_spotlightDir);
    }

    bool SpotlightManager::exportImage(size_t index, const std::filesystem::path& path) const
    {
        if(index >= m_images.size())
        {
            return false;
        }
        std::filesystem::copy_file(m_images[index], path, std::filesystem::copy_options::overwrite_existing);
        return true;
    }

    bool SpotlightManager::exportImage(const std::filesystem::path& image, const std::filesystem::path& path) const
    {
        std::vector<std::filesystem::path>::const_iterator it{ std::find(m_images.begin(), m_images.end(), image) };
        if(it == m_images.end())
        {
            return false;
        }
        return exportImage(it - m_images.begin(), path);
    }

    bool SpotlightManager::exportAllImages(const std::filesystem::path& path) const
    {
        if(!std::filesystem::exists(path))
        {
            std::filesystem::create_directories(path);
        }
        for(const std::filesystem::path& image : m_images)
        {
            std::filesystem::copy_file(image, path / image.filename(), std::filesystem::copy_options::overwrite_existing);
        }
        return true;
    }

    bool SpotlightManager::setAsDesktopBackground(size_t index) const
    {
        if(index >= m_images.size())
        {
            return false;
        }
        std::filesystem::path tempPath{ UserDirectories::getCache() / "NickvisionSpotlight.jpg" };
        std::filesystem::copy_file(m_images[index], tempPath, std::filesystem::copy_options::overwrite_existing);
        if(SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (PVOID)tempPath.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE) == 0)
        {
            return false;
        }
        return true;
    }

    bool SpotlightManager::setAsDesktopBackground(const std::filesystem::path& image) const
    {
        std::vector<std::filesystem::path>::const_iterator it{ std::find(m_images.begin(), m_images.end(), image) };
        if(it == m_images.end())
        {
            return false;
        }
        return setAsDesktopBackground(it - m_images.begin());
    }

    SpotlightManager::operator bool() const
    {
        return isValid();
    }
}