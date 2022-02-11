#include "SpotlightManager.h"
#include <stdexcept>
#include <windows.h>

namespace NickvisionSpotlight::Models
{
	SpotlightManager::SpotlightManager() : m_spotlightDir(std::string(getenv("localappdata")) + "\\Packages\\Microsoft.Windows.ContentDeliveryManager_cw5n1h2txyewy\\LocalState\\Assets\\"), m_dataDir(std::string(getenv("appdata")) + "\\Nickvision\\NickvisionSpotlight\\Images\\")
	{
		if (!std::filesystem::exists(m_spotlightDir))
		{
			throw std::runtime_error("Unable to access Windows Spotlight images directory.");
		}
		if (!std::filesystem::exists(m_dataDir))
		{
			std::filesystem::create_directories(m_dataDir);
		}
	}

	const std::vector<std::filesystem::path>& SpotlightManager::GetSpotlightImages() const
	{
		return m_spotlightImages;
	}

	void SpotlightManager::SyncSpotlightImages()
	{
		m_spotlightImages.clear();
		for (const std::filesystem::path& path : std::filesystem::directory_iterator(m_spotlightDir))
		{
			if (std::filesystem::file_size(path) / 1000 >= 200)
			{
				std::filesystem::path newPath = std::filesystem::path(m_dataDir.string() + path.stem().string() + ".jpg");
				std::filesystem::copy(path, newPath, std::filesystem::copy_options::overwrite_existing);
				m_spotlightImages.push_back(newPath);
			}
		}
	}

	void SpotlightManager::ExportImage(size_t selectedPath, const std::filesystem::path& savePath) const
	{
		try
		{
			std::filesystem::copy(m_spotlightImages.at(selectedPath), savePath, std::filesystem::copy_options::overwrite_existing);
		}
		catch (...)
		{
			throw std::invalid_argument("Invalid path index");
		}
	}

	void SpotlightManager::ExportAllImages(const std::filesystem::path& saveDir) const
	{
		for (const std::filesystem::path& path : m_spotlightImages)
		{
			std::filesystem::path copyPath = std::filesystem::path(saveDir.string() + path.filename().string());
			std::filesystem::copy(path, copyPath, std::filesystem::copy_options::overwrite_existing);
		}
	}

	void SpotlightManager::SetImageAsBackground(size_t selectedPath) const
	{
		std::filesystem::path backgroundPath = std::filesystem::path(m_dataDir.string() + "background.jpg");
		try
		{
			std::filesystem::copy(m_spotlightImages.at(selectedPath), backgroundPath, std::filesystem::copy_options::overwrite_existing);
		}
		catch (...)
		{
			throw std::invalid_argument("Invalid path index");
		}
		std::string backgroundPathAsString = backgroundPath.string();
		SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void*)(std::wstring(backgroundPathAsString.begin(), backgroundPathAsString.end()).c_str()), SPIF_UPDATEINIFILE);
	}
}