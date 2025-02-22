/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */
#pragma once

#include <mrpt/opengl/CSetOfObjects.h>
#include <mrpt/poses/CPose3D.h>
#include <mrpt/serialization/CSerializable.h>

#include <vector>

namespace mrpt
{
/** Kinematics classes namespace \ingroup mrpt_kinematics_grp
 */
namespace kinematics
{
/** An individual kinematic chain element (one link) which builds up a
 * CKinematicChain.
 * The parameterization of the SE(3) transformation from the starting point to
 * the end point
 * follows a Denavit-Hartenberg standard parameterization: [theta, d, a,
 * alpha].
 * \ingroup mrpt_kinematics_grp
 */
struct TKinematicLink
{
	/** Rotation from X_i to X_{i+1} (radians) */
	double theta{0};
	/** Distance along Z_i to the common normal between Z_i and Z_{i+1} */
	double d{0};
	/** Distance along the common normal (in the same direction than the new
	 * X_{i+1}) */
	double a{0};
	/** Rotation along X_{i+1} to transform Z_i into Z_{i+1} */
	double alpha{0};

	/** "false": Is revolute ("q_i" is "theta"), "true": is prismatic ("q_i" is
	 * "d") */
	bool is_prismatic{false};

	TKinematicLink(
		double _theta, double _d, double _a, double _alpha, bool _is_prismatic)
		: theta(_theta),
		  d(_d),
		  a(_a),
		  alpha(_alpha),
		  is_prismatic(_is_prismatic)
	{
	}
	TKinematicLink() = default;
};

mrpt::serialization::CArchive& operator>>(
	mrpt::serialization::CArchive& in, TKinematicLink& o);
mrpt::serialization::CArchive& operator<<(
	mrpt::serialization::CArchive& out, const TKinematicLink& o);

/** A open-loop kinematic chain model, suitable to robotic manipulators.
 *  Each link is parameterized with standard Denavit-Hartenberg standard
 * parameterization [theta, d, a, alpha].
 *
 *  The orientation of the first link can be modified with setOriginPose(),
 * which defaults to standard XYZ axes with +Z pointing upwards.
 *
 * \sa CPose3D
 * \ingroup mrpt_kinematics_grp
 */
class CKinematicChain : public mrpt::serialization::CSerializable
{
	DEFINE_SERIALIZABLE(CKinematicChain, mrpt::kinematics)

   private:
	/** Smart pointers to the last objects for each link, as returned in
	 * getAs3DObject(), for usage within update3DObject() */
	mutable std::vector<mrpt::opengl::CRenderizable::Ptr> m_last_gl_objects;

	/** The links of this robot arm */
	std::vector<TKinematicLink> m_links;
	/** The pose of the first link. */
	mrpt::poses::CPose3D m_origin;

   public:
	/** Return the number of links */
	size_t size() const { return m_links.size(); }
	/** Erases all links and leave the robot arm empty. */
	void clear();

	/** Appends a new link to the robotic arm, with the given Denavit-Hartenberg
	 * parameters (see TKinematicLink for further details) */
	void addLink(
		double theta, double d, double a, double alpha, bool is_prismatic);

	/** Removes one link from the kinematic chain (0<=idx<N) */
	void removeLink(const size_t idx);

	/** Get a ref to a given link (read-only) */
	const TKinematicLink& getLink(const size_t idx) const;

	/** Get a ref to a given link (read-write) */
	TKinematicLink& getLinkRef(const size_t idx);

	/** Can be used to define a first degree of freedom along a +Z axis which
	 * does not coincide with the global +Z axis. */
	void setOriginPose(const mrpt::poses::CPose3D& new_pose);

	/** Returns the current pose of the first link. */
	const mrpt::poses::CPose3D& getOriginPose() const;

	/** Get all the DOFs of the arm at once, returning them in a vector with all
	 * the "q_i" values, which
	 * can be interpreted as rotations (radians) or displacements (meters)
	 * depending on links being "revolute" or "prismatic".
	 * The vector is automatically resized to the correct size (the number of
	 * links).
	 * \tparam VECTOR Can be any Eigen vector, mrpt::math::CVectorDouble, or
	 * std::vector<double>
	 */
	template <class VECTOR>
	void getConfiguration(VECTOR& v) const
	{
		const size_t N = m_links.size();
		v.resize(N);
		for (size_t i = 0; i < N; i++)
		{
			if (m_links[i].is_prismatic) v[i] = m_links[i].d;
			else
				v[i] = m_links[i].theta;
		}
	}

	/** Set all the DOFs of the arm at once, from a vector with all the "q_i"
	 * values, which
	 * are interpreted as rotations (radians) or displacements (meters)
	 * depending on links being "revolute" or "prismatic".
	 * \exception std::exception If the size of the vector doesn't match the
	 * number of links.
	 * \tparam VECTOR Can be any Eigen vector, mrpt::math::CVectorDouble, or
	 * std::vector<double>
	 */
	template <class VECTOR>
	void setConfiguration(const VECTOR& v)
	{
		ASSERT_EQUAL_(v.size(), this->size());
		const size_t N = m_links.size();
		for (size_t i = 0; i < N; i++)
		{
			if (m_links[i].is_prismatic) m_links[i].d = v[i];
			else
				m_links[i].theta = v[i];
		}
	}

	/** Constructs a 3D representation of the kinematic chain, in its current
	 * state.
	 * You can call update3DObject() to update the kinematic state of these
	 * OpenGL objects in the future, since
	 * an internal list of smart pointers to the constructed objects is kept
	 * internally. This is more efficient
	 * than calling this method again to build a new representation.
	 * \param[out] out_all_poses Optional output vector, will contain the poses
	 * in the format of recomputeAllPoses()
	 * \sa update3DObject
	 */
	void getAs3DObject(
		mrpt::opengl::CSetOfObjects::Ptr& inout_gl_obj,
		std::vector<mrpt::poses::CPose3D>* out_all_poses = nullptr) const;

	/** Read getAs3DObject() for a description.
	 * \param[out] out_all_poses Optional output vector, will contain the poses
	 * in the format of recomputeAllPoses()
	 */
	void update3DObject(
		std::vector<mrpt::poses::CPose3D>* out_all_poses = nullptr) const;

	/** Go thru all the links of the chain and compute the global pose of each
	 * link. The "ground" link pose "pose0" defaults to the origin of
	 * coordinates,
	 * but anything else can be passed as the optional argument.
	 * The returned vector has N+1 elements (N=number of links), since [0]
	 * contains the base frame, [1] the pose after the first link, and so on.
	 */
	void recomputeAllPoses(
		std::vector<mrpt::poses::CPose3D>& poses,
		const mrpt::poses::CPose3D& pose0 = mrpt::poses::CPose3D()) const;

};	// End of class def.

}  // namespace kinematics

// Specialization must occur in the same namespace
// (This is to ease serialization)
namespace typemeta
{
MRPT_DECLARE_TTYPENAME_NAMESPACE(TKinematicLink, ::mrpt::kinematics)
}  // namespace typemeta

}  // namespace mrpt
