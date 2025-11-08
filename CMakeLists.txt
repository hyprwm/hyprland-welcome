cmake_minimum_required(VERSION 3.19)

file(READ "${CMAKE_SOURCE_DIR}/VERSION" VER_RAW)
string(STRIP ${VER_RAW} HYPRLAND_WELCOME_VERSION)

add_compile_definitions(HYPRLAND_WELCOME_VERSION="${HYPRLAND_WELCOME_VERSION}")

project(
  hyprland-welcome
  VERSION ${HYPRLAND_WELCOME_VERSION}
  DESCRIPTION "Volume management center for Hyprland")

include(CTest)
include(CheckIncludeFile)
include(GNUInstallDirs)

set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(INCLUDE ${CMAKE_INSTALL_FULL_INCLUDEDIR})
set(LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(
  deps
  REQUIRED
  IMPORTED_TARGET
  hyprtoolkit
  pixman-1
  libdrm
  hyprutils
)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(
  -Wall
  -Wextra
  -Wno-unused-parameter
  -Wno-unused-value
  -Wno-missing-field-initializers
  -Wpedantic)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DEBUG)
  message(STATUS "Configuring hyprland-welcome in Debug")
  add_compile_definitions(hyprland-welcome_DEBUG)
else()
  add_compile_options(-O3)
  message(STATUS "Configuring hyprland-welcome in Release")
endif()

file(GLOB_RECURSE SRCFILES CONFIGURE_DEPENDS "src/*.cpp" "include/*.hpp")

add_executable(hyprland-welcome ${SRCFILES})

target_link_libraries(hyprland-welcome PkgConfig::deps)

install(
  FILES contrib/hyprland-welcome.desktop
  DESTINATION "share/applications")

install(TARGETS hyprland-welcome)