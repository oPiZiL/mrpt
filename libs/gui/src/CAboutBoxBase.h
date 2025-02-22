/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */
#pragma once
#include <string>

class CAboutBoxBase
{
   public:
	CAboutBoxBase(
		const std::string& appName, const std::string& additionalInfo,
		const bool showStandardInfo);
	virtual ~CAboutBoxBase();

   protected:
	std::string MRPTBuildVersion() const;
	std::string tutorial() const;
	std::string license() const;
	std::string information(
		const std::string& guiLibName, const std::string& guiLibVersion) const;

	const std::string m_appName;
	const std::string m_additionalInfo;
	const bool m_showStandardInfo;
};
