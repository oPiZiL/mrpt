/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

// Note: Matrices unit tests have been split in different files since
// building them with eigen3 eats a lot of RAM and may be a problem while
// compiling in small systems.

#include <gtest/gtest.h>
#include <mrpt/math/CMatrixDynamic.h>
#include <mrpt/math/CMatrixFixed.h>
#include <mrpt/math/CVectorDynamic.h>
#include <mrpt/math/ops_containers.h>
#include <mrpt/math/ops_matrices.h>
#include <mrpt/random.h>

#include <Eigen/Dense>

using namespace mrpt;
using namespace mrpt::math;
using namespace mrpt::random;
using namespace std;

TEST(Matrices, meanAndStdColumns)
{
	/* meanAndStd: Computes a row with the mean values of each column in the
	   matrix and
		the associated vector with the standard deviation of each column. */

	const double dat_A[] = {
		2.8955668335, 2.3041932983, 1.9002381085, 1.7993158652, 1.8456197228,
		2.9632296740, 1.9368565578, 2.1988923358, 2.0547605617, 2.5655678993,
		2.3041932983, 3.8406914364, 2.1811218706, 3.2312564555, 2.4736403918,
		3.4703311380, 1.4874417483, 3.1073538218, 2.1353324397, 2.9541115932,
		1.9002381085, 2.1811218706, 2.4942067597, 1.6851007198, 1.4585872052,
		2.3015952197, 1.0955231591, 2.2979627790, 1.3918738834, 2.1854562572,
		1.7993158652, 3.2312564555, 1.6851007198, 3.1226161015, 1.6779632687,
		2.7195826381, 1.2397348013, 2.3757864319, 1.6291224768, 2.4463194915,
		1.8456197228, 2.4736403918, 1.4585872052, 1.6779632687, 2.8123267839,
		2.5860688816, 1.4131630919, 2.1914803135, 1.5542420639, 2.7170092067,
		2.9632296740, 3.4703311380, 2.3015952197, 2.7195826381, 2.5860688816,
		4.1669180394, 2.1145239023, 3.3214801332, 2.6694845663, 3.0742063088,
		1.9368565578, 1.4874417483, 1.0955231591, 1.2397348013, 1.4131630919,
		2.1145239023, 1.8928811570, 1.7097998455, 1.7205860530, 1.8710847505,
		2.1988923358, 3.1073538218, 2.2979627790, 2.3757864319, 2.1914803135,
		3.3214801332, 1.7097998455, 3.4592638415, 2.1518695071, 2.8907499694,
		2.0547605617, 2.1353324397, 1.3918738834, 1.6291224768, 1.5542420639,
		2.6694845663, 1.7205860530, 2.1518695071, 2.1110960664, 1.6731209980,
		2.5655678993, 2.9541115932, 2.1854562572, 2.4463194915, 2.7170092067,
		3.0742063088, 1.8710847505, 2.8907499694, 1.6731209980, 3.9093678727};
	CMatrixFixed<double, 10, 10> A(dat_A);

	// Compute mean & std of each column:
	CVectorDouble result_mean, result_std;
	mrpt::math::meanAndStdColumns(A, result_mean, result_std);

	// Result from MATLAB:
	const double dat_good_M[] = {
		2.246424086, 2.718547419, 1.899166596, 2.192679825, 2.073010093,
		2.938742050, 1.648159507, 2.570463898, 1.909148862, 2.628699435};
	const CVectorDouble good_M(dat_good_M);
	const double dat_good_S[] = {
		0.428901371, 0.720352792, 0.468999497, 0.684910097, 0.546595053,
		0.604303301, 0.328759015, 0.582584159, 0.382009344, 0.644788760};
	const CVectorDouble good_S(dat_good_S);

	EXPECT_NEAR((result_mean - good_M).sum_abs(), 0, 1e-4);
	EXPECT_NEAR((result_std - good_S).sum_abs(), 0, 1e-4);
}

TEST(Matrices, meanAndStdAll)
{
	/* meanAndStd: Computes a row with the mean values of each column in the
	   matrix and
		the associated vector with the standard deviation of each column. */

	const double dat_A[] = {
		2.8955668335, 2.3041932983, 1.9002381085, 1.7993158652, 1.8456197228,
		2.9632296740, 1.9368565578, 2.1988923358, 2.0547605617, 2.5655678993,
		2.3041932983, 3.8406914364, 2.1811218706, 3.2312564555, 2.4736403918,
		3.4703311380, 1.4874417483, 3.1073538218, 2.1353324397, 2.9541115932,
		1.9002381085, 2.1811218706, 2.4942067597, 1.6851007198, 1.4585872052,
		2.3015952197, 1.0955231591, 2.2979627790, 1.3918738834, 2.1854562572,
		1.7993158652, 3.2312564555, 1.6851007198, 3.1226161015, 1.6779632687,
		2.7195826381, 1.2397348013, 2.3757864319, 1.6291224768, 2.4463194915,
		1.8456197228, 2.4736403918, 1.4585872052, 1.6779632687, 2.8123267839,
		2.5860688816, 1.4131630919, 2.1914803135, 1.5542420639, 2.7170092067,
		2.9632296740, 3.4703311380, 2.3015952197, 2.7195826381, 2.5860688816,
		4.1669180394, 2.1145239023, 3.3214801332, 2.6694845663, 3.0742063088,
		1.9368565578, 1.4874417483, 1.0955231591, 1.2397348013, 1.4131630919,
		2.1145239023, 1.8928811570, 1.7097998455, 1.7205860530, 1.8710847505,
		2.1988923358, 3.1073538218, 2.2979627790, 2.3757864319, 2.1914803135,
		3.3214801332, 1.7097998455, 3.4592638415, 2.1518695071, 2.8907499694,
		2.0547605617, 2.1353324397, 1.3918738834, 1.6291224768, 1.5542420639,
		2.6694845663, 1.7205860530, 2.1518695071, 2.1110960664, 1.6731209980,
		2.5655678993, 2.9541115932, 2.1854562572, 2.4463194915, 2.7170092067,
		3.0742063088, 1.8710847505, 2.8907499694, 1.6731209980, 3.9093678727};
	CMatrixDouble A(10, 10, dat_A);

	// Compute mean & std of each column:
	double result_mean, result_std;
	mrpt::math::meanAndStd(A, result_mean, result_std);

	// Result from MATLAB:
	const double good_M = 2.282504177034;
	const double good_S = 0.660890754096;

	EXPECT_NEAR(std::abs(result_mean - good_M), 0, 1e-4);
	EXPECT_NEAR(std::abs(result_std - good_S), 0, 1e-4);
}

