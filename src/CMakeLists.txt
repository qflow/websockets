cmake_minimum_required(VERSION 2.8.11)
include(ExternalProject)
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

find_package(websocketpp QUIET)
if(websocketpp_FOUND)
else()
    ExternalProject_Add(
      websocketpp
      GIT_REPOSITORY "https://github.com/zaphoyd/websocketpp.git"
      GIT_TAG "master"
      PREFIX websocketpp
      UPDATE_COMMAND ""
      PATCH_COMMAND ""
      CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=install
      TEST_COMMAND ""
    )
    set(WEBSOCKETPP_INCLUDE_DIRS ${PROJECT_BINARY_DIR}/websocketpp/src/websocketpp-build/install/include)
    message(STATUS "Websocketpp: " ${WEBSOCKETPP_INCLUDE_DIRS})
    if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        set(WEBSOCKETPP_LIBRARIES ${WEBSOCKETPP_LIBRARIES} ws2_32)
    endif()
endif()

set(QT_MIN_VERSION "5.6.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Qml Network)
add_definitions(${WEBSOCKETPP_DEFINITIONS})
add_library(websockets SHARED ${DATABASE_SRCS} qmldir)
set_property(TARGET websockets PROPERTY CXX_STANDARD 14)
add_dependencies(websockets websocketpp core)
target_include_directories (websockets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${WEBSOCKETPP_INCLUDE_DIRS})
target_link_libraries(websockets core Qt5::Core Qt5::Qml Qt5::Network ${WEBSOCKETPP_LIBRARIES})
set(WEBSOCKETS_INSTALL_PATH "plugins/QFlow/Websockets")
install (TARGETS websockets DESTINATION ${WEBSOCKETS_INSTALL_PATH} COMPONENT qflow)
install (FILES qmldir DESTINATION ${WEBSOCKETS_INSTALL_PATH} COMPONENT qflow)
