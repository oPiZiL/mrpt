/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */
#pragma once

#include <mrpt/config.h>
#include <mrpt/obs/CObservationImage.h>
#include <mrpt/obs/CObservationStereoImages.h>

#include <list>

namespace mrpt::hwdrivers
{
typedef enum
{
	FRAMERATE_1_875 = 32,
	FRAMERATE_3_75,
	FRAMERATE_7_5,
	FRAMERATE_15,
	FRAMERATE_30,
	FRAMERATE_60,
	FRAMERATE_120,
	FRAMERATE_240
} grabber_dc1394_framerate_t;

typedef enum
{
	COLOR_CODING_MONO8 = 352,
	COLOR_CODING_YUV411,
	COLOR_CODING_YUV422,
	COLOR_CODING_YUV444,
	COLOR_CODING_RGB8,
	COLOR_CODING_MONO16
} grabber_dc1394_color_coding_t;

/** Options used when creating an dc1394 capture object
 *   All but the frame size, framerate, and color_coding can be changed
 * dynamically by CImageGrabber_dc1394::changeCaptureOptions
 * \sa CImageGrabber_dc1394
 * \ingroup mrpt_hwdrivers_grp
 */
struct TCaptureOptions_dc1394
{
	TCaptureOptions_dc1394() = default;

	/** Capture resolution (Default: 640x480) */
	int frame_width{640}, frame_height{480};
	grabber_dc1394_framerate_t framerate{FRAMERATE_15};
	grabber_dc1394_color_coding_t color_coding{COLOR_CODING_YUV422};

	/** -1: Normal mode, i>=0: use MODE7_i, then frame_width/height and
	 * color_coding are ignored. */
	int mode7{-1};

	/** Shutter, -1=default:Do not change */
	int shutter{-1};
	/** Gain, -1=default:Do not change */
	int gain{-1};
	/** Gamma, -1=default:Do not change */
	int gamma{-1};
	/** Brightness, -1=default:Do not change */
	int brightness{-1};
	/** Exposure, -1=default:Do not change */
	int exposure{-1};
	/** Sharpness, -1=default:Do not change */
	int sharpness{-1};
	/** White balance, -1=default:Do not change */
	int white_balance{-1};
	/** Shutter mode, -1=default:Do not change */
	int shutter_mode{-1};
	/** Gain mode, -1=default:Do not change */
	int gain_mode{-1};
	/** Gamma mode, -1=default:Do not change */
	int gamma_mode{-1};
	/** Brightness mode, -1=default:Do not change */
	int brightness_mode{-1};
	/** Exposure mode, -1=default:Do not change */
	int exposure_mode{-1};
	/** Sharpness mode, -1=default:Do not change */
	int sharpness_mode{-1};
	/** White balance mode, -1=default:Do not change */
	int white_balance_mode{-1};
	/** For stereo cameras (eg PR Bumblebee) */
	bool deinterlace_stereo{false};
	int trigger_power{-1};
	int trigger_mode{-1};
	int trigger_source{-1};
	int trigger_polarity{-1};
	/** Size of the libdc1394 ring buffer */
	int ring_buffer_size{15};
};

/** A class for grabing images from a IEEE1394 (Firewire) camera using the
 * libdc1394-2 library.
 *   See the constructor for the options when opening the camera. Notice that
 * you may have
 *    to carefully set the resolution, framerate and color_mode. See the
 * verbose parameter of
 *    the constructor, which can display a list of supported modes in your
 * camera.
 *
 *  This class is able to manage any Firewire cameras, including Stereo or
 * multi-cameras in general,
 *    so this can be used to open the Bumblebee camera (not tested yet).
 *
 * A static method (CImageGrabber_dc1394::enumerateCameras) is provided to
 * enumerate all existing cameras and their properties. It can be used
 *  to find the GUID of the desired camera, then open it at the constructor.
 *
 * \note This class requires MRPT compiled with "libdc1394-2" (Only works under
 * Linux for now) and "opencv".
 * \note In Linux you may need to execute "chmod 666 /dev/video1394/ * " and
 * "chmod 666 /dev/raw1394" for allowing any user R/W access to firewire
 * cameras.
 * \note [New in MRPT 1.3.0] Length of ring buffer is now configurable via
 * TCaptureOptions_dc1394::ring_buffer_size
 * \sa The most generic camera grabber in MRPT: mrpt::hwdrivers::CCameraSensor
 * \ingroup mrpt_hwdrivers_grp
 */
class CImageGrabber_dc1394
{
   protected:
	/** Set to false if we could not initialize the camera.
	 */
	bool m_bInitialized{false};

