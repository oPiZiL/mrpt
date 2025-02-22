/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include <gtest/gtest.h>

// Reuse code from example:
//#define SOCKET_TEST_VERBOSE
#include "samples/comms_socket_example/SocketsTest_impl.cpp"

TEST(SocketTests, send_receive_object)
{
	SocketsTest();
	EXPECT_TRUE(sockets_test_passed_ok);
}
