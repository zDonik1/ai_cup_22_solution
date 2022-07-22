cmake_minimum_required(VERSION 3.14)

set(DEPLOYMENT_DIR "$ENV{HOME}/ai_cup/trainer")
set(TARGET ai_cup_trainer)

project(${TARGET} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(${TARGET} main.cpp)
target_link_libraries(${TARGET} Qt${QT_VERSION_MAJOR}::Core)

install(
    PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${TARGET} bootstrap_server.sh bootstrap_client.sh
    DESTINATION ${DEPLOYMENT_DIR}
)
