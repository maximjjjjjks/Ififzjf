cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(FallbackSong VERSION 1.0.0)

add_library(${PROJECT_NAME} SHARED src/main.cpp)

if (NOT DEFINED ENV{GEODE_SDK})
	message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable.")
endif()


# Workaround: Geode SDK 5.0.0 header has a duplicate init() declaration
# that newer Clang (NDK 29) rejects. Remove the duplicate before building.
set(_hdr "$ENV{GEODE_SDK}/loader/include/Geode/cocos/platform/win32/CCFileUtilsWin32.h")
if (EXISTS "${_hdr}")
	file(READ "${_hdr}" _c)
	string(REGEX REPLACE "\n[ \t]+bool init\\(\\);" "" _c_new "${_c}")
	if (NOT "${_c}" STREQUAL "${_c_new}")
		file(WRITE "${_hdr}" "${_c_new}")
		message(STATUS "Patched CCFileUtilsWin32.h")
	endif()
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)
setup_geode_mod(${PROJECT_NAME})
