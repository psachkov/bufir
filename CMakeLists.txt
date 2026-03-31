cmake_minimum_required(VERSION 3.20)
project(ClipboardManager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql DBus)

# Try KDE Frameworks 6 first, fallback to KF5
find_package(KF6GlobalAccel QUIET)
if (NOT KF6GlobalAccel_FOUND)
    find_package(KF5GlobalAccel QUIET)
    if (NOT KF5GlobalAccel_FOUND)
        message(WARNING "Neither KF6GlobalAccel nor KF5GlobalAccel found. Global hotkey support will be disabled.")
        set(KGlobalAccel_TARGET "")
    else()
        set(KGlobalAccel_TARGET KF5::GlobalAccel)
        message(STATUS "Found KF5GlobalAccel")
    endif()
else()
    set(KGlobalAccel_TARGET KF6::GlobalAccel)
    message(STATUS "Found KF6GlobalAccel")
endif()

# KWindowSystem for paste functionality
find_package(KF6WindowSystem QUIET)
if (NOT KF6WindowSystem_FOUND)
    find_package(KF5WindowSystem QUIET)
    if (NOT KF5WindowSystem_FOUND)
        message(WARNING "Neither KF6WindowSystem nor KF5WindowSystem found. Paste functionality may be limited.")
        set(KWindowSystem_TARGET "")
    else()
        set(KWindowSystem_TARGET KF5::WindowSystem)
        message(STATUS "Found KF5WindowSystem")
    endif()
else()
    set(KWindowSystem_TARGET KF6::WindowSystem)
    message(STATUS "Found KF6WindowSystem")
endif()

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
    ${KGlobalAccel_TARGET}
    ${KWindowSystem_TARGET}
)

# Installation
install(TARGETS ClipboardManager DESTINATION bin)
