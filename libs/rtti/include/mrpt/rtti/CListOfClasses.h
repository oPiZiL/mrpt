/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */
#pragma once

#include <mrpt/core/Stringifyable.h>
#include <mrpt/rtti/CObject.h>

#include <set>

namespace mrpt::rtti
{
/** A list (actually based on a std::set) of MRPT classes, capable of keeping
 * any class registered by the mechanism of CObject classes. Access to "data"
 * for the actual content, or use any of the helper methods in this class.
 * \ingroup mrpt_rtti_grp
 */
class CListOfClasses : public mrpt::Stringifyable
{
   public:
	using TSet = std::set<const mrpt::rtti::TRuntimeClassId*>;
	TSet data;

	/** Insert a class in the list. Example of usage:
	 *   \code
	 *     myList.insert(CLASS_ID(CObservationImage));
	 *   \endcode
	 */
	inline void insert(const mrpt::rtti::TRuntimeClassId* id)
	{
		data.insert(id);
	}

	/** Does the list contains this class? */
	inline bool contains(const mrpt::rtti::TRuntimeClassId* id) const
	{
		return data.find(id) != data.end();
	}

	/** Does the list contains a class derived from...? */
	bool containsDerivedFrom(const mrpt::rtti::TRuntimeClassId* id) const;

	/** Return a string representation of the list, for example: "CPose2D,
	 * CObservation, CPose3D".
	 */
	std::string asString() const override;

	/** Builds from a string representation of the list, for example: "CPose2D,
	 * CObservation, CPose3D".
	 * \exception std::exception On unregistered class name found.
	 */
	void fromString(const std::string& s);

};	// end of class

}  // namespace mrpt::rtti
