/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include <mrpt/core/exceptions.h>
#include <mrpt/hwdrivers/CJoystick.h>
#include <mrpt/system/CTicTac.h>
#include <mrpt/system/os.h>

#include <cstdio>
#include <iostream>
#include <thread>

using namespace std;
using namespace mrpt;
using namespace mrpt::system;
using namespace mrpt::hwdrivers;

// ------------------------------------------------------
//				TestJoystick
// ------------------------------------------------------
void TestJoystick()
{
	// Open first joystick:
	// ---------------------------
	float x, y, z;
	std::vector<bool> buttons;
	CTicTac tictac;
	CJoystick joy;

	const int nJoystick = 0;  // The first one

	printf("Press any key to stop program...\n");

	while (!mrpt::system::os::kbhit())
	{
		tictac.Tic();
		if (joy.getJoystickPosition(nJoystick, x, y, z, buttons))
		{
			double t = tictac.Tac();

			printf("Joystick readings: %.03f, %.03f, %.03f  (", x, y, z);
			for (unsigned b = 0; b < buttons.size(); b++)
				printf("B%u:%c ", b, buttons[b] ? 'X' : '-');
			printf(") [Query %uus]  \r", (unsigned)(t * 1e6));

			fflush(stdout);
		}
		else
		{
			printf(
				"Error reading from joystick, please connect one to the "
				"system...\r");
		}

		std::this_thread::sleep_for(20ms);
	}
}

int main()
{
	try
	{
		TestJoystick();

		return 0;
	}
	catch (const std::exception& e)
	{
		std::cerr << "MRPT error: " << mrpt::exception_to_str(e) << std::endl;
		return -1;
	}
}
