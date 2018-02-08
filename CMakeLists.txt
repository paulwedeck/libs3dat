cmake_minimum_required(VERSION 3.2.2)
project(libs3dat)

add_subdirectory(libs3util)

include(CPack)
include(CTest)
include(TestBigEndian)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")
set(CPACK_PACKAGE_NAME "libs3dat")

set(STRING_BUFFER 1024)

TEST_BIG_ENDIAN(IS_BE)

option(PRIVATE_FILENAME "stop leaking complete path to file (should be enabled on distribution)" ON)

configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)

include_directories(src ${PROJECT_BINARY_DIR} libs3util/src)


#s3dat
file(GLOB_RECURSE s3dat_SOURCES "src/*.[ch]")

add_library(s3dat SHARED ${s3dat_SOURCES})
target_link_libraries(s3dat s3util)

add_executable(cli cli.c)
target_link_libraries(cli s3dat)

add_executable(gen_gui gen_gui.c)
target_link_libraries(gen_gui s3dat m)

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

