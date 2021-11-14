# *****************************************************************************
# *
# *  PROJECT:           SampX
# *  LICENSE:           See LICENSE in the top level directory
# *  FILE:              CMakeLists.txt
# *  DESCRIPTION:       CMake
# *  COPYRIGHT:         (c) 2021 RINWARES <rinwares.com>
# *  AUTHOR:            Rinat Namazov <rinat.namazov@rinwares.com>
# *
# *****************************************************************************

cmake_minimum_required(VERSION 3.15)

project(SampX)

option(UPDATE_TRANSLATION_SOURCES "Updates *.ts files" ON)

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets Network LinguistTools REQUIRED)
find_package(pe-parse CONFIG REQUIRED)

file(GLOB SAMPX_SRCS
    ${CMAKE_SOURCE_DIR}/src/AdaptersDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/GameLauncher.cpp
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/MainWindow.cpp
    ${CMAKE_SOURCE_DIR}/src/ProxysDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/SampFavorites.cpp
    ${CMAKE_SOURCE_DIR}/src/SampQuery.cpp
    ${CMAKE_SOURCE_DIR}/src/SampVersions.cpp
    ${CMAKE_SOURCE_DIR}/src/SampVersionsDialog.cpp
    ${CMAKE_SOURCE_DIR}/src/SettingsData.cpp
    ${CMAKE_SOURCE_DIR}/src/utils.cpp
)

file(GLOB SAMPX_HDRS
    ${CMAKE_SOURCE_DIR}/src/AdaptersDialog.hpp
    ${CMAKE_SOURCE_DIR}/src/GameLauncher.hpp
    ${CMAKE_SOURCE_DIR}/src/MainWindow.hpp
    ${CMAKE_SOURCE_DIR}/src/ProxysDialog.hpp
    ${CMAKE_SOURCE_DIR}/src/SampFavorites.hpp
    ${CMAKE_SOURCE_DIR}/src/SampQuery.hpp
    ${CMAKE_SOURCE_DIR}/src/SampVersions.hpp
    ${CMAKE_SOURCE_DIR}/src/SampVersionsDialog.hpp
    ${CMAKE_SOURCE_DIR}/src/SettingsData.hpp
    ${CMAKE_SOURCE_DIR}/src/utils.hpp
)

file(GLOB SAMPX_UI
    ${CMAKE_SOURCE_DIR}/src/AdaptersDialog.ui
    ${CMAKE_SOURCE_DIR}/src/MainWindow.ui
    ${CMAKE_SOURCE_DIR}/src/ProxysDialog.ui
    ${CMAKE_SOURCE_DIR}/src/SampVersionsDialog.ui
)

file(GLOB SAMPX_TS
    ${CMAKE_SOURCE_DIR}/translations/en_US.ts
    ${CMAKE_SOURCE_DIR}/translations/ru_RU.ts
)

if (UPDATE_TRANSLATION_SOURCES)
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR} ${SAMPX_TS})
else()
    qt5_add_translation(QM_FILES ${TRANSLATION_FILES})
endif()

qt5_wrap_ui(UI_HEADERS ${SAMPX_UI})

if (WIN32)
    set(APP_ICON_RESOURCE_WINDOWS ${CMAKE_SOURCE_DIR}/resources/resources.rc)
    set(WIN32DEF "WIN32") # To prevent the console from appearing.
endif()

add_executable(SampX ${WIN32DEF}
    ${SAMPX_SRCS}
    ${SAMPX_HDRS}
    ${SAMPX_UI}
    ${SAMPX_TS}
    ${UI_HEADERS}
    ${QM_FILES}
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
    ${APP_ICON_RESOURCE_WINDOWS}
)

target_compile_features(SampX PRIVATE cxx_std_17)

target_include_directories(SampX PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(SampX PRIVATE
    Qt5::Widgets
    Qt5::Network
    pe-parse::pe-parser-library
)

install(TARGETS SampX DESTINATION .)
install(FILES ${QM_FILES} DESTINATION languages)

# Use windeployqt/linuxdeployqt/macdeployqt.
