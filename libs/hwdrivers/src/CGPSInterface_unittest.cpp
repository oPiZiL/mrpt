/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include <gtest/gtest.h>
#include <mrpt/hwdrivers/CGPSInterface.h>
#include <mrpt/io/CMemoryStream.h>

using namespace mrpt;
using namespace mrpt::hwdrivers;
using namespace mrpt::obs;
using namespace std;

// Example cmds:
// https://www.sparkfun.com/datasheets/GPS/NMEA%20Reference%20Manual-Rev2.1-Dec07.pdf

TEST(CGPSInterface, parse_NMEA_GGA)
{
	// Test with a correct frame:
	{
		const char* test_cmd =
			"$GPGGA,101830.00,3649.76162994,N,00224.53709052,W,2,08,1.1,9.3,M,"
			"47.4,M,5.0,0120*58";
		mrpt::obs::CObservationGPS obsGPS;
		const bool parse_ret = CGPSInterface::parse_NMEA(test_cmd, obsGPS);
		EXPECT_TRUE(parse_ret) << "Failed parse of: " << test_cmd << endl;

		const gnss::Message_NMEA_GGA* msg =
			obsGPS.getMsgByClassPtr<gnss::Message_NMEA_GGA>();
		EXPECT_TRUE(msg != nullptr);
		if (!msg) return;
		EXPECT_NEAR(
			msg->fields.latitude_degrees, 36 + 49.76162994 / 60.0, 1e-10);
		EXPECT_NEAR(
			msg->fields.longitude_degrees, -(002 + 24.53709052 / 60.0), 1e-10);
		EXPECT_NEAR(msg->fields.altitude_meters, 9.3, 1e-10);
	}
	// Test with an empty frame:
	{
		const char* test_cmd = "$GPGGA,,,,,,0,,,,M,,M,,*6";
		mrpt::obs::CObservationGPS obsGPS;
		const bool parse_ret = CGPSInterface::parse_NMEA(test_cmd, obsGPS);
		EXPECT_FALSE(parse_ret);
	}
}

TEST(CGPSInterface, parse_NMEA_RMC)
{
	const char* test_cmd =
		"$GPRMC,161229.487,A,3723.2475,N,12158.3416,W,0.13,309.62,120598, ,*10";
	mrpt::obs::CObservationGPS obsGPS;
	const bool parse_ret = CGPSInterface::parse_NMEA(test_cmd, obsGPS);
	EXPECT_TRUE(parse_ret) << "Failed parse of: " << test_cmd << endl;

	const gnss::Message_NMEA_RMC* msg =
		obsGPS.getMsgByClassPtr<gnss::Message_NMEA_RMC>();

	EXPECT_TRUE(msg != nullptr);
	if (!msg) return;
	EXPECT_NEAR(msg->fields.latitude_degrees, 37 + 23.2475 / 60.0, 1e-10);
	EXPECT_NEAR(msg->fields.longitude_degrees, -(121 + 58.3416 / 60.0), 1e-10);
}

TEST(CGPSInterface, parse_NMEA_GLL)
{
	const char* test_cmd = "$GPGLL,3723.2475,N,12158.3416,W,161229.487,A,A*41";
	mrpt::obs::CObservationGPS obsGPS;
	const bool parse_ret = CGPSInterface::parse_NMEA(test_cmd, obsGPS);
	EXPECT_TRUE(parse_ret) << "Failed parse of: " << test_cmd << endl;

	const gnss::Message_NMEA_GLL* msg =
		obsGPS.getMsgByClassPtr<gnss::Message_NMEA_GLL>();

	EXPECT_TRUE(msg != nullptr);
	if (!msg) return;
	EXPECT_NEAR(msg->fields.latitude_degrees, 37 + 23.2475 / 60.0, 1e-10);
	EXPECT_NEAR(msg->fields.longitude_degrees, -(121 + 58.3416 / 60.0), 1e-10);
}