	/** Internal use: */
	void /* dc1394_t * */* m_dc1394_lib_context{nullptr};
	void /* dc1394camera_t* */* m_dc1394camera{nullptr};
	int m_desired_mode;

	TCaptureOptions_dc1394 m_options;

   public:
	/** Constructor: open an ieee1394 camera.
	 * \param cameraGUID Set the camera GUID to open, or 0 to open the first
	 * found camera.
	 * \param cameraUnit (Ignored if cameraGUID=0). The number of camera to open
	 * within the device with the given GUID: In a stereo camera this may be 0
	 * or 1. Normally this is 0.
	 * \param options Capture options, defined in
	 * mrpt::hwdrivers::TCaptureOptions_dc1394.
	 * \param verbose  Displays a lot of information about the camera to be open
	 * and its valid video modes.
	 */
	CImageGrabber_dc1394(
		uint64_t cameraGUID = 0, uint16_t cameraUnit = 0,
		const TCaptureOptions_dc1394& options = TCaptureOptions_dc1394(),
		bool verbose = false);

	/** Destructor
	 */
	virtual ~CImageGrabber_dc1394();

	/** Check whether the camera has been open successfully. */
	bool isOpen() const { return m_bInitialized; }
	/** Changes the capture properties (brightness, gain, shutter, etc)
	 * The frame size, framerate, and color_coding fields in options are
	 * ignored since they can be only set at construction time.
	 * \return false on error
	 */
	bool changeCaptureOptions(const TCaptureOptions_dc1394& options);

	/** Grab an image from the opened camera (for monocular cameras).
	 * \param out_observation The object to be filled with sensed data.
	 * \note This may be blocking when using software trigger and no frame is
	 * available yet. Ensure trigger before getObservation() or take into
	 * account that this call may block.
	 * \return false on any error, true if all go fine.
	 */
	bool getObservation(mrpt::obs::CObservationImage& out_observation);

	/** Grab an image from the opened camera (for stereo cameras).
	 * \param out_observation The object to be filled with sensed data.
	 *
	 * \return false on any error, true if all go fine.
	 */
	bool getObservation(mrpt::obs::CObservationStereoImages& out_observation);

	/** Changes the boolean level associated to Software Trigger (ON/OFF)
	 * Can be used to control camera triggering trough software
	 * \return false on error
	 */
	bool setSoftwareTriggerLevel(bool level);

	/** Used in enumerateCameras */
	struct TCameraInfo
	{
		uint64_t guid;
		int unit;
		uint32_t unit_spec_ID;
		uint32_t unit_sw_version;
		uint32_t unit_sub_sw_version;
		uint32_t command_registers_base;
		uint32_t unit_directory;
		uint32_t unit_dependent_directory;
		uint64_t advanced_features_csr;
		uint64_t PIO_control_csr;
		uint64_t SIO_control_csr;
		uint64_t strobe_control_csr;
		uint64_t format7_csr[16];
		int iidc_version;
		std::string vendor;
		std::string model;
		uint32_t vendor_id;
		uint32_t model_id;
		bool bmode_capable;
		bool one_shot_capable;
		bool multi_shot_capable;
		bool can_switch_on_off;
		bool has_vmode_error_status;
		bool has_feature_error_status;
		int max_mem_channel;
	};

	using TCameraInfoList = std::list<TCameraInfo>;

	/** Generates a list with the information on all the existing (Firewire)
	 * cameras in the system.
	 * \exception std::runtime_error On any error calling libdc1394.
	 */
	static void enumerateCameras(TCameraInfoList& out_list);

};	// End of class

}  // namespace mrpt::hwdrivers
