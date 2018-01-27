cmake_minimum_required(VERSION 3.2.2)
project(libs3dat)

add_subdirectory(libs3util)

include(CPack)
include(CTest)
include(TestBigEndian)

set(CMAKE_C_FLAGS "-Wall -Wno-pointer-sign")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")
set(CPACK_PACKAGE_NAME "libs3dat")

TEST_BIG_ENDIAN(IS_BE)

option(FORCE_ICONV "Force iconv even if no iconv.h is found" OFF)
option(DISABLE_ICONV "Disables testing for iconv" OFF)
option(PRIVATE_FILENAME "stop leaking complete path to file (should be enabled on distribution)" ON)

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
	message(WARNING "libs3dat needs iconv for good iso8859-2 (Polish) to utf8 conversion and EUC-KR (Korean) or SHIFT_JIS (Japanese) to utf8 conversion.\nISO8859-1 (western Europa) to utf8 conversion is builtin")
endif()

configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)

include_directories(src ${PROJECT_BINARY_DIR} libs3util/src)


#s3dat
file(GLOB_RECURSE s3dat_SOURCES "src/*.[ch]")

add_library(s3dat SHARED ${s3dat_SOURCES})
target_link_libraries(s3dat s3util)

add_executable(cli cli.c)
target_link_libraries(cli s3dat)

add_executable(invert invert.c)
target_link_libraries(invert s3dat)

find_package(glfw3)
find_package(OpenGL)

if(glfw3_FOUND AND OpenGL_FOUND)

	add_executable(walk walk.c)
	target_link_libraries(walk s3dat ${OPENGL_LIBRARIES} glfw m)
	target_include_directories(walk PUBLIC ${OPENGL_INCLUDE_DIRS} ${glfw3_INCLUDE_DIRS})
endif()

install(TARGETS s3dat DESTINATION lib)
install(FILES src/s3dat_ext.h src/s3dat.h DESTINATION include/s3dat)

add_executable(testme test.c)
target_link_libraries(testme s3dat)

foreach(test_name search_gfx open_landscape try_blending)
	add_test(${test_name} bin/testme ${test_name})
endforeach()

