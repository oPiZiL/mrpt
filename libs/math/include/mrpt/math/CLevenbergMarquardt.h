/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */
#pragma once

#include <mrpt/containers/printf_vector.h>
#include <mrpt/math/CMatrixDynamic.h>
#include <mrpt/math/CVectorDynamic.h>
#include <mrpt/math/num_jacobian.h>
#include <mrpt/math/ops_containers.h>
#include <mrpt/system/COutputLogger.h>

#include <functional>

namespace mrpt::math
{
/** An implementation of the Levenberg-Marquardt algorithm for least-square
 * minimization.
 *
 * Refer to: \ref tutorial_math_levenberg_marquardt
 *
 * \tparam VECTORTYPE The type for input/output vectors
 * \tparam USERPARAM The type of the additional constant parameters input to the
 * user supplied evaluation functor. Default type is a vector of NUMTYPE.
 * \ingroup mrpt_math_grp
 */
template <typename VECTORTYPE = CVectorDouble, class USERPARAM = VECTORTYPE>
class CLevenbergMarquardtTempl : public mrpt::system::COutputLogger
{
   public:
	using NUMTYPE = typename VECTORTYPE::Scalar;
	using matrix_t = CMatrixDynamic<NUMTYPE>;
	using vector_t = VECTORTYPE;

	CLevenbergMarquardtTempl()
		: mrpt::system::COutputLogger("CLevenbergMarquardt")
	{
	}

	/** The type of the function passed to execute. The user must supply a
	 * function which evaluates the error of a given point in the solution
	 * space.
	 *  \param x The state point under examination.
	 *  \param y The same object passed to "execute" as the parameter
	 * "userParam".
	 *  \param out The vector of (non-squared) errors, of the average square
	 * root error, for the given "x". The functor code must set the size of this
	 * vector.
	 */
	using TFunctorEval = std::function<void(
		const VECTORTYPE& x, const USERPARAM& y, VECTORTYPE& out)>;

	/** The type of an optional functor passed to \a execute to replace the
	 * Euclidean addition "x_new = x_old + x_incr" by any other operation.
	 */
	using TFunctorIncrement = std::function<void(
		VECTORTYPE& x_new, const VECTORTYPE& x_old, const VECTORTYPE& x_incr,
		const USERPARAM& user_param)>;

	struct TResultInfo
	{
		NUMTYPE final_sqr_err = 0, initial_sqr_err = 0;
		size_t iterations_executed = 0;
		/** The last error vector returned by the user-provided functor. */
		VECTORTYPE last_err_vector;
		/** Each row is the optimized value at each iteration. */
		matrix_t path;

		/** This matrix can be used to obtain an estimate of the optimal
		 * parameters covariance matrix:
		 *  \f[ COV = H M H^\top \f]
		 *  With COV the covariance matrix of the optimal parameters, H this
		 * matrix, and M the covariance of the input (observations).
		 */
		matrix_t H;
	};