TEST(Matrices, laplacian)
{
	// The laplacian matrix of W is L = D - W.  (D:diagonals with degrees of
	// nodes)
	const double W_vals[6 * 6] = {0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0,
								  0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1,
								  1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0};
	const CMatrixDouble W(6, 6, W_vals);

	CMatrixDouble L;
	mrpt::math::laplacian(W, L);

	const double real_laplacian_vals[6 * 6] = {
		2, -1, 0,  0, -1, 0,  -1, 3,  -1, 0,  -1, 0, 0, -1, 2, -1, 0, 0,
		0, 0,  -1, 3, -1, -1, -1, -1, 0,  -1, 3,  0, 0, 0,	0, -1, 0, 1};
	const CMatrixDouble GT_L(6, 6, real_laplacian_vals);

	EXPECT_NEAR((GT_L - L).sum_abs(), 0, 1e-4);
}

TEST(Matrices, loadFromTextFile)
{
	{
		const std::string s1 =
			"1 2 3\n"
			"4 5 6";
		std::stringstream s(s1);
		CMatrixDouble M;
		bool retval = false;
		try
		{
			M.loadFromTextFile(s);
			retval = true;
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
		EXPECT_TRUE(retval) << "string:\n" << s1 << endl;
		EXPECT_EQ(M.rows(), 2);
		EXPECT_EQ(M.cols(), 3);
	}
	{
		const std::string s1 =
			"1 \t 2\n"
			"  4 \t\t 1    ";
		std::stringstream s(s1);
		CMatrixDouble M;
		bool retval = false;
		try
		{
			M.loadFromTextFile(s);
			retval = true;
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
		EXPECT_TRUE(retval) << "string:\n" << s1 << endl;
		EXPECT_EQ(M.rows(), 2);
		EXPECT_EQ(M.cols(), 2);
	}
	{
		const std::string s1 = "1 2";
		std::stringstream s(s1);
		CMatrixDouble M;
		bool retval = false;
		try
		{
			M.loadFromTextFile(s);
			retval = true;
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
		EXPECT_TRUE(retval) << "string:\n" << s1 << endl;
		EXPECT_EQ(M.rows(), 1);
		EXPECT_EQ(M.cols(), 2);
	}
	{
		const std::string s1 =
			"1 2 3\n"
			"0 1 0\n"
			"4 5 6\n";
		std::stringstream s(s1);
		CMatrixDouble33 M;
		bool retval = false;
		try
		{
			M.loadFromTextFile(s);
			retval = true;
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
		EXPECT_TRUE(retval) << "string:\n" << s1 << endl;
		EXPECT_EQ(M.rows(), 3);
		EXPECT_EQ(M.cols(), 3);
	}
	{
		const std::string s1 =
			"1 2 3\n"
			"4 5\n";
		std::stringstream s(s1);
		CMatrixDouble M;
		bool retval = false;
		try
		{
			M.loadFromTextFile(s);
			retval = true;
		}
		catch (std::exception&)
		{
		}
		EXPECT_FALSE(retval) << "string:\n" << s1 << endl;
	}
	{
		const std::string s1 =
			"1 2 3\n"
			"4 5\n";
		std::stringstream s(s1);
		CMatrixDouble M;
		bool retval = false;
		try
		{
			M.loadFromTextFile(s);
			retval = true;
		}
		catch (std::exception&)
		{
		}
		EXPECT_FALSE(retval) << "string:\n" << s1 << endl;
	}
	{
		const std::string s1 = "  \n";
		std::stringstream s(s1);
		CMatrixDouble M;
		bool retval = false;
		try
		{
			M.loadFromTextFile(s);
			retval = true;
		}
		catch (std::exception&)
		{
		}
		EXPECT_FALSE(retval) << "string:\n" << s1 << endl;
	}
	{
		const std::string s1 =
			"1 2 3\n"
			"1 2 3\n"
			"1 2 3";
		std::stringstream s(s1);
		CMatrixDouble22 M;
		bool retval = false;
		try
		{
			M.loadFromTextFile(s);
			retval = true;
		}
		catch (std::exception&)
		{
		}
		EXPECT_FALSE(retval) << "string:\n" << s1 << endl;
	}
}
