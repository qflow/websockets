cmake_minimum_required(VERSION 2.8.11)
set (CMAKE_PREFIX_PATH "/home/michal/Qt/5.6/gcc_64/lib/cmake")
project(websockets)
aux_source_directory(. SRC_LIST)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(QT_MIN_VERSION "5.6.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Qml)

# Tell CMake to create the helloworld executable
add_library(websockets SHARED ${SRC_LIST})

# Use the Widgets module from Qt 5.
target_link_libraries(websockets Qt5::Core Qt5::Qml)
