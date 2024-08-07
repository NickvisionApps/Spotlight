#include "models/spotlightmanager.h"
#include <fstream>
#include <boost/gil/extension/io/jpeg.hpp>
#include <libnick/filesystem/userdirectories.h>
#include <windows.h>

using namespace Nickvision::Filesystem;
using namespace Nickvision::Logging;

namespace Nickvision::Spotlight::Shared::Models
{
    static std::string GetLastErrorAsString()
    {
        DWORD errorMessageID{ GetLastError() };
        if(errorMessageID == 0) 
        {
            return std::string();
        }
        LPSTR messageBuffer{ nullptr };
        size_t size{ FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr) };
        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);
        return message;
    }

    SpotlightManager::SpotlightManager(const std::string& appName, Logger& logger)
        : m_spotlightLockScreenDir{ UserDirectories::get(UserDirectory::LocalData) / "Packages/Microsoft.Windows.ContentDeliveryManager_cw5n1h2txyewy/LocalState/Assets" },
        m_spotlightDesktopDir{ UserDirectories::get(UserDirectory::LocalData) / "Packages/MicrosoftWindows.Client.CBS_cw5n1h2txyewy/LocalCache/Microsoft/IrisService" },
        m_dataDir{ UserDirectories::get(ApplicationUserDirectory::Config, appName) / "Images" },
        m_logger{ logger }
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
        m_logger.log(LogLevel::Debug, "Loading spotlight images...");
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
            m_images.push_back(entry.path());
        }
        m_logger.log(LogLevel::Info, "Loaded " + std::to_string(m_images.size()) + " image(s).");
        return m_images;
    }

    bool SpotlightManager::exportImage(size_t index, const std::filesystem::path& path) const
    {
        if(index >= m_images.size())
        {
            return false;
        }
        std::filesystem::copy_file(m_images[index], path, std::filesystem::copy_options::overwrite_existing);
        m_logger.log(LogLevel::Info, "Exported " + m_images[index].stem().string() + " to " + path.string());
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
        m_logger.log(LogLevel::Info, "Exported all images to " + path.string());
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
        m_logger.log(LogLevel::Info, "Setting desktop background to " + m_images[index].stem().string() + "...");
        if(SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (void*)tempPath.wstring().c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE) == 0)
        {
            m_logger.log(LogLevel::Error, "Failed to set desktop background: " + GetLastErrorAsString());
            return false;
        }
        m_logger.log(LogLevel::Info, "Desktop background set.");
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

    void SpotlightManager::processEntry(const std::filesystem::directory_entry& entry, SpotlightImageType type)
    {
        if(type == SpotlightImageType::LockScreen && entry.file_size() / 1000 < 200)
        {
           return;
        }
        else if(type == SpotlightImageType::Desktop && entry.path().extension() != ".jpg")
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