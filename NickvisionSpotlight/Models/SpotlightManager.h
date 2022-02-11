#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace NickvisionSpotlight::Models
{
	class SpotlightManager
	{
	public:
		SpotlightManager();
		const std::vector<std::filesystem::path>& GetSpotlightImages() const;
		void SyncSpotlightImages();
		void ExportImage(size_t selectedPath, const std::filesystem::path& savePath) const;
		void ExportAllImages(const std::filesystem::path& saveDir) const;
		void SetImageAsBackground(size_t selectedPath) const;

	private:
		std::filesystem::path m_spotlightDir;
		std::filesystem::path m_dataDir;
		std::vector<std::filesystem::path> m_spotlightImages;
	};
}