TEST(CGPSInterface, parse_NMEA_VTG)
{
	const char* test_cmd = "$GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48";
	mrpt::obs::CObservationGPS obsGPS;
	const bool parse_ret = CGPSInterface::parse_NMEA(test_cmd, obsGPS);
	EXPECT_TRUE(parse_ret) << "Failed parse of: " << test_cmd << endl;

	const gnss::Message_NMEA_VTG* msg =
		obsGPS.getMsgByClassPtr<gnss::Message_NMEA_VTG>();

	EXPECT_TRUE(msg != nullptr);
	if (!msg) return;
	EXPECT_NEAR(msg->fields.true_track, 54.7, 1e-6);
	EXPECT_NEAR(msg->fields.magnetic_track, 34.4, 1e-6);
	EXPECT_NEAR(msg->fields.ground_speed_knots, 5.5, 1e-6);
	EXPECT_NEAR(msg->fields.ground_speed_kmh, 10.2, 1e-6);
}

TEST(CGPSInterface, parse_NMEA_ZDA)
{
	const char* test_cmd = "$GPZDA,181813,14,10,2003,00,00*4F";
	mrpt::obs::CObservationGPS obsGPS;
	const bool parse_ret = CGPSInterface::parse_NMEA(test_cmd, obsGPS);
	EXPECT_TRUE(parse_ret) << "Failed parse of: " << test_cmd << endl;

	const gnss::Message_NMEA_ZDA* msg =
		obsGPS.getMsgByClassPtr<gnss::Message_NMEA_ZDA>();

	EXPECT_TRUE(msg != nullptr);
	if (!msg) return;
	EXPECT_TRUE(msg->fields.date_day == 14);
	EXPECT_TRUE(msg->fields.date_month == 10);
	EXPECT_TRUE(msg->fields.date_year == 2003);
	EXPECT_TRUE(msg->fields.UTCTime.hour == 18);
	EXPECT_TRUE(msg->fields.UTCTime.minute == 18);
	EXPECT_TRUE(msg->fields.UTCTime.sec == 13.0);
	// Replaced from EXPECT_EQ() to avoid a "bus error" in  a gtest template
	// under armhf.
}

TEST(CGPSInterface, parse_NMEA_ZDA_stream)
{
	auto buf = std::make_shared<mrpt::io::CMemoryStream>();
	{
		const std::string s("$GPZDA,181813,14,10,2003,00,00*4F\n");
		buf->Write(s.c_str(), s.size());
		buf->Seek(0);
	}

	CGPSInterface gps;
	gps.bindStream(buf);

	gps.initialize();
	gps.doProcess();

	mrpt::hwdrivers::CGenericSensor::TListObservations obss;
	gps.getObservations(obss);

	EXPECT_EQ(obss.size(), 1U);

	auto obsGPS = mrpt::ptr_cast<CObservationGPS>::from(obss.begin()->second);

	const gnss::Message_NMEA_ZDA* msg =
		obsGPS->getMsgByClassPtr<gnss::Message_NMEA_ZDA>();

	EXPECT_TRUE(msg != nullptr);
	if (!msg) return;
	EXPECT_TRUE(msg->fields.date_day == 14);
	EXPECT_TRUE(msg->fields.date_month == 10);
	EXPECT_TRUE(msg->fields.date_year == 2003);
	EXPECT_TRUE(msg->fields.UTCTime.hour == 18);
	EXPECT_TRUE(msg->fields.UTCTime.minute == 18);
	EXPECT_TRUE(msg->fields.UTCTime.sec == 13.0);
	// Replaced from EXPECT_EQ() to avoid a "bus error" in  a gtest template
	// under armhf.
}

