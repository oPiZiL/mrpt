#  See "DeclareMRPTLib.cmake"
list(APPEND io_EXTRA_SRCS		"${MRPT_LIBS_ROOT}/io/*.cpp" "${MRPT_LIBS_ROOT}/io/*.h")
list(APPEND io_EXTRA_SRCS_NAME 	"io" "io")

# Embedded version of zlib, if really needed:
# Only include zlib if not available as system library OR it's already in wxWidgets:
if(CMAKE_MRPT_HAS_ZLIB)
	if(CMAKE_MRPT_HAS_ZLIB_SYSTEM_IS_WX OR NOT CMAKE_MRPT_HAS_ZLIB_SYSTEM)
		include_directories("${MRPT_SOURCE_DIR}/3rdparty/zlib/")
		if(NOT MSVC OR NOT CMAKE_MRPT_HAS_ZLIB_SYSTEM_IS_WX)
			list(APPEND io_EXTRA_SRCS			"${MRPT_SOURCE_DIR}/3rdparty/zlib/*.c" "${MRPT_SOURCE_DIR}/3rdparty/zlib/*.h")
			list(APPEND io_EXTRA_SRCS_NAME 	"io/zlib" "io/zlib")
		endif()
	endif()
endif()

set_property(GLOBAL PROPERTY mrpt_io_UNIT_TEST_EXTRA_DEPS mrpt-random)


define_mrpt_lib(
	io 		# Lib name
	# Dependencies:
	mrpt-core
	mrpt-system
	)

if(BUILD_mrpt-io)
	# Ignore precompiled headers in some sources:
	if(MRPT_ENABLE_PRECOMPILED_HDRS AND MSVC)
		set_source_files_properties(
			${io/zlib_FILES}
			PROPERTIES COMPILE_FLAGS "/Y-"
			)
	endif()

	if(CMAKE_MRPT_HAS_ZLIB)
		if(NOT CMAKE_MRPT_HAS_ZLIB_SYSTEM_IS_WX AND CMAKE_MRPT_HAS_ZLIB_SYSTEM)
		    target_link_libraries(io PRIVATE ${ZLIB_LIBRARIES})
			target_include_directories(io PRIVATE ${ZLIB_INCLUDE_DIR})
		endif()
	endif()

	# Use wxWidgets version of libzip (gz* funtions)
	if(MSVC AND CMAKE_MRPT_HAS_WXWIDGETS)
	    target_link_libraries(io PRIVATE imp_wxwidgets)
	endif()
endif()
