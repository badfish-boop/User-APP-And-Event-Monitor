# CMake 最低版本号要求

cmake_minimum_required (VERSION 2.8)

# 项目信息

project (login_monitor)

#增加指定头文件
find_package(PkgConfig)

pkg_check_modules(GLib REQUIRED glib-2.0)

if(GLib_FOUND)
include_directories(${GLib_INCLUDE_DIRS})
link_libraries(${GLib_LIBRARIES})
endif(GLib_FOUND)

pkg_check_modules(DBus REQUIRED dbus-1)

if(DBus_FOUND)
include_directories(${DBus_INCLUDE_DIRS})
link_libraries(${DBus_LIBRARIES})
endif(DBus_FOUND)

pkg_check_modules(DBus-GLib REQUIRED dbus-glib-1)

if(DBus-GLib_FOUND)
include_directories(${DBus-GLib_INCLUDE_DIRS})
link_libraries(${DBus-GLib_LIBRARIES})
endif(DBus-GLib_FOUND)

pkg_check_modules(GIo REQUIRED gio-2.0)

if(GIo_FOUND)
include_directories(${GIo_INCLUDE_DIRS})
link_libraries(${GIo_LIBRARIES})
endif(GIo_FOUND)

include_directories(/usr/include/glib-2.0/glib/)

aux_source_directory(./src/server SRC_MAIN)

add_definitions("-Wall -g")

# 指定生成目标

add_executable(login_monitor ${SRC_MAIN})

target_link_libraries(login_monitor libglib-2.0.so libdbus-1.so libdbus-glib-1.so libpthread.so)

set_target_properties(
login_monitor
PROPERTIES
RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../"
)

