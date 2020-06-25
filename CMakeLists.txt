cmake_minimum_required(VERSION 3.0)
project(switch-simulator)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -w -O3")

# The version number.
set(LWS_MAJOR_VERSION 0)
set(LWS_MINOR_VERSION 2)
set(LWS_PATCH_VERSION 0)
set(LWS_VERSION
        ${LWS_MAJOR_VERSION}.${LWS_MINOR_VERSION}.${LWS_PATCH_VERSION})


set(PROJECT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks)

configure_file(
        common/LWSConfig.h.in
        ${PROJECT_BINARY_DIR}/LWSConfig.h
)


option(LWS_BUILD_TESTS "Set to ON to build tests" OFF)

set(EXTERNAL ${CMAKE_CURRENT_SOURCE_DIR}/external)
include_directories(common ${PROJECT_BINARY_DIR} ${EXTERNAL}/include)

# common header files
set(common_headers
        "${CMAKE_CURRENT_SOURCE_DIR}/common/lws.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/common/json.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/common/cxxopts.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/common/random_variable.hpp")

set(common_libraries ${EXTERNAL}/lib/libhdr_histogram_static.a ${EXTERNAL}/lib/libfmt.a)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/C_Serenade")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/O_Serenade")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Serena")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/C_Serenade_for_Proofs_Sake")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/O_Serenade_for_Proof_Sake")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/iSLIP")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/C_Serenade_for_Proofs_Sake_V2")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/O_Serenade_for_Proof_Sake_V2")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/E_Serenade_V1")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/C_Serenade_for_Proofs_Sake_for_TON2018")

if (LWS_BUILD_TESTS)
    find_package(GTest REQUIRED)
    if (GTEST_FOUND)
        include_directories(${GTEST_INCLUDE_DIRS})
    else ()
        message(FATAL_ERROR "Can not find gtest")
    endif ()
    add_executable(runTests common/lws.hpp common/cxxopts.hpp common/json.hpp test/lws_test.cpp)
    target_link_libraries(runTests hdr_histogram fmt gtest)
endif ()


# handling data
FILE(GLOB data_files_ourobo "data-revised/Ouroboros/*.txt")
FILE(GLOB data_files_bsit "data-revised/BSIT/*.txt")

# handling experiments decription files
FILE(GLOB exp_files "py/*.yaml")

# copy dependent files
FOREACH (infileName ${data_files_ourobo})
    message(STATUS "Process data file: ${infileName}")
    # copy
    file(COPY ${infileName}
            DESTINATION ${PROJECT_BINARY_DIR}/data/Ouroboros)
ENDFOREACH (infileName)

FOREACH (infileName ${data_files_bsit})
    message(STATUS "Process BSIT file: ${infileName}")
    # copy
    file(COPY ${infileName}
            DESTINATION ${PROJECT_BINARY_DIR}/data/BSIT)
ENDFOREACH (infileName)

FOREACH (infileName ${exp_files})
    message(STATUS "Process experiment file: ${infileName}")
    # copy
    file(COPY ${infileName}
            DESTINATION ${PROJECT_BINARY_DIR}/configs)
    file(COPY py/pySimu.py
            DESTINATION ${PROJECT_BINARY_DIR})
ENDFOREACH (infileName)

#install(FILES ${data_files} DESTINATION data/Ouroboros)
install(FILES ${data_files_ourobo} DESTINATION data/Ouroboros)
install(FILES ${data_files_bsit} DESTINATION data/BSIT)
install(DIRECTORY results DESTINATION .)
install(FILES py/pySimu.py ${exp_files} DESTINATION bin)

