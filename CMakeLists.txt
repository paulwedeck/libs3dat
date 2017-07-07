cmake_minimum_required(VERSION 3.2.2)
project(s3dat)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")

include_directories(src)

#s3dat
file(GLOB_RECURSE s3dat_SOURCES "src/*.[ch]")

add_library(s3dat SHARED ${s3dat_SOURCES})

add_executable(cli cli.c)
target_link_libraries(cli s3dat)

find_package(PkgConfig)

if(PKGCONFIG_FOUND)

	pkg_check_modules(GL REQUIRED glfw3 gl)

	add_executable(walk walk.c)
	target_link_libraries(walk s3dat ${GL_LIBRARIES} m)
endif()