	/** Executes the LM-method, with derivatives estimated from
	 *  \a functor is a user-provided function which takes as input two
	 *vectors, in this order:
	 *		- x: The parameters to be optimized.
	 *		- userParam: The vector passed to the LM algorithm, unmodified.
	 *	  and must return the "error vector", or the error (not squared) in each
	 *measured dimension, so the sum of the square of that output is the
	 *overall square error.
	 *
	 * \a x_increment_adder Is an optional functor which may replace the
	 *Euclidean "x_new = x + x_increment" at the core of the incremental
	 *optimizer by
	 *     any other operation. It can be used for example, in on-manifold
	 *optimizations.
	 */
	void execute(
		VECTORTYPE& out_optimal_x, const VECTORTYPE& x0, TFunctorEval functor,
		const VECTORTYPE& increments, const USERPARAM& userParam,
		TResultInfo& out_info,
		mrpt::system::VerbosityLevel verbosity = mrpt::system::LVL_INFO,
		const size_t maxIter = 200, const NUMTYPE tau = 1e-3,
		const NUMTYPE e1 = 1e-8, const NUMTYPE e2 = 1e-8,
		bool returnPath = true, TFunctorIncrement x_increment_adder = nullptr)
	{
		using namespace mrpt;
		using namespace mrpt::math;
		using namespace std;

		MRPT_START

		this->setMinLoggingLevel(verbosity);

		VECTORTYPE& x = out_optimal_x;	// Var rename

		// Asserts:
		ASSERT_(increments.size() == x0.size());

		x = x0;	 // Start with the starting point
		VECTORTYPE f_x;	 // The vector error from the user function
		matrix_t AUX;
		matrix_t J;	 // The Jacobian of "f"
		VECTORTYPE g;  // The gradient

		// Compute the jacobian and the Hessian:
		mrpt::math::estimateJacobian(x, functor, increments, userParam, J);
		out_info.H.matProductOf_AtA(J);

		const size_t H_len = out_info.H.cols();

		// Compute the gradient:
		functor(x, userParam, f_x);
		// g <- J' * f_x
		g.matProductOf_Atb(J, f_x);

		// Start iterations:
		bool found = math::norm_inf(g) <= e1;
		if (found)
			logFmt(
				mrpt::system::LVL_INFO,
				"End condition: math::norm_inf(g)<=e1 :%f\n",
				math::norm_inf(g));

		NUMTYPE lambda = tau * out_info.H.maximumDiagonal();
		size_t iter = 0;
		NUMTYPE v = 2;

		VECTORTYPE h_lm;
		VECTORTYPE xnew, f_xnew;
		NUMTYPE F_x = pow(math::norm(f_x), 2);
		out_info.initial_sqr_err = F_x;

		const size_t N = x.size();

		if (returnPath)
		{
			out_info.path.setSize(maxIter, N + 1);
			for (size_t i = 0; i < N; i++)
				out_info.path(iter, i) = x[i];
		}
		else
			out_info.path = matrix_t();	 // Empty matrix

		while (!found && ++iter < maxIter)
		{
			// H_lm = -( H + \lambda I ) ^-1 * g
			matrix_t H = out_info.H;
			for (size_t k = 0; k < H_len; k++)
				H(k, k) += lambda;

			AUX = H.inverse_LLt();
			// AUX.matProductOf_Ab(g,h_lm);	h_lm <- AUX*g
			h_lm.matProductOf_Ab(AUX, g);
			h_lm *= NUMTYPE(-1.0);

			double h_lm_n2 = math::norm(h_lm);
			double x_n2 = math::norm(x);

			MRPT_LOG_DEBUG_STREAM(
				"Iter:" << iter
						<< " x=" << mrpt::containers::sprintf_vector(" %f", x));

			if (h_lm_n2 < e2 * (x_n2 + e2))
			{
				// Done:
				found = true;
				logFmt(
					mrpt::system::LVL_INFO, "End condition: %e < %e\n", h_lm_n2,
					e2 * (x_n2 + e2));
			}
			else
			{
				// Improvement: xnew = x + h_lm;
				if (!x_increment_adder)
					xnew = x + h_lm;  // Normal Euclidean space addition.
				else
					x_increment_adder(xnew, x, h_lm, userParam);

				functor(xnew, userParam, f_xnew);
				const double F_xnew = pow(math::norm(f_xnew), 2);

				// denom = h_lm^t * ( \lambda * h_lm - g )
				VECTORTYPE tmp(h_lm);
				tmp *= lambda;
				tmp -= g;
				const double denom = tmp.dot(h_lm);
				const double l = (F_x - F_xnew) / denom;

				if (l > 0)	// There is an improvement:
				{
					// Accept new point:
					x = xnew;
					f_x = f_xnew;
					F_x = F_xnew;

					math::estimateJacobian(
						x, functor, increments, userParam, J);
					out_info.H.matProductOf_AtA(J);
					g.matProductOf_Atb(J, f_x);

					found = math::norm_inf(g) <= e1;
					if (found)
						logFmt(
							mrpt::system::LVL_INFO,
							"End condition: math::norm_inf(g)<=e1 : %e\n",
							math::norm_inf(g));

					lambda *= max(0.33, 1 - pow(2 * l - 1, 3));
					v = 2;
				}
				else
				{
					// Nope...
					lambda *= v;
					v *= 2;
				}

				if (returnPath)
				{
					for (size_t i = 0; i < N; i++)
						out_info.path(iter, i) = x[i];
					out_info.path(iter, x.size()) = F_x;
				}
			}
		}  // end while

		// Output info:
		out_info.final_sqr_err = F_x;
		out_info.iterations_executed = iter;
		out_info.last_err_vector = f_x;
		if (returnPath) out_info.path.setSize(iter, N + 1);

		MRPT_END
	}

};	// End of class def.

/** The default name for the LM class is an instantiation for "double" */
using CLevenbergMarquardt = CLevenbergMarquardtTempl<mrpt::math::CVectorDouble>;

}  // namespace mrpt::math
