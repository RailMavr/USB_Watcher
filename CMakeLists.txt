cmake_minimum_required(VERSION 3.14)

project(untitled LANGUAGES CXX)

find_library(LIBUSB NAMES libusb libusb-1.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(untitled
  main.cpp
)
target_include_directories(untitled PRIVATE ${CMAKE_SOURCE_DIR}/libusb)
target_link_libraries(untitled Qt${QT_VERSION_MAJOR}::Core ${LIBUSB})
