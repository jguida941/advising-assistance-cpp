cmake_minimum_required(VERSION 3.30)
project(final_project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(catalog_core STATIC
    src/catalog/catalog.cpp
)
target_include_directories(catalog_core PUBLIC ${PROJECT_INCLUDE_DIR})

add_executable(advisor_cli
    src/cli/main_cli.cpp
)
target_include_directories(advisor_cli PRIVATE ${PROJECT_INCLUDE_DIR})
target_link_libraries(advisor_cli PRIVATE catalog_core)

add_executable(final_project ALIAS advisor_cli)

add_executable(advisor_gui
    src/gui/main_gui.cpp
    src/gui/mainwindow.cpp
    src/gui/models.cpp
    include/gui/mainwindow.hpp
    include/gui/models.hpp
)
target_include_directories(advisor_gui PRIVATE ${PROJECT_INCLUDE_DIR})
target_link_libraries(advisor_gui PRIVATE catalog_core Qt6::Widgets)
