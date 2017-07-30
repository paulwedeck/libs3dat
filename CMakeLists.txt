cmake_minimum_required(VERSION 3.2.2)
project(s3dat)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")

option(FORCE_ICONV "Force iconv even if no iconv.h is found" OFF)
option(DISABLE_ICONV "Disables testing for iconv" OFF)

if(NOT DISABLE_ICONV AND NOT FORCE_ICONV)
	INCLUDE (CheckIncludeFiles)
	CHECK_INCLUDE_FILES(iconv.h ICONV_H_FOUND)
endif()

if(ICONV_H_FOUND)
	set(USE_ICONV 1)
endif()

if(FORCE_ICONV)
	set(USE_ICONV 1)
endif()

if(DISABLE_ICONV)
	unset(USE_ICONV)
endif()

if(NOT USE_ICONV)
	message(WARNING "libs3dat needs iconv for good iso8859-2 (Polish) to utf8 conversion and EUC-KR (Korean) or SHIFT_JIS (Japanese) to utf8 conversion.\nIso8859-1 (western Europa) to utf8 conversion is builtin")
endif()

configure_file(config.h.in ${PROJECT_SOURCE_DIR}/src/config.h)

include_directories(src)

#s3dat
file(GLOB_RECURSE s3dat_SOURCES "src/*.[ch]")

add_library(s3dat SHARED ${s3dat_SOURCES})

add_executable(cli cli.c)
target_link_libraries(cli s3dat)

find_package(glfw3)
find_package(OpenGL)

if(glfw3_FOUND AND OpenGL_FOUND)

	add_executable(walk walk.c)
	target_link_libraries(walk s3dat ${OPENGL_LIBRARIES} glfw m)
	target_include_directories(walk PUBLIC ${OPENGL_INCLUDE_DIRS} ${glfw3_INCLUDE_DIRS})
endif()
