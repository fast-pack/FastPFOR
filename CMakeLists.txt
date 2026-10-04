# This code is released under the
# Apache License Version 2.0 http://www.apache.org/licenses/.
#
# Copyright (c) 2012 Louis Dionne
#
cmake_minimum_required(VERSION 3.10)

set (CMAKE_CXX_STANDARD 11) # for constexpr specifier and other goodies
set(CMAKE_CXX_STANDARD_REQUIRED True)
set (CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")
include(AppendCompilerFlags)

include(cmake/CPM.cmake)

project(FastPFOR CXX C)
set(PROJECT_URL "https://github.com/fast-pack/FastPFOR")
set(PROJECT_DESCRIPTION "The FastPFOR C++ library: Fast integer compression")
# Need to bump this when we release a new version.
set(PROJECT_VERSION "0.5.1")

include(DetectCPUFeatures)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/environment.cmake")


message("Building for architecture: ${CMAKE_SYSTEM_PROCESSOR}")


MESSAGE( STATUS "CMAKE_SIZEOF_VOID_P (should be 8): " ${CMAKE_SIZEOF_VOID_P} )
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
   MESSAGE( STATUS "Good. You appear to have a 64-bit system. " )
else()
   MESSAGE( STATUS "Please use a 64-bit system. " )
endif()
MESSAGE( STATUS "CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID} )
MESSAGE( STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER} )
if( SUPPORT_SSE42 )
    MESSAGE( STATUS "SSE 4.2 support detected" )
else()
    if (SUPPORT_NEON)
        MESSAGE(STATUS "Using native ARM NEON intrinsics for SIMD operations")
    else ()
        MESSAGE(STATUS "Neither SSE 4.2 nor ARM NEON support detected")
    endif ()
endif()


# GNUInstallDirs defines CMAKE_INSTALL_INCLUDEDIR (and friends), which is used
# below in the INSTALL_INTERFACE include path. It must be included before that
# expansion, otherwise the variable is empty and the exported target ends up
# with a bogus "/fastpfor" include directory that breaks find_package().
include(GNUInstallDirs)

# library target
add_library(FastPFOR STATIC)
target_include_directories(FastPFOR PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/headers>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/fastpfor>
)
target_sources(FastPFOR PRIVATE
    src/bitpacking.cpp
    src/bitpackingaligned.cpp
    src/bitpackingunaligned.cpp
    src/horizontalbitpacking.cpp
    src/simdunalignedbitpacking.cpp
    src/codecfactory.cpp
    src/simdbitpacking.cpp
    src/varintdecode.c
    src/streamvbyte.c
)
set_target_properties(FastPFOR PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

option(FASTPFOR_SANITIZE "Use sanitizers" OFF)

if(FASTPFOR_SANITIZE)
  message(STATUS "Enabling Sanitizers")
  target_compile_options(FastPFOR PUBLIC -fsanitize=address  -fno-omit-frame-pointer -fno-sanitize-recover=all)
  target_compile_definitions(FastPFOR PUBLIC ASAN_OPTIONS=detect_leaks=1)
  target_link_libraries(FastPFOR PUBLIC -fsanitize=address  -fno-omit-frame-pointer -fno-sanitize-recover=all)
endif()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES AND NOT FASTPFOR_SANITIZE)
		message(STATUS "No build type selected, default to Release")
		set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()
MESSAGE( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )


if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
   target_compile_options(FastPFOR PRIVATE -Wall -Wextra  -Weffc++ -Wsign-compare -Wshadow -Wwrite-strings -Wpointer-arith -Winit-self -Wconversion -Wno-sign-conversion)
endif()
include(CheckCXXCompilerFlag)

# SIMD mode: native, portable, or runtime
# - native: Use -march=native for maximum performance on the build machine (not portable)
# - portable: Use baseline SSE4.2 only for maximum compatibility (default)
# - runtime: Use function multi-versioning for runtime CPU dispatch (future)
set(FASTPFOR_SIMD_MODE "native" CACHE STRING "SIMD compilation mode: native, portable, or runtime")
set_property(CACHE FASTPFOR_SIMD_MODE PROPERTY STRINGS native portable runtime)

if(FASTPFOR_SIMD_MODE STREQUAL "native")
    unset(FASTPFOR_COMPILER_SUPPORTS_MARCH_NATIVE CACHE)
    CHECK_CXX_COMPILER_FLAG(-march=native FASTPFOR_COMPILER_SUPPORTS_MARCH_NATIVE)
    if(FASTPFOR_COMPILER_SUPPORTS_MARCH_NATIVE)
        target_compile_options(FastPFOR PRIVATE -march=native)
    else()
        message(STATUS "native target not supported, falling back to portable mode")
        target_compile_options(FastPFOR PRIVATE -msse4.2)
    endif()
elseif(FASTPFOR_SIMD_MODE STREQUAL "portable")
    # Baseline: SSE4.2 is required by FastPFOR SIMD code
    target_compile_options(FastPFOR PRIVATE -msse4.2)
elseif(FASTPFOR_SIMD_MODE STREQUAL "runtime")
    # Runtime dispatch: compile with baseline SSE4.2 and enable multi-versioning
    target_compile_options(FastPFOR PRIVATE -msse4.2)
    target_compile_definitions(FastPFOR PRIVATE FASTPFOR_RUNTIME_DISPATCH)
    message(STATUS "Runtime dispatch mode is experimental")
else()
    message(FATAL_ERROR "Invalid FASTPFOR_SIMD_MODE: ${FASTPFOR_SIMD_MODE}. Use native, portable, or runtime.")
endif()
message(STATUS "FASTPFOR_SIMD_MODE: ${FASTPFOR_SIMD_MODE}")

MESSAGE( STATUS "CMAKE_CXX_FLAGS_DEBUG: " ${CMAKE_CXX_FLAGS_DEBUG} )
MESSAGE( STATUS "CMAKE_CXX_FLAGS_RELEASE: " ${CMAKE_CXX_FLAGS_RELEASE} )
MESSAGE( STATUS "CMAKE_C_FLAGS_DEBUG: " ${CMAKE_C_FLAGS_DEBUG} )
MESSAGE( STATUS "CMAKE_C_FLAGS_RELEASE: " ${CMAKE_C_FLAGS_RELEASE} )


# other executables
add_executable(gapstats src/gapstats.cpp)
target_link_libraries(gapstats PRIVATE FastPFOR)

add_executable(partitionbylength src/partitionbylength.cpp)
target_link_libraries(partitionbylength PRIVATE FastPFOR)

add_executable(csv2maropu src/csv2maropu.cpp)
target_link_libraries(csv2maropu PRIVATE FastPFOR)


add_executable(entropy src/entropy.cpp)
target_link_libraries(entropy FastPFOR)

if( SUPPORT_SSE42 )
    add_executable(benchbitpacking src/benchbitpacking.cpp)
    target_link_libraries(benchbitpacking FastPFOR)
endif()

find_package(snappy)
if(NOT ${snappy_FOUND})
    message(STATUS "Snappy was not found. codecssnappy and "
                   "inmemorybenchmarksnappy targets are not available.")
else()
    message(STATUS "Snappy was found. Building additional targets "
                   "codecssnappy and inmemorybenchmarksnappy.")
    include_directories(${snappy_INCLUDE_DIRS})
    add_executable(codecssnappy src/codecs.cpp)
    set_target_properties(codecssnappy PROPERTIES DEFINE_SYMBOL USESNAPPY)
    target_link_libraries(codecssnappy FastPFOR ${snappy_LIBRARIES})

    add_executable(inmemorybenchmarksnappy src/inmemorybenchmark.cpp)
    set_target_properties(inmemorybenchmarksnappy PROPERTIES DEFINE_SYMBOL USESNAPPY)
    target_link_libraries(inmemorybenchmarksnappy FastPFOR ${snappy_LIBRARIES})
endif()

option(FASTPFOR_WITH_TEST "Build with Google Test" ON)

if(FASTPFOR_WITH_TEST)
  CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG  v1.17.0
    VERSION 1.17.0
    OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
 )

  add_executable(codecs src/codecs.cpp)
  target_link_libraries(codecs FastPFOR)

  add_executable(example example.cpp)
  target_link_libraries(example FastPFOR)

  add_executable(inmemorybenchmark src/inmemorybenchmark.cpp)
  target_link_libraries(inmemorybenchmark FastPFOR)

  add_executable(unit src/unit.cpp)
  target_link_libraries(unit FastPFOR)
  add_custom_target(check unit DEPENDS unit)

  add_executable(FastPFOR_unittest
    unittest/test_composite.cpp
    unittest/test_driver.cpp
    unittest/test_fastpfor.cpp
    unittest/test_simple8b.cpp
    unittest/test_simple16.cpp
    unittest/test_variablebyte.cpp)
  target_link_libraries(FastPFOR_unittest gtest FastPFOR)
  enable_testing()
  add_test("FastPFOR_unittest" FastPFOR_unittest)
