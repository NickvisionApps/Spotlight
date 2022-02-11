#pragma once

#include <string>
#include <vector>

namespace NickvisionSpotlight::Models
{
	class SpotlightManager
	{
	public:
		SpotlightManager();
		const std::vector<std::string>& GetSpotlightImagePaths();
		void SyncSpotlightImages();
		void ExportImage(const std::string& selectedPath, const std::string& savePath) const;
		void ExportAllImages(const std::string& saveDir) const;
		void SetImageAsBackground(const std::string& selectedPath);

	private:
		std::string m_spotlightDir;
		std::string m_dataDir;
		std::vector<std::string> m_spotlightImagePaths;
	};
}