TEST(CGPSInterface, parse_NOVATEL6_stream)
{
	auto buf = std::make_shared<mrpt::io::CMemoryStream>();
	{
		const uint8_t sample_novatel6_gps[] = {
			0xaa, 0x44, 0x12, 0x1c, 0x2a, 0x00, 0x00, 0xa0, 0x48, 0x00, 0x00,
			0x00, 0x5a, 0xb4, 0x59, 0x07, 0x10, 0x4a, 0xb7, 0x16, 0x00, 0x00,
			0x00, 0x00, 0xf6, 0xb1, 0x4a, 0x34, 0x00, 0x00, 0x00, 0x00, 0x38,
			0x00, 0x00, 0x00, 0x97, 0x2b, 0x45, 0xa9, 0xc8, 0x6a, 0x42, 0x40,
			0xfc, 0x54, 0x43, 0x6f, 0x11, 0x18, 0x03, 0xc0, 0x00, 0x00, 0x20,
			0x8f, 0xe8, 0x0e, 0x1c, 0x40, 0x66, 0x66, 0x48, 0x42, 0x3d, 0x00,
			0x00, 0x00, 0x1d, 0x9b, 0x96, 0x3c, 0x2c, 0xd5, 0x9c, 0x3c, 0xd1,
			0x39, 0xa8, 0x3c, 0x35, 0x35, 0x35, 0x00, 0x00, 0x00, 0x60, 0x41,
			0x00, 0x00, 0x00, 0x00, 0x0f, 0x0e, 0x0e, 0x0d, 0x00, 0x00, 0x00,
			0x33, 0x82, 0xba, 0x79, 0xe5, 0xaa, 0x44, 0x13, 0x58, 0xfc, 0x01,
			0x59, 0x07, 0x10, 0x4a, 0xb7, 0x16, 0x59, 0x07, 0x00, 0x00, 0x33,
			0x33, 0x33, 0x33, 0xdb, 0x42, 0x17, 0x41, 0xa7, 0xf0, 0xaf, 0xa5,
			0xc8, 0x6a, 0x42, 0x40, 0xa2, 0xad, 0xac, 0x28, 0x12, 0x18, 0x03,
			0xc0, 0x00, 0x00, 0x8a, 0x8b, 0x52, 0x8d, 0x4c, 0x40, 0x10, 0xe2,
			0xdb, 0x3c, 0x4b, 0xbd, 0x82, 0xbf, 0x52, 0x23, 0x1e, 0x50, 0x08,
			0xf1, 0x9b, 0xbf, 0xd4, 0xa6, 0xd1, 0x7c, 0xcd, 0x16, 0xc8, 0x3f,
			0x31, 0x27, 0xe1, 0x16, 0xa2, 0x6b, 0x10, 0x40, 0xc7, 0x1c, 0xc7,
			0x39, 0x6a, 0x9c, 0x00, 0x40, 0xa0, 0x3c, 0x9f, 0x79, 0xca, 0xdd,
			0x63, 0x40, 0x03, 0x00, 0x00, 0x00, 0x27, 0xbb, 0xff, 0xf8, 0xaa,
			0x44, 0x12, 0x1c, 0x2a, 0x00, 0x00, 0xa0, 0x48, 0x00, 0x00, 0x00,
			0x5a, 0xb4, 0x59, 0x07, 0x42, 0x4a, 0xb7, 0x16, 0x00, 0x00, 0x00,
			0x00, 0xf6, 0xb1, 0x4a, 0x34, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00,
			0x00, 0x00, 0xf0, 0x23, 0x3c, 0xa9, 0xc8, 0x6a, 0x42, 0x40, 0xdd,
			0x10, 0x6c, 0x71, 0x11, 0x18, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x03,
			0xa7, 0x18, 0x1c, 0x40, 0x66, 0x66, 0x48, 0x42, 0x3d, 0x00, 0x00,
			0x00, 0x32, 0x9b, 0x96, 0x3c, 0x82, 0xd4, 0x9c, 0x3c, 0x5d, 0x3a,
			0xa8, 0x3c, 0x35, 0x35, 0x35, 0x00, 0x00, 0x00, 0x60, 0x41, 0x00,
			0x00, 0x00, 0x00, 0x0f, 0x0e, 0x0e, 0x0d, 0x00, 0x00, 0x00, 0x33,
			0xcb, 0x95, 0xa0, 0x9b, 0xaa, 0x44, 0x13, 0x58, 0xfc, 0x01, 0x59,
			0x07, 0x42, 0x4a, 0xb7, 0x16, 0x59, 0x07, 0x00, 0x00, 0x67, 0x66,
			0x66, 0x66, 0xdb, 0x42, 0x17, 0x41, 0xe6, 0xae, 0xa1, 0xa5, 0xc8,
			0x6a, 0x42, 0x40, 0x26, 0x1e, 0x82, 0x2b, 0x12, 0x18, 0x03, 0xc0,
			0x00, 0x00, 0x62, 0xb6, 0x8b, 0x8e, 0x4c, 0x40, 0x10, 0x63, 0x42,
			0x19, 0x38, 0x19, 0x7a, 0xbf, 0x1e, 0xa9, 0x79, 0x02, 0x24, 0x6c,
			0x9d, 0xbf, 0x52, 0x13, 0x38, 0xa4, 0x35, 0x2c, 0xc8, 0x3f, 0xa9,
			0x3b, 0x21, 0x59, 0xe0, 0xa0, 0x10, 0x40, 0x51, 0xd1, 0x8c, 0x50,
			0x0b, 0xa0, 0x00, 0x40, 0x16, 0x40, 0x94, 0xbe, 0xc2, 0xdd, 0x63,
			0x40, 0x03, 0x00, 0x00, 0x00, 0x20, 0x4d, 0xe7, 0xa2, 0xaa, 0x44,
			0x12, 0x1c, 0x2a, 0x00, 0x00, 0xa0, 0x48, 0x00, 0x00, 0x00, 0x5a,
			0xb4, 0x59, 0x07, 0x74, 0x4a, 0xb7, 0x16, 0x00, 0x00, 0x00, 0x00,
			0xf6, 0xb1, 0x4a, 0x34, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00,
			0x00, 0xaa, 0x41, 0x32, 0xa9, 0xc8, 0x6a, 0x42, 0x40, 0xff, 0x59,
			0xa8, 0x73, 0x11, 0x18, 0x03, 0xc0, 0x00, 0x00, 0xa0, 0xd6, 0x6b,
			0x22, 0x1c, 0x40, 0x66, 0x66, 0x48, 0x42, 0x3d, 0x00, 0x00, 0x00,
			0x92, 0x9b, 0x96, 0x3c, 0x70, 0xd3, 0x9c, 0x3c, 0x06, 0x3b, 0xa8,
			0x3c, 0x35, 0x35, 0x35, 0x00};
		const unsigned int sample_novatel6_gps_len = 500;
		buf->Write(sample_novatel6_gps, sample_novatel6_gps_len);
		buf->Seek(0);
	}

	CGPSInterface gps;
	gps.bindStream(buf);

	gps.initialize();
	gps.doProcess();

	mrpt::hwdrivers::CGenericSensor::TListObservations obss;
	gps.getObservations(obss);

	EXPECT_EQ(obss.size(), 4U);
	if (obss.empty()) return;

	auto itObs = obss.begin();
	auto obsGPS1 = mrpt::ptr_cast<CObservationGPS>::from(itObs->second);
	++itObs;
	auto obsGPS2 = mrpt::ptr_cast<CObservationGPS>::from(itObs->second);

	EXPECT_TRUE(obsGPS1);
	EXPECT_TRUE(obsGPS2);

	const auto* msg1 =
		obsGPS1->getMsgByClassPtr<gnss::Message_NV_OEM6_BESTPOS>();
	EXPECT_TRUE(msg1 != nullptr);
	if (!msg1) return;
	EXPECT_TRUE(msg1->fields.num_sats_tracked == 15);

	const auto* msg2 =
		obsGPS2->getMsgByClassPtr<gnss::Message_NV_OEM6_INSPVAS>();
	EXPECT_TRUE(msg2 != nullptr);
	if (!msg2) return;
	EXPECT_NEAR(msg2->fields.roll, 4.10511, 1e-4);
}

