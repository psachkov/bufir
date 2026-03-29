cmake_minimum_required(VERSION 3.20)
project(ClipboardManager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql DBus)

set(SOURCES
    src/main.cpp
    src/clipboarditem.cpp
    src/clipboardmodel.cpp
    src/clipboardmonitor.cpp
    src/database.cpp
    src/mainwindow.cpp
    src/systemtray.cpp
    src/globalhotkey.cpp
)

set(HEADERS
    src/clipboarditem.h
    src/clipboardmodel.h
    src/clipboardmonitor.h
    src/database.h
    src/mainwindow.h
    src/systemtray.h
    src/globalhotkey.h
)

qt_add_executable(ClipboardManager
    ${SOURCES}
    ${HEADERS}
    resources/resources.qrc
)

target_link_libraries(ClipboardManager PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
    Qt6::DBus
)

# Installation
install(TARGETS ClipboardManager DESTINATION bin)
