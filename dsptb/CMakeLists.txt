cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall")
#set(CMAKE_CXX_FLAGS_DEBUG "-g -Od")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

list(APPEND 
     SOURCES
     "${CMAKE_SOURCE_DIR}/dsptb/dsptb.cpp"
     "${CMAKE_SOURCE_DIR}/dsptb/dsptb.hpp"
     "${CMAKE_SOURCE_DIR}/dsptb/utils.hpp"
     "${CMAKE_SOURCE_DIR}/dsptb/filter.cpp"
     "${CMAKE_SOURCE_DIR}/dsptb/filter.hpp"
     "${CMAKE_SOURCE_DIR}/dsptb/ir.hpp"
     "${CMAKE_SOURCE_DIR}/dsptb/ir.cpp"
     "${CMAKE_SOURCE_DIR}/dsptb/processing.hpp"
     "${CMAKE_SOURCE_DIR}/dsptb/processing.cpp"
)
list(APPEND 
     HEADERS 
     "${CMAKE_SOURCE_DIR}/include/dsptb.h")

add_library(dsptb SHARED ${SOURCES})
target_link_libraries(dsptb PRIVATE pffft)
target_link_libraries(dsptb PRIVATE mysofa-static)
target_sources(dsptb PUBLIC ${HEADERS})

set_target_properties(dsptb
    PROPERTIES
    PUBLIC_HEADER "${HEADERS}"
)
target_include_directories(dsptb
    PUBLIC 
        "${CMAKE_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}")
