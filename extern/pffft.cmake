cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(pffft)

set(PFFFT_INCLUDE_DIRS "${DSPTB_EXTERN_DIR}pffft/")
include_directories(${PFFFT_INCLUDE_DIRS})

add_library(pffft
    STATIC
    EXCLUDE_FROM_ALL
    ${PFFFT_INCLUDE_DIRS}test_pffft.c
    ${PFFFT_INCLUDE_DIRS}fftpack.c
    ${PFFFT_INCLUDE_DIRS}pffft.c)

set_target_properties(pffft 
    PROPERTIES
        LINKER_LANGUAGE CXX
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        EXTERNAL_OBJECT True
        GENERATED False)

if (WIN32) 
    add_compile_definitions(_USE_MATH_DEFINES)
else()
    target_link_libraries(pffft m)
endif()