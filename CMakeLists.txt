cmake_minimum_required(VERSION 3.30)
project(final_project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(catalog_core STATIC
    catalog.cpp
)
target_include_directories(catalog_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(advisor_cli
    main_cli.cpp
)
target_link_libraries(advisor_cli PRIVATE catalog_core)

add_executable(final_project ALIAS advisor_cli)

add_executable(advisor_gui
    main_gui.cpp
    mainwindow.cpp
    models.cpp
)
target_link_libraries(advisor_gui PRIVATE catalog_core Qt6::Widgets)
