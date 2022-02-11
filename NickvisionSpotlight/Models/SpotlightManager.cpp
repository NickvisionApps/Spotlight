#include "SpotlightManager.h"
#include <stdexcept>
#include <filesystem>
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

	const std::vector<std::string>& SpotlightManager::GetSpotlightImagePaths()
	{
		return m_spotlightImagePaths;
	}

	void SpotlightManager::SyncSpotlightImages()
	{
		m_spotlightImagePaths.clear();
		for (const std::filesystem::path& path : std::filesystem::directory_iterator(m_spotlightDir))
		{
			if (std::filesystem::file_size(path) / 1000 >= 200)
			{
				std::string newPath = m_dataDir + path.stem().string() + ".jpg";
				std::filesystem::copy(path, newPath, std::filesystem::copy_options::overwrite_existing);
				m_spotlightImagePaths.push_back(newPath);
			}
		}
	}

	void SpotlightManager::ExportImage(const std::string& selectedPath, const std::string& savePath) const
	{
		std::filesystem::copy(selectedPath, savePath);
	}

	void SpotlightManager::ExportAllImages(const std::string& saveDir) const
	{
		for (const std::string& path : m_spotlightImagePaths)
		{
			std::string copyPath = saveDir + std::filesystem::path(path).filename().string();
			std::filesystem::copy(path, copyPath, std::filesystem::copy_options::overwrite_existing);
		}
	}

	void SpotlightManager::SetImageAsBackground(const std::string& selectedPath)
	{
		std::string backgroundPath = m_dataDir + "background.jpg";
		std::filesystem::copy(selectedPath, backgroundPath, std::filesystem::copy_options::overwrite_existing);
		SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)backgroundPath.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
	}
}