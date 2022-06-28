cmake_minimum_required(VERSION 3.5)

project(modtool VERSION 0.2 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

find_package(QT NAMES Qt5 Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        startpage.cpp
        startpage.h
        startpage.ui
        backend.h
        backend.cpp
        downloadpage.h
        downloadpage.cpp
        downloadpage.ui
        completewidget.h
        completewidget.cpp
        completewidget.ui
        resources.qrc
)

set(APP_ICON_RESOURCE_WIN32 "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")
set(MACOSX_BUNDLE_ICON_FILE logo.icns)
set(APP_ICON_MACOSX "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.icns")
set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(modtool
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${APP_ICON_RESOURCE_WIN32}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET modtool APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(modtool SHARED
            ${PROJECT_SOURCES}
            ${APP_ICON_RESOURCE_WIN32}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(modtool
            ${PROJECT_SOURCES}
            ${APP_ICON_RESOURCE_WIN32}
        )
    endif()
endif()

target_link_libraries(modtool PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(modtool PRIVATE Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(modtool PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER dev.tallie.modtool
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

install(TARGETS modtool
    RUNTIME DESTINATION bin
)

if(LINUX)
    message(STATUS "Running on Linux, preparing for XDG Desktop install")
    configure_file(${CMAKE_SOURCE_DIR}/assets/logo.png ${CMAKE_CURRENT_BINARY_DIR}/assets/modtool.png)
    configure_file(${CMAKE_SOURCE_DIR}/assets/modtool.desktop ${CMAKE_CURRENT_BINARY_DIR}/assets/modtool.desktop)

    install(SCRIPT "${CMAKE_SOURCE_DIR}/assets/install_desktop.cmake")
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(modtool)
endif()