endif()

install(TARGETS FastPFOR
    EXPORT FastPFORExport
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
set(PACKAGE_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/FastPFOR")
install(EXPORT FastPFORExport
    FILE "FastPFORTargets.cmake"
    DESTINATION "${PACKAGE_CMAKE_INSTALL_DIR}"
    NAMESPACE "FastPFOR::")
install(DIRECTORY "headers/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fastpfor")
install(FILES AUTHORS LICENSE README.md
    DESTINATION "${CMAKE_INSTALL_DOCDIR}")
if(NOT CMAKE_VERSION VERSION_LESS 3.0.0)
    include(CMakePackageConfigHelpers)
    configure_package_config_file("FastPFORConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/FastPFORConfig.cmake"
        INSTALL_DESTINATION "${PACKAGE_CMAKE_INSTALL_DIR}")
    write_basic_package_version_file("FastPFORConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion)
    install(FILES
       "${CMAKE_CURRENT_BINARY_DIR}/FastPFORConfig.cmake"
       "${CMAKE_CURRENT_BINARY_DIR}/FastPFORConfigVersion.cmake"
        DESTINATION "${PACKAGE_CMAKE_INSTALL_DIR}")
endif()
configure_file("fastpfor.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/fastpfor.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/fastpfor.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")


if (SUPPORT_NEON)
    message(STATUS "Building with native ARM NEON support (no SIMD emulation layer).")
endif()
