cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_executable(sandbox sandbox.cpp)
target_compile_features(sandbox PUBLIC cxx_std_20)
target_include_directories(sandbox PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_include_directories(sandbox PRIVATE "${DSPTB_EXTERN_DIR}/libmysofa/src/hrtf")
target_link_libraries(sandbox PRIVATE dsptb)
target_link_libraries(sandbox PRIVATE mysofa-static)
target_link_libraries(sandbox PRIVATE zlib)
target_link_libraries(sandbox PRIVATE sndfile)

# add_custom_command(TARGET asr POST_BUILD        # Adds a post-build event to MyTest
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
#         "${PROJECT_SOURCE_DIR}/libs/test.dll"      # <--this is in-file
#         $<TARGET_FILE_DIR:MyTest>)                 # <--this is out-file path

add_custom_command(TARGET sandbox POST_BUILD 
               COMMAND ${CMAKE_COMMAND} -E copy_if_different
               $<TARGET_FILE:dsptb>
               $<TARGET_FILE_DIR:sandbox>)