/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include <mrpt/math/CMatrixDynamic.h>
#include <mrpt/math/CMatrixFixed.h>
#include <mrpt/random.h>

#include "common.h"

using namespace mrpt;
using namespace mrpt::math;
using namespace mrpt::random;
using namespace std;

// ------------------------------------------------------
// register_tests_matrices:
//   The code for matrices is split in several files
//   to avoid excesive RAM usage by the compiler, which
//   made fail the build in "small" MIPS machines.
// ------------------------------------------------------
void register_tests_matrices1();
void register_tests_matrices2();

void register_tests_matrices()
{
	getRandomGenerator().randomize(1234);

	register_tests_matrices1();
	register_tests_matrices2();
}

template <typename T>
double matrix_test_unit_dyn(int a1, int a2)
{
	CMatrixDynamic<T> C(a1, a1);

	const long N = 1000000;
	CTicTac tictac;
	for (long i = 0; i < N; i++)
	{
		C.resize(a1, a1);
		C.setIdentity();
	}
	return tictac.Tac() / N;
}

template <typename T, size_t DIM>
double matrix_test_unit_fix(int a1, int a2)
{
	CMatrixFixed<T, DIM, DIM> C;

	const long N = 1000000;
	CTicTac tictac;
	for (long i = 0; i < N; i++)
	{
		C.resize(DIM, DIM);
		C.setIdentity();
	}
	return tictac.Tac() / N;
}

template <typename T, size_t DIM1, size_t DIM2, size_t DIM3>
double matrix_test_mult_dyn(int a1, int a2)
{
	CMatrixDynamic<T> A(DIM1, DIM2);
	CMatrixDynamic<T> B(DIM2, DIM3);
	CMatrixDynamic<T> C(DIM1, DIM3);

	getRandomGenerator().drawGaussian1DMatrix(A, T(0), T(1));
	getRandomGenerator().drawGaussian1DMatrix(B, T(0), T(1));

	const long N = 10000;
	CTicTac tictac;
	for (long i = 0; i < N; i++)
	{
		C.matProductOf_AB(A, B);
	}
	return tictac.Tac() / N;
}

template <typename T, size_t DIM1, size_t DIM2, size_t DIM3>
double matrix_test_mult_fix(int a1, int a2)
{
	CMatrixFixed<T, DIM1, DIM2> A;
	CMatrixFixed<T, DIM2, DIM3> B;
	CMatrixFixed<T, DIM1, DIM3> C;

	getRandomGenerator().drawGaussian1DMatrix(A, T(0), T(1));
	getRandomGenerator().drawGaussian1DMatrix(B, T(0), T(1));

	const long N = 10000;
	CTicTac tictac;
	for (long i = 0; i < N; i++)
	{
		C.matProductOf_AB(A, B);
	}
	return tictac.Tac() / N;
}

template <typename T, size_t DIM1>
double matrix_test_inv_dyn(int a1, int a2)
{
	CMatrixDynamic<T> A(DIM1, DIM1);
	CMatrixDynamic<T> A2(DIM1, DIM1);
	getRandomGenerator().drawGaussian1DMatrix(A, T(0), T(1));

	const long N = 1000;
	CTicTac tictac;
	for (long i = 0; i < N; i++)
		A2 = A.inverse_LLt();
	return tictac.Tac() / N;
}

template <typename T, size_t DIM1>
double matrix_test_inv_fix(int a1, int a2)
{
	CMatrixFixed<T, DIM1, DIM1> A, A2;
	getRandomGenerator().drawGaussian1DMatrix(A, T(0), T(1));

	const long N = 1000;
	CTicTac tictac;
	for (long i = 0; i < N; i++)
		A2 = A.inverse_LLt();
	return tictac.Tac() / N;
}

