cmake_minimum_required(VERSION 3.14)


###############################################################################
# Recurse git submodules
###############################################################################
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
RESULT_VARIABLE GIT_SUBMOD_RESULT)
if(NOT GIT_SUBMOD_RESULT EQUAL "0")
message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
endif()
endif()
###############################################################################

set(DSPTB_EXTERN_DIR "${CMAKE_SOURCE_DIR}/extern/")
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
include(ExternalProject)

include(extern/pffft.cmake)


# add_subdirectory(${DSPTB_EXTERN_DIR}/libsndfile)
# set_target_properties(sndfile PROPERTIES BUILD_EXAMPLES OFF)
# set_target_properties(sndfile PROPERTIES BUILD_SHARED_LIBS ON)

project(dsptb VERSION 1.0
DESCRIPTION "DSP Toolbox"
LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()


include(CTest)
enable_testing()

add_subdirectory(dsptb)
add_subdirectory(sandbox)
add_subdirectory(test)

set(CMAKE_INSTALL_PREFIX "install")
install(TARGETS dsptb sandbox
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION "include")

