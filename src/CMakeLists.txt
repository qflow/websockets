cmake_minimum_required(VERSION 2.8.11)
project(websockets)
add_definitions(-DWEBSOCKETS_LIBRARY)
file(GLOB_RECURSE DATABASE_SRCS
         RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set(QT_MIN_VERSION "5.6.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Qml Network)
find_package(websocketpp REQUIRED)
add_definitions(${WEBSOCKETPP_DEFINITIONS})
add_library(websockets SHARED ${DATABASE_SRCS} qmldir)
set_property(TARGET websockets PROPERTY CXX_STANDARD 11)
add_dependencies(websockets core)
target_include_directories (websockets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${WEBSOCKETPP_INCLUDE_DIRS})
target_link_libraries(websockets core Qt5::Core Qt5::Qml Qt5::Network ${WEBSOCKETPP_LIBRARIES})
set(WEBSOCKETS_INSTALL_PATH "bin/plugins/QFlow/Websockets")
install (TARGETS websockets DESTINATION ${WEBSOCKETS_INSTALL_PATH})
install (FILES qmldir DESTINATION ${WEBSOCKETS_INSTALL_PATH})
