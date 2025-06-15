#include "models/spotlightmanager.h"
#include <fstream>
#include <boost/gil/extension/io/jpeg.hpp>
#include <libnick/filesystem/userdirectories.h>
#include <windows.h>

#define SPOTLIGHT_LOCKSCREEN_PACKAGE "Packages/Microsoft.Windows.ContentDeliveryManager_cw5n1h2txyewy/LocalState/Assets"
#define SPOTLIGHT_DESKTOP_PACKAGE "Packages/MicrosoftWindows.Client.CBS_cw5n1h2txyewy/LocalCache/Microsoft/IrisService"

using namespace Nickvision::Filesystem;

namespace Nickvision::Spotlight::Shared::Models
{
    SpotlightManager::SpotlightManager(const std::string& appName)
        : m_dataDir{ UserDirectories::get(ApplicationUserDirectory::Config, appName) / "Images" },
        m_spotlightLockScreenDir{ UserDirectories::get(UserDirectory::LocalData) / SPOTLIGHT_LOCKSCREEN_PACKAGE },
        m_spotlightDesktopDir{ UserDirectories::get(UserDirectory::LocalData) / SPOTLIGHT_DESKTOP_PACKAGE }
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

    SpotlightSupport SpotlightManager::getSupportLevel() const
    {
        if(std::filesystem::exists(m_spotlightLockScreenDir) && std::filesystem::exists(m_spotlightDesktopDir))
        {
            return SpotlightSupport::Full;
        }
        else if(std::filesystem::exists(m_spotlightLockScreenDir))
        {
            return SpotlightSupport::LockScreenOnly;
        }
        else if(std::filesystem::exists(m_spotlightDesktopDir))
        {
            return SpotlightSupport::DesktopOnly;
        }
        return SpotlightSupport::None;
    }

    const std::vector<std::filesystem::path>& SpotlightManager::sync()
    {
        m_images.clear();
        if(std::filesystem::exists(m_spotlightLockScreenDir))
        {
            for(const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(m_spotlightLockScreenDir))
            {
                processEntry(entry, SpotlightImageType::LockScreen);
            }
        }
        if(std::filesystem::exists(m_spotlightDesktopDir))
        {
            for(const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(m_spotlightDesktopDir))
            {
                processEntry(entry, SpotlightImageType::Desktop);
            }
        }
        for(const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(m_dataDir))
        {
            if(entry.path().extension() == ".jpg")
            {
                m_images.push_back(entry.path());
            }
        }
        return m_images;
    }

    const std::vector<std::filesystem::path>& SpotlightManager::clearAndSync()
    {
        std::filesystem::remove_all(m_dataDir);
        std::filesystem::create_directories(m_dataDir);
        return sync();
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
        std::filesystem::path tempPath{ UserDirectories::get(UserDirectory::LocalData) / "NickvisionSpotlight.jpg" };
        std::filesystem::copy_file(m_images[index], tempPath, std::filesystem::copy_options::overwrite_existing);
        return SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (void*)tempPath.wstring().c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE) != 0;
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

    void SpotlightManager::processEntry(const std::filesystem::directory_entry& entry, SpotlightImageType type)
    {
        if(!entry.is_regular_file())
        {
            return;
        }
        if(type == SpotlightImageType::LockScreen && entry.file_size() / 1000 < 200)
        {
           return;
        }
        if(type == SpotlightImageType::Desktop && entry.path().extension() != ".jpg")
        {
            return;
        }
        if(entry.path().filename().string().find("_") != std::string::npos)
        {
            return;
        }
        boost::gil::rgb8_image_t img;
        boost::gil::read_image(entry.path().string(), img, boost::gil::jpeg_tag());
        if(img.width() > img.height())
        {
            std::filesystem::path newPath{ m_dataDir / (entry.path().stem().string() + ".jpg") };
            std::filesystem::copy_file(entry.path(), newPath, std::filesystem::copy_options::skip_existing);
        }
    }
}
