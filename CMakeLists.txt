#  @file CMakeLists.txt
#  @author Gaspard Kirira
#
#  Copyright 2025, Gaspard Kirira.
#  All rights reserved.
#  https://github.com/vixcpp/vix
#
#  Use of this source code is governed by a MIT license
#  that can be found in the License file.
#
# ====================================================================
# Vix.cpp - Io Module
# ====================================================================
# Purpose:
#   Simple, modern, and cross-platform I/O utilities for Vix.
#
#   This module owns lightweight developer-facing APIs:
#     - vix/io/*
#     - vix/print.hpp
#     - vix/format.hpp
#     - vix/inspect.hpp
#     - vix/input.hpp
#     - vix/console.hpp
#
# Public Targets:
#   - vix_io  : The actual library target
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

project(vix_io
  VERSION 0.1.0
  LANGUAGES CXX
)

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
# Options
# --------------------------------------------------------------------
option(VIX_IO_FETCH_ERROR "Auto-fetch vix::error if missing" ON)
option(VIX_IO_BUILD_TESTS "Build io module tests" OFF)

# Tracks whether standalone io used local sibling dependencies.
# If yes, we skip standalone package export because local sibling targets
# are not part of vix_ioTargets.
set(VIX_IO_HAS_LOCAL_DEPENDENCIES OFF)

# --------------------------------------------------------------------
# Umbrella build policy
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
# Dependency resolver
# --------------------------------------------------------------------
function(vix_io_resolve_dependency module_name target_name package_name fetch_option git_url git_tag)
  if (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD)
    if (NOT TARGET ${target_name})
      message(FATAL_ERROR "[io] Umbrella build: ${target_name} must be provided before io.")
    endif()

    return()
  endif()

  if (TARGET ${target_name})
    return()
  endif()

  find_package(${package_name} CONFIG QUIET)

  if (TARGET ${target_name})
    return()
  endif()

  get_filename_component(_VIX_MODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
  set(_VIX_DEP_DIR "${_VIX_MODULES_DIR}/${module_name}")

  if (EXISTS "${_VIX_DEP_DIR}/CMakeLists.txt")
    message(STATUS "[io] Adding local sibling ${module_name}: ${_VIX_DEP_DIR}")
    set(VIX_IO_HAS_LOCAL_DEPENDENCIES ON PARENT_SCOPE)
    add_subdirectory("${_VIX_DEP_DIR}" "${CMAKE_BINARY_DIR}/_vix_${module_name}")
  endif()

  if (TARGET ${target_name})
    return()
  endif()

  if (${fetch_option})
    include(FetchContent)

    message(STATUS "[io] Fetching ${target_name} via FetchContent")

    fetchcontent_declare(${package_name}
      GIT_REPOSITORY ${git_url}
      GIT_TAG        ${git_tag}
    )

    fetchcontent_makeavailable(${package_name})
  else()
    message(FATAL_ERROR
      "${target_name} not found.\n"
      "Expected one of:\n"
      "  - existing target: ${target_name}\n"
      "  - installed package: find_package(${package_name} CONFIG)\n"
      "  - local sibling module: ../${module_name}\n"
      "  - FetchContent enabled through ${fetch_option}=ON\n"
    )
  endif()

  if (NOT TARGET ${target_name})
    message(FATAL_ERROR "[io] ${target_name} not found after dependency resolution.")
  endif()
endfunction()

# --------------------------------------------------------------------
# Required dependencies
# --------------------------------------------------------------------
vix_io_resolve_dependency(
  error
  vix::error
  vix_error
  VIX_IO_FETCH_ERROR
  https://github.com/vixcpp/error.git
  v0.1.0
)

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
      $<BUILD_INTERFACE:${VIX_ERROR_TARGET}>
      $<INSTALL_INTERFACE:vix::error>
  )

  if (NOT MSVC)
    target_compile_options(vix_io PRIVATE
      -Wall
      -Wextra
      -Wpedantic
    )
  endif()

  if (VIX_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(vix_io PUBLIC
      $<BUILD_INTERFACE:-fno-omit-frame-pointer>
      $<BUILD_INTERFACE:-fsanitize=address,undefined>
    )

    target_link_options(vix_io PUBLIC
      $<BUILD_INTERFACE:-fsanitize=address,undefined>
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
  message(STATUS "[io] Building HEADER-ONLY library with no sources.")

  add_library(vix_io INTERFACE)
  add_library(vix::io ALIAS vix_io)

  target_compile_features(vix_io INTERFACE cxx_std_20)

  target_include_directories(vix_io INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  target_link_libraries(vix_io
    INTERFACE
      $<BUILD_INTERFACE:${VIX_ERROR_TARGET}>
      $<INSTALL_INTERFACE:vix::error>
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
      PATTERN "*.inc"
  )
endif()

# --------------------------------------------------------------------
# Standalone package config/export
# --------------------------------------------------------------------
if (NOT (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD))
  if (VIX_IO_HAS_LOCAL_DEPENDENCIES)
    message(STATUS "[io] Local sibling dependencies detected; skipping standalone package export.")
    message(STATUS "[io] Installable package export requires installed dependency packages.")
  else()
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
endif()

# --------------------------------------------------------------------
# Tests
# --------------------------------------------------------------------
if (VIX_IO_BUILD_TESTS)
  include(CTest)
  enable_testing()

  if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")
    add_subdirectory(test)
  else()
    message(WARNING "[io/tests] VIX_IO_BUILD_TESTS=ON but test/CMakeLists.txt not found, skipping.")
  endif()
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

message(STATUS "Export set:         ${VIX_IO_EXPORT_SET}")
message(STATUS "Error target:       ${VIX_ERROR_TARGET}")
message(STATUS "Local deps:         ${VIX_IO_HAS_LOCAL_DEPENDENCIES}")
message(STATUS "Tests:              ${VIX_IO_BUILD_TESTS}")
message(STATUS "Include dir:        ${CMAKE_CURRENT_SOURCE_DIR}/include")
message(STATUS "Public headers:")
message(STATUS "  - <vix/io/...>")
message(STATUS "  - <vix/meta/...>")
message(STATUS "  - <vix/print.hpp>")
message(STATUS "  - <vix/format.hpp>")
message(STATUS "  - <vix/inspect.hpp>")
message(STATUS "  - <vix/input.hpp>")
message(STATUS "  - <vix/console.hpp>")
message(STATUS "Build type:         ${CMAKE_BUILD_TYPE}")
message(STATUS "------------------------------------------------------")
