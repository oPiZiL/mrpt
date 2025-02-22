/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include <mrpt/comms/CInterfaceFTDI.h>
#include <mrpt/system/datetime.h>
#include <mrpt/system/os.h>

#include <cstdio>
#include <iostream>
#include <thread>

using namespace mrpt;
using namespace mrpt::comms;
using namespace std;

// ------------------------------------------------------
//				Test_EnumerateDevices
// ------------------------------------------------------
void Test_EnumerateDevices()
{
	CInterfaceFTDI usbDevice;

	unsigned long nConectedDevices;

	TFTDIDeviceList lstDevs;

	while (!mrpt::system::os::kbhit())
	{
		// Create list of devices:
		usbDevice.ListAllDevices(lstDevs);

		nConectedDevices = (unsigned long)lstDevs.size();

		cout << "There are " << nConectedDevices << " USB devices - "
			 << mrpt::system::dateTimeToString(mrpt::system::getCurrentTime())
			 << endl;

		for (size_t i = 0; i < nConectedDevices; i++)
			cout << lstDevs[i] << endl;

		printf("\nPRESS ANY KEY TO END THE PROGRAM...\n\n");
		cout.flush();
		std::this_thread::sleep_for(500ms);
	};
}

int main()
{
	try
	{
		Test_EnumerateDevices();
		return 0;
	}
	catch (const std::exception& e)
	{
		std::cerr << "MRPT error: " << mrpt::exception_to_str(e) << std::endl;
		return -1;
	}
	catch (...)
	{
		printf("Another exception!!");
		return -1;
	}
}