template <typename T, size_t DIM1>
double matrix_test_det_dyn(int a1, int a2)
{
	CMatrixDynamic<T> A(DIM1, DIM1);
	getRandomGenerator().drawGaussian1DMatrix(A, T(0), T(1));

	const long N = 10000;
	CTicTac tictac;
	for (long i = 0; i < N; i++)
		A.det();
	return tictac.Tac() / N;
}

template <typename T, size_t DIM1>
double matrix_test_det_fix(int a1, int a2)
{
	CMatrixFixed<T, DIM1, DIM1> A;
	getRandomGenerator().drawGaussian1DMatrix(A, T(0), T(1));

	const long N = 10000;
	CTicTac tictac;
	for (long i = 0; i < N; i++)
		A.det();
	return tictac.Tac() / N;
}

template <typename MAT, size_t DIM>
double matrix_test_vector_resize(int VECTOR_LEN, int a2)
{
	using vec_t = std::vector<MAT>;

	const long N = 10000;
	CTicTac tictac;
	for (long i = 0; i < N; i++)
	{
		vec_t v;
		v.resize(VECTOR_LEN);
		for (auto& m : v)
			m.resize(DIM, DIM);
	}
	return tictac.Tac() / N;
}

// ------------------------------------------------------
// register_tests_matrices: Part 1
// ------------------------------------------------------
void register_tests_matrices1()
{
	lstTests.emplace_back(
		"matrix: unit, dyn[float], 3x3", matrix_test_unit_dyn<float>, 3);
	lstTests.emplace_back(
		"matrix: unit, dyn[double], 3x3", matrix_test_unit_dyn<double>, 3);
	lstTests.emplace_back(
		"matrix: unit, dyn[float], 6x6", matrix_test_unit_dyn<float>, 6);
	lstTests.emplace_back(
		"matrix: unit, dyn[double], 6x6", matrix_test_unit_dyn<double>, 6);

	lstTests.emplace_back(
		"matrix: unit, fix[float,3,3]", matrix_test_unit_fix<float, 3>);
	lstTests.emplace_back(
		"matrix: unit, fix[double,3,3]", matrix_test_unit_fix<double, 3>);
	lstTests.emplace_back(
		"matrix: unit, fix[float,6,6]", matrix_test_unit_fix<float, 6>);
	lstTests.emplace_back(
		"matrix: unit, fix[double,6,6]", matrix_test_unit_fix<double, 6>);

	lstTests.emplace_back(
		"matrix: multiply, dyn[float ], 3x3 * 3x3",
		matrix_test_mult_dyn<float, 3, 3, 3>);
	lstTests.emplace_back(
		"matrix: multiply, fix[float ], 3x3 * 3x3",
		matrix_test_mult_fix<float, 3, 3, 3>);
	lstTests.emplace_back(
		"matrix: multiply, dyn[double], 3x3 * 3x3",
		matrix_test_mult_dyn<double, 3, 3, 3>);
	lstTests.emplace_back(
		"matrix: multiply, fix[double], 3x3 * 3x3",
		matrix_test_mult_fix<double, 3, 3, 3>);
	lstTests.emplace_back(
		"matrix: multiply, dyn[float ], 3x6 * 6x3",
		matrix_test_mult_dyn<float, 3, 6, 3>);
	lstTests.emplace_back(
		"matrix: multiply, dyn[double], 3x6 * 6x3",
		matrix_test_mult_dyn<double, 3, 6, 3>);
	lstTests.emplace_back(
		"matrix: multiply, dyn[float ], 10x40 * 40x10",
		matrix_test_mult_dyn<float, 10, 40, 10>);
	lstTests.emplace_back(
		"matrix: multiply, dyn[double], 10x40 * 40x10",
		matrix_test_mult_dyn<double, 10, 40, 10>);

	// Note: All "float" tests below were removed since they produced weird
	// compile errors in MSVC :-(

	lstTests.emplace_back(
		"matrix: inverse_LLt(), dyn[double] 3x3",
		matrix_test_inv_dyn<double, 3>);
	lstTests.emplace_back(
		"matrix: inverse_LLt(), fix[double] 3x3",
		matrix_test_inv_fix<double, 3>);
	lstTests.emplace_back(
		"matrix: inverse_LLt(), dyn[double] 6x6",
		matrix_test_inv_dyn<double, 6>);
	lstTests.emplace_back(
		"matrix: inverse_LLt(), fix[double] 6x6",
		matrix_test_inv_fix<double, 6>);
	lstTests.emplace_back(
		"matrix: inverse_LLt(), dyn[double] 20x20",
		matrix_test_inv_dyn<double, 20>);
	lstTests.emplace_back(
		"matrix: inverse_LLt(), dyn[double] 40x40",
		matrix_test_inv_dyn<double, 40>);

	lstTests.emplace_back(
		"matrix: det, dyn[double] 2x2", matrix_test_det_dyn<double, 2>);
	lstTests.emplace_back(
		"matrix: det, fix[double] 2x2", matrix_test_det_fix<double, 2>);
	lstTests.emplace_back(
		"matrix: det, dyn[double] 3x3", matrix_test_det_dyn<double, 3>);
	lstTests.emplace_back(
		"matrix: det, fix[double] 3x3", matrix_test_det_fix<double, 3>);
	lstTests.emplace_back(
		"matrix: det, dyn[double] 6x6", matrix_test_det_dyn<double, 6>);
	lstTests.emplace_back(
		"matrix: det, fix[double] 6x6", matrix_test_det_fix<double, 6>);
	lstTests.emplace_back(
		"matrix: det, dyn[double] 20x20", matrix_test_det_dyn<double, 20>);
	lstTests.emplace_back(
		"matrix: det, dyn[double] 40x40", matrix_test_det_dyn<double, 40>);

	// clang-format off
	lstTests.emplace_back("matrix: vector of, resize(10) dyn[double] 4x4",    matrix_test_vector_resize<CMatrixDynamic<double>, 4>, 10);
	lstTests.emplace_back("matrix: vector of, resize(100) dyn[double] 4x4",   matrix_test_vector_resize<CMatrixDynamic<double>, 4>, 100);
	lstTests.emplace_back("matrix: vector of, resize(1000) dyn[double] 4x4",  matrix_test_vector_resize<CMatrixDynamic<double>, 4>, 1000);

	lstTests.emplace_back("matrix: vector of, resize(10) dyn[double] 5x5",    matrix_test_vector_resize<CMatrixDynamic<double>, 5>, 10);
	lstTests.emplace_back("matrix: vector of, resize(100) dyn[double] 5x5",   matrix_test_vector_resize<CMatrixDynamic<double>, 5>, 100);
	lstTests.emplace_back("matrix: vector of, resize(1000) dyn[double] 5x5",  matrix_test_vector_resize<CMatrixDynamic<double>, 5>, 1000);

	lstTests.emplace_back("matrix: vector of, resize(10) fix[double] 4x4",    matrix_test_vector_resize<CMatrixDouble44, 4>, 10);
	lstTests.emplace_back("matrix: vector of, resize(100) fix[double] 4x4",   matrix_test_vector_resize<CMatrixDouble44, 4>, 100);
	lstTests.emplace_back("matrix: vector of, resize(1000) fix[double] 4x4",  matrix_test_vector_resize<CMatrixDouble44, 4>, 1000);

	lstTests.emplace_back("matrix: vector of, resize(10) fix[double] 5x5",    matrix_test_vector_resize<CMatrixFixed<double,5,5>, 5>, 10);
	lstTests.emplace_back("matrix: vector of, resize(100) fix[double] 5x5",   matrix_test_vector_resize<CMatrixFixed<double,5,5>, 5>, 100);
	lstTests.emplace_back("matrix: vector of, resize(1000) fix[double] 5x5",  matrix_test_vector_resize<CMatrixFixed<double,5,5>, 5>, 1000);

	// clang-format on
}
