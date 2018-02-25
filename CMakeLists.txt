cmake_minimum_required(VERSION 3.2.2)
project(libs3dat VERSION 1.0.0)

option(PRIVATE_FILENAME "stop leaking complete path to file (should be enabled on distribution)" ON)

include(CTest)
include(TestBigEndian)
include(CMakePackageConfigHelpers)

set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}/bin")
set(CPACK_PACKAGE_NAME "libs3dat")
set(CPACK_PACKAGE_VERSION ${libs3dat_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${libs3dat_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${libs3dat_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${libs3dat_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libs3util (=1.0.0)")
include(CPack)

set(STRING_BUFFER 1024)

TEST_BIG_ENDIAN(IS_BE)

find_package(s3util REQUIRED)


configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.s3dat.h)
configure_file(libs3dat.pc.in ${PROJECT_BINARY_DIR}/libs3dat.pc @ONLY)

include_directories(src ${PROJECT_BINARY_DIR} ${s3util_INCLUDE_DIRS})


#s3dat
file(GLOB_RECURSE s3dat_SOURCES "src/*.[ch]")

add_library(s3dat SHARED ${s3dat_SOURCES})
target_link_libraries(s3dat s3util)
target_include_directories(s3dat PUBLIC $<INSTALL_INTERFACE:include>)

add_executable(cli cli.c)
target_link_libraries(cli s3dat)

add_executable(gen_gui gen_gui.c)
target_link_libraries(gen_gui s3dat m)

add_executable(invert invert.c)
target_link_libraries(invert s3dat)

find_package(Qt5Widgets)

if(Qt5Widgets_FOUND)
	add_executable(search search.cpp)
	target_link_libraries(search s3dat Qt5::Widgets)
endif()

find_package(glfw3)
find_package(OpenGL)

if(glfw3_FOUND AND OpenGL_FOUND)

	add_executable(animation animation.c)
	target_link_libraries(animation s3dat ${OPENGL_LIBRARIES} glfw m)
	target_include_directories(animation PUBLIC ${OPENGL_INCLUDE_DIRS} ${glfw3_INCLUDE_DIRS})

	add_executable(walk walk.c)
	target_link_libraries(walk s3dat ${OPENGL_LIBRARIES} glfw m)
	target_include_directories(walk PUBLIC ${OPENGL_INCLUDE_DIRS} ${glfw3_INCLUDE_DIRS})
endif()

write_basic_package_version_file(${PROJECT_BINARY_DIR}/s3datConfigVersion.cmake COMPATIBILITY ExactVersion)
configure_package_config_file(s3datConfig.cmake.in s3datConfig.cmake INSTALL_DESTINATION lib/cmake/s3dat PATH_VARS CMAKE_INSTALL_PREFIX CMAKE_SOURCE_DIR)

install(TARGETS s3dat EXPORT s3datTarget DESTINATION lib)
install(EXPORT s3datTarget DESTINATION lib/cmake/s3dat)

#for find_package on build dirs
export(TARGETS s3dat FILE s3datTarget.cmake)

install(FILES src/s3dat_ext.h src/s3dat.h DESTINATION include/s3dat)
install(FILES ${PROJECT_BINARY_DIR}/libs3dat.pc DESTINATION lib/pkgconfig)
install(FILES ${PROJECT_BINARY_DIR}/s3datConfigVersion.cmake ${PROJECT_BINARY_DIR}/s3datConfig.cmake DESTINATION lib/cmake/s3dat)
install(FILES LICENSE DESTINATION share/doc/libs3dat)

add_executable(testme test.c)
target_link_libraries(testme s3dat)

foreach(test_name search_gfx open_landscape try_blending)
	add_test(${test_name} bin/testme ${test_name})
endforeach()

