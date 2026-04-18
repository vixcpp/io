#  @file CMakeLists.txt
#  @author Gaspard Kirira
#
#  Copyright 2025, Gaspard Kirira. All rights reserved.
#  https://github.com/vixcpp/vix
#  Use of this source code is governed by a MIT license
#  that can be found in the License file.
#
# ====================================================================
# Vix.cpp - Io Module
# ====================================================================
# Purpose:
#   Simple, modern, and cross-platform I/O utilities for Vix.
#   Provides developer-friendly APIs for reading, writing, buffers,
#   and standard input/output streams.
#
#   Builds as STATIC when sources exist, otherwise as a header-only
#   INTERFACE target.
#
# Public Targets:
#   - vix_io  : The actual library target (STATIC or INTERFACE)
#   - vix::io : Namespaced alias for consumers
#
# Public Dependencies:
#   - vix::error
#
# Installation/Export:
#   - Umbrella build exports into: VixTargets
#   - Standalone build exports into: vix_ioTargets
# ====================================================================

cmake_minimum_required(VERSION 3.20)
project(vix_io VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# --------------------------------------------------------------------
# Global settings
# --------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --------------------------------------------------------------------
# Export set selection
# --------------------------------------------------------------------
if (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD)
  set(VIX_IO_EXPORT_SET VixTargets)
else()
  set(VIX_IO_EXPORT_SET vix_ioTargets)
endif()

# --------------------------------------------------------------------
# Umbrella build policy:
# When building inside the Vix umbrella, dependencies are managed by the
# umbrella (add_subdirectory order + options). Io must not FetchContent
# nor add sibling subdirectories in umbrella mode.
# --------------------------------------------------------------------
if (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD)
  set(VIX_IO_FETCH_ERROR OFF CACHE BOOL "Auto-fetch vix::error if missing" FORCE)
endif()

# --------------------------------------------------------------------
# Sources discovery
# --------------------------------------------------------------------
file(GLOB_RECURSE IO_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# --------------------------------------------------------------------
# Error dependency
# --------------------------------------------------------------------
option(VIX_IO_FETCH_ERROR "Auto-fetch vix::error if missing" ON)

if (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD)
  if (NOT TARGET vix::error)
    message(FATAL_ERROR "[io] Umbrella build: vix::error must be provided by umbrella before io.")
  endif()
else()
  if (NOT TARGET vix::error)
    if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../error/CMakeLists.txt")
      message(STATUS "[io] Adding error from sibling: ../error")
      add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../error" "error")
    else()
      find_package(vix_error CONFIG QUIET)
    endif()

    if (NOT TARGET vix::error)
      if (VIX_IO_FETCH_ERROR)
        include(FetchContent)
        message(STATUS "[io] Fetching vix::error via FetchContent")
        FetchContent_Declare(vix_error
          GIT_REPOSITORY https://github.com/vixcpp/error.git
          GIT_TAG        v0.1.0
        )
        FetchContent_MakeAvailable(vix_error)
      else()
        message(FATAL_ERROR
          "vix::error not found. Enable VIX_IO_FETCH_ERROR=ON, install vix_error, or provide the target before io."
        )
      endif()
    endif()
  endif()
endif()

set(VIX_ERROR_TARGET vix::error)

# --------------------------------------------------------------------
# Library target
# --------------------------------------------------------------------
if (IO_SOURCES)
  message(STATUS "[io] Building STATIC library with detected sources.")

  add_library(vix_io STATIC ${IO_SOURCES})
  add_library(vix::io ALIAS vix_io)

  target_compile_features(vix_io PUBLIC cxx_std_20)

  target_include_directories(vix_io
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  target_link_libraries(vix_io
    PUBLIC
      ${VIX_ERROR_TARGET}
  )

  # Build-only helpers must not leak into exported interfaces.
  if (NOT MSVC)
    target_compile_options(vix_io PRIVATE
      -Wall
      -Wextra
      -Wpedantic
    )
  endif()

  if (VIX_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(vix_io PRIVATE
      -fno-omit-frame-pointer
      -fsanitize=address,undefined
    )
    target_link_options(vix_io PRIVATE
      -fsanitize=address,undefined
    )
  endif()

  set_target_properties(vix_io PROPERTIES
    OUTPUT_NAME vix_io
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    EXPORT_NAME io
  )

  install(TARGETS vix_io
    EXPORT ${VIX_IO_EXPORT_SET}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

else()
  message(STATUS "[io] Building HEADER-ONLY library (no sources).")

  add_library(vix_io INTERFACE)
  add_library(vix::io ALIAS vix_io)

  target_compile_features(vix_io INTERFACE cxx_std_20)

  target_include_directories(vix_io INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  target_link_libraries(vix_io INTERFACE
    ${VIX_ERROR_TARGET}
  )

  install(TARGETS vix_io
    EXPORT ${VIX_IO_EXPORT_SET}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()

# --------------------------------------------------------------------
# Install headers
# --------------------------------------------------------------------
if (NOT (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD))
  install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
      PATTERN "*.hpp"
      PATTERN "*.h"
  )
endif()

# --------------------------------------------------------------------
# Standalone package config/export
# Only for non-umbrella builds
# --------------------------------------------------------------------
if (NOT (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD))

  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/vix_ioConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/vix_ioConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/vix_io"
  )

  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/vix_ioConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/vix_ioConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/vix_ioConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/vix_io"
  )

  install(EXPORT vix_ioTargets
    FILE vix_ioTargets.cmake
    NAMESPACE vix::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/vix_io"
  )

endif()

# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------
option(VIX_IO_BUILD_TESTS "Build io module tests" OFF)

if (VIX_IO_BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(test)
endif()

# --------------------------------------------------------------------
# Summary
# --------------------------------------------------------------------
message(STATUS "------------------------------------------------------")
message(STATUS "vix::io configured (${PROJECT_VERSION})")
if (IO_SOURCES)
  message(STATUS "Mode: STATIC / sources found")
else()
  message(STATUS "Mode: HEADER-ONLY / no sources")
endif()
message(STATUS "Export set: ${VIX_IO_EXPORT_SET}")
message(STATUS "Error target: ${VIX_ERROR_TARGET}")
message(STATUS "Include dir: ${CMAKE_CURRENT_SOURCE_DIR}/include (for <vix/io/...>)")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "------------------------------------------------------")