TEST(CGPSInterface, parse_NMEA_stream)
{
	auto buf = std::make_shared<mrpt::io::CMemoryStream>();
	{
		// Data captured with a uBlox8 (University of Almeria, 2020)
		const uint8_t sample_nmea_gps[] = {
			0x31, 0x33, 0x2c, 0x32, 0x34, 0x2c, 0x33, 0x30, 0x2c, 0x32, 0x36,
			0x34, 0x2c, 0x33, 0x30, 0x2c, 0x32, 0x38, 0x2c, 0x34, 0x35, 0x2c,
			0x30, 0x36, 0x30, 0x2c, 0x31, 0x37, 0x2c, 0x33, 0x30, 0x2c, 0x31,
			0x38, 0x2c, 0x30, 0x35, 0x39, 0x2c, 0x33, 0x31, 0x2c, 0x33, 0x36,
			0x2c, 0x33, 0x34, 0x2c, 0x31, 0x33, 0x32, 0x2c, 0x2a, 0x37, 0x37,
			0x0a, 0x24, 0x47, 0x50, 0x47, 0x53, 0x56, 0x2c, 0x34, 0x2c, 0x34,
			0x2c, 0x31, 0x33, 0x2c, 0x34, 0x39, 0x2c, 0x34, 0x37, 0x2c, 0x31,
			0x36, 0x38, 0x2c, 0x2a, 0x34, 0x41, 0x0a, 0x24, 0x47, 0x4c, 0x47,
			0x53, 0x56, 0x2c, 0x32, 0x2c, 0x31, 0x2c, 0x30, 0x36, 0x2c, 0x36,
			0x36, 0x2c, 0x34, 0x32, 0x2c, 0x31, 0x34, 0x30, 0x2c, 0x2c, 0x36,
			0x37, 0x2c, 0x37, 0x39, 0x2c, 0x33, 0x34, 0x37, 0x2c, 0x31, 0x30,
			0x2c, 0x36, 0x38, 0x2c, 0x32, 0x35, 0x2c, 0x33, 0x32, 0x38, 0x2c,
			0x32, 0x36, 0x2c, 0x37, 0x36, 0x2c, 0x32, 0x35, 0x2c, 0x30, 0x33,
			0x34, 0x2c, 0x31, 0x31, 0x2a, 0x36, 0x38, 0x0a, 0x24, 0x47, 0x4c,
			0x47, 0x53, 0x56, 0x2c, 0x32, 0x2c, 0x32, 0x2c, 0x30, 0x36, 0x2c,
			0x37, 0x37, 0x2c, 0x37, 0x36, 0x2c, 0x33, 0x34, 0x33, 0x2c, 0x2c,
			0x37, 0x38, 0x2c, 0x33, 0x39, 0x2c, 0x32, 0x33, 0x32, 0x2c, 0x31,
			0x38, 0x2a, 0x36, 0x39, 0x0a, 0x24, 0x47, 0x4e, 0x47, 0x53, 0x54,
			0x2c, 0x31, 0x30, 0x30, 0x33, 0x35, 0x34, 0x2e, 0x34, 0x30, 0x2c,
			0x32, 0x35, 0x2c, 0x2c, 0x2c, 0x2c, 0x34, 0x35, 0x2c, 0x32, 0x30,
			0x2c, 0x36, 0x31, 0x2a, 0x36, 0x33, 0x0a, 0xb5, 0x62, 0x01, 0x01,
			0x14, 0x00, 0xb8, 0x9b, 0xc2, 0x16, 0xc1, 0x84, 0x70, 0x1e, 0x24,
			0x55, 0xb8, 0xfe, 0xb0, 0xee, 0xa9, 0x16, 0x8e, 0x19, 0x00, 0x00,
			0x47, 0xb2, 0xb5, 0x62, 0x01, 0x12, 0x24, 0x00, 0xb8, 0x9b, 0xc2,
			0x16, 0xee, 0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xea, 0xff,
			0xff, 0xff, 0x1d, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x99,
			0xa6, 0x14, 0x02, 0x08, 0x01, 0x00, 0x00, 0x80, 0xa8, 0x12, 0x01,
			0xf3, 0xeb, 0x24, 0x47, 0x4e, 0x52, 0x4d, 0x43, 0x2c, 0x31, 0x30,
			0x30, 0x33, 0x35, 0x34, 0x2e, 0x36, 0x30, 0x2c, 0x41, 0x2c, 0x33,
			0x36, 0x34, 0x39, 0x2e, 0x37, 0x38, 0x39, 0x32, 0x39, 0x2c, 0x4e,
			0x2c, 0x30, 0x30, 0x32, 0x32, 0x34, 0x2e, 0x34, 0x36, 0x38, 0x36,
			0x33, 0x2c, 0x57, 0x2c, 0x30, 0x2e, 0x33, 0x37, 0x38, 0x2c, 0x33,
			0x34, 0x39, 0x2e, 0x30, 0x38, 0x2c, 0x32, 0x37, 0x30, 0x32, 0x32,
			0x30, 0x2c, 0x2c, 0x2c, 0x41, 0x2a, 0x36, 0x36, 0x0a, 0x24, 0x47,
			0x4e, 0x47, 0x4e, 0x53, 0x2c, 0x31, 0x30, 0x30, 0x33, 0x35, 0x34,
			0x2e, 0x36, 0x30, 0x2c, 0x33, 0x36, 0x34, 0x39, 0x2e, 0x37, 0x38,
			0x39, 0x32, 0x39, 0x2c, 0x4e, 0x2c, 0x30, 0x30, 0x32, 0x32, 0x34,
			0x2e, 0x34, 0x36, 0x38, 0x36, 0x33, 0x2c, 0x57, 0x2c, 0x41, 0x4e,
			0x2c, 0x30, 0x34, 0x2c, 0x32, 0x2e, 0x33, 0x31, 0x2c, 0x35, 0x35,
			0x2e, 0x34, 0x2c, 0x34, 0x36, 0x2e, 0x32, 0x2c, 0x2c, 0x2a, 0x34,
			0x45, 0x0a, 0x24, 0x47, 0x4e, 0x47, 0x53, 0x41, 0x2c, 0x4d, 0x2c,
			0x33, 0x2c, 0x31, 0x35, 0x2c, 0x32, 0x34, 0x2c, 0x33, 0x30, 0x2c,
			0x31, 0x37, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c,
			0x34, 0x2e, 0x32, 0x31, 0x2c, 0x32, 0x2e, 0x33, 0x31, 0x2c, 0x33,
			0x2e, 0x35, 0x32, 0x2a, 0x31, 0x34, 0x0a, 0x24, 0x47, 0x4e, 0x47,
			0x53, 0x41, 0x2c, 0x4d, 0x2c, 0x33, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c,
			0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x2c, 0x34, 0x2e, 0x32,
			0x31, 0x2c, 0x32, 0x2e, 0x33, 0x31, 0x2c, 0x33, 0x2e, 0x35, 0x32,
			0x2a, 0x31, 0x33, 0x0a, 0x24, 0x47, 0x50, 0x47, 0x53, 0x56, 0x2c,
			0x34, 0x2c, 0x31, 0x2c, 0x31, 0x33, 0x2c, 0x30, 0x35, 0x2c, 0x33,
			0x36, 0x2c, 0x31, 0x38, 0x32, 0x2c, 0x32, 0x30, 0x2c, 0x31, 0x32,
			0x2c, 0x30, 0x33, 0x2c, 0x32, 0x30, 0x32, 0x2c, 0x2c, 0x31, 0x33,
			0x2c, 0x36, 0x39, 0x2c, 0x30, 0x34, 0x36, 0x2c, 0x31, 0x32, 0x2c,
			0x31, 0x35, 0x2c, 0x36, 0x31, 0x2c, 0x33, 0x32, 0x31, 0x2c, 0x32,
			0x39, 0x2a, 0x37, 0x33, 0x0a, 0x24, 0x47, 0x50, 0x47, 0x53, 0x56,
			0x2c, 0x34, 0x2c, 0x32, 0x2c, 0x31, 0x33, 0x2c, 0x31, 0x37, 0x2c,
			0x31, 0x31, 0x2c, 0x31, 0x30, 0x38, 0x2c, 0x32, 0x38, 0x2c, 0x31,
			0x39, 0x2c, 0x30, 0x34, 0x2c, 0x31, 0x33, 0x31, 0x2c, 0x31, 0x36,
			0x2c, 0x32, 0x30, 0x2c, 0x31, 0x33, 0x2c, 0x33, 0x31, 0x38, 0x2c,
			0x31, 0x37, 0x2c, 0x32, 0x31, 0x2c, 0x30, 0x33, 0x2c, 0x32, 0x38,
			0x38, 0x2c, 0x2a, 0x37, 0x45, 0x0a, 0x24, 0x47, 0x50, 0x47, 0x53,
			0x56, 0x2c, 0x34, 0x2c, 0x33, 0x2c, 0x31, 0x33, 0x2c, 0x32, 0x34,
			0x2c, 0x33, 0x30, 0x2c, 0x32, 0x36, 0x34, 0x2c, 0x33, 0x30, 0x2c,
			0x32, 0x38, 0x2c, 0x34, 0x35, 0x2c, 0x30, 0x36, 0x30, 0x2c, 0x31,
			0x37, 0x2c, 0x33, 0x30, 0x2c, 0x31, 0x38, 0x2c, 0x30, 0x35, 0x39,
			0x2c, 0x33, 0x31, 0x2c, 0x33, 0x36, 0x2c, 0x33, 0x34, 0x2c, 0x31,
			0x33, 0x32, 0x2c, 0x2a, 0x37, 0x37, 0x0a, 0x24, 0x47, 0x50, 0x47,
			0x53, 0x56, 0x2c, 0x34, 0x2c, 0x34, 0x2c, 0x31, 0x33, 0x2c, 0x34,
			0x39, 0x2c, 0x34, 0x37, 0x2c, 0x31, 0x36, 0x38, 0x2c, 0x2a, 0x34,
			0x41, 0x0a, 0x24, 0x47, 0x4c, 0x47, 0x53, 0x56, 0x2c, 0x32, 0x2c,
			0x31, 0x2c, 0x30, 0x36, 0x2c, 0x36, 0x36, 0x2c, 0x34, 0x32, 0x2c,
			0x31, 0x34, 0x30, 0x2c, 0x2c, 0x36, 0x37, 0x2c, 0x37, 0x39, 0x2c,
			0x33, 0x34, 0x37, 0x2c, 0x31, 0x31, 0x2c, 0x36, 0x38, 0x2c, 0x32,
			0x35, 0x2c, 0x33, 0x32, 0x38, 0x2c, 0x32, 0x36, 0x2c, 0x37, 0x36,
			0x2c, 0x32, 0x35, 0x2c, 0x30, 0x33, 0x34, 0x2c, 0x31, 0x31, 0x2a,
			0x36, 0x39, 0x0a, 0x24, 0x47, 0x4c, 0x47, 0x53, 0x56, 0x2c, 0x32,
			0x2c, 0x32, 0x2c, 0x30, 0x36, 0x2c, 0x37, 0x37, 0x2c, 0x37, 0x36,
			0x2c, 0x33, 0x34, 0x33, 0x2c, 0x2c, 0x37, 0x38, 0x2c, 0x33, 0x39,
			0x2c, 0x32, 0x33, 0x32, 0x2c, 0x31, 0x38, 0x2a, 0x36, 0x39, 0x0a,
			0x24, 0x47, 0x4e, 0x47, 0x53, 0x54, 0x2c, 0x31, 0x30, 0x30, 0x33,
			0x35, 0x34, 0x2e, 0x36, 0x30, 0x2c, 0x32, 0x35, 0x2c, 0x2c, 0x2c,
			0x2c, 0x33, 0x36, 0x2c, 0x31, 0x37, 0x2c, 0x35, 0x32, 0x2a, 0x36,
			0x31, 0x0a, 0xb5, 0x62, 0x01, 0x01, 0x14, 0x00, 0x80, 0x9c, 0xc2,
			0x16, 0xc6, 0x84, 0x70, 0x1e, 0x22, 0x55, 0xb8, 0xfe, 0xaf, 0xee,
			0xa9, 0x16, 0x90, 0x19, 0x00, 0x00, 0x14, 0x9d, 0xb5, 0x62, 0x01,
			0x12, 0x24, 0x00, 0x80, 0x9c, 0xc2, 0x16, 0xee, 0xff, 0xff, 0xff,
			0xf9, 0xff, 0xff, 0xff, 0xea, 0xff, 0xff, 0xff, 0x1d, 0x00, 0x00,
			0x00, 0x13, 0x00, 0x00, 0x00, 0x99, 0xa6, 0x14, 0x02, 0x14, 0x02,
			0x00, 0x00, 0x80, 0xa8, 0x12, 0x01, 0xc9, 0x95, 0x24, 0x47, 0x4e,
			0x52, 0x4d, 0x43, 0x2c, 0x31, 0x30, 0x30, 0x33, 0x35, 0x34, 0x2e,
			0x38, 0x30, 0x2c, 0x56, 0x2c, 0x33, 0x36, 0x34, 0x39, 0x2e};
		const unsigned int sample_nmea_gps_len = 1000;
		buf->Write(sample_nmea_gps, sample_nmea_gps_len);
		buf->Seek(0);
	}

	CGPSInterface gps;
	gps.bindStream(buf);

	gps.initialize();
	gps.doProcess();

	mrpt::hwdrivers::CGenericSensor::TListObservations obss;
	gps.getObservations(obss);

	EXPECT_EQ(obss.size(), 3U);
	if (obss.empty()) return;

	auto itObs = obss.begin();
	auto obsGPS1 = mrpt::ptr_cast<CObservationGPS>::from(itObs->second);
	++itObs;
	auto obsGPS2 = mrpt::ptr_cast<CObservationGPS>::from(itObs->second);
	++itObs;
	auto obsGPS3 = mrpt::ptr_cast<CObservationGPS>::from(itObs->second);

	EXPECT_TRUE(obsGPS1);
	EXPECT_TRUE(obsGPS2);
	EXPECT_TRUE(obsGPS3);

	const auto* msg1 = obsGPS1->getMsgByClassPtr<gnss::Message_NMEA_GSA>();
	EXPECT_TRUE(msg1 != nullptr);
	if (!msg1) return;
	EXPECT_EQ(msg1->fields.PRNs[0][0], '1');
	EXPECT_EQ(msg1->fields.PRNs[0][1], '5');
	EXPECT_NEAR(msg1->fields.HDOP, 2.31, 0.1);

	const auto* msg2 = obsGPS2->getMsgByClassPtr<gnss::Message_NMEA_GSA>();
	EXPECT_TRUE(msg2 != nullptr);

	const auto* msg3 = obsGPS3->getMsgByClassPtr<gnss::Message_NMEA_RMC>();
	EXPECT_TRUE(msg3 != nullptr);
	if (!msg3) return;
	EXPECT_NEAR(msg3->fields.longitude_degrees, -2.407810500, 0.0001);
	EXPECT_NEAR(msg3->fields.latitude_degrees, 36.829821500, 0.0001);
}
