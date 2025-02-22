/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */
#pragma once

namespace mrpt::graphslam::deciders
{
template <class GRAPH_T>
void CRangeScanEdgeRegistrationDecider<GRAPH_T>::loadParams(
	const std::string& source_fname)
{
	MRPT_START

	parent_t::loadParams(source_fname);
	range_ops_t::params.loadFromConfigFileName(source_fname, "ICP");

	MRPT_END
}

template <class GRAPH_T>
void CRangeScanEdgeRegistrationDecider<GRAPH_T>::printParams() const
{
	MRPT_START

	parent_t::printParams();
	range_ops_t::params.dumpToConsole();

	MRPT_END
}
}  // namespace mrpt::graphslam::deciders
