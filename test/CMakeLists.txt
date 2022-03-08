cmake_minimum_required(VERSION 3.14)

#
# PFFFT
#
add_executable(test_pffft ${PFFFT_INCLUDE_DIRS}test_pffft.c)
target_link_libraries(test_pffft PRIVATE pffft)
set_target_properties(test_pffft PROPERTIES LINKER_LANGUAGE CXX)
target_link_directories(test_pffft PUBLIC "${PFFFT_INCLUDE_DIRS}")
enable_testing()
add_test(asr_test_pffft test_pffft)