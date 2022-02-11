#pragma once

#include <string>

namespace NickvisionSpotlight::Models
{
	class Configuration
	{
	public:
		Configuration();
		bool PreferLightTheme() const;
		void SetPreferLightTheme(bool preferLightTheme);
		void Save() const;

	private:
		std::string m_configDir;
		bool m_preferLightTheme;
	};
}

