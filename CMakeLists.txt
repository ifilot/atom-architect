cmake_minimum_required(VERSION 3.16)

# Set the project name
project(atom-architect LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# get Git HASH
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_compile_definitions(GIT_HASH="${GIT_HASH}")

find_package(Qt5 REQUIRED COMPONENTS Widgets Charts)
find_package(Eigen3 REQUIRED)
find_package(glm REQUIRED)

add_executable(atom-architect WIN32
    src/main.cpp
    src/gui/anaglyph_widget.cpp
    src/gui/interface_window.cpp
    src/gui/mainwindow.cpp
    src/gui/periodic_table.cpp
    src/gui/shader_program.cpp
    src/gui/shader_program_manager.cpp
    src/gui/structure_renderer.cpp
    src/gui/structure_info_widget.cpp
    src/gui/structure_info_basic_tab.cpp
    src/gui/fragment_selector.cpp
    src/gui/toolbar.cpp
    src/gui/user_action.cpp
    src/gui/scene.cpp
    src/gui/analysis_geometry_optimization.cpp
    src/gui/geometry_optimization_viewer.cpp
    src/gui/geometry_optimization_graph.cpp
    src/gui/analysis_neb.cpp
    src/gui/logwindow.cpp
    src/data/atom_settings.cpp
    src/data/atom.cpp
    src/data/bond.cpp
    src/data/fragment.cpp
    src/data/model.cpp
    src/data/model_loader.cpp
    src/data/structure.cpp
    src/data/structure_loader.cpp
    src/data/structure_saver.cpp
    src/data/structure_operator.cpp
    src/atomarchitectapplication.cpp
    resources.qrc
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
    set_target_properties(atom-architect PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()

target_link_libraries(atom-architect PRIVATE 
    Qt5::Core 
    Qt5::Widgets 
    Qt5::Charts
    Eigen3::Eigen
    glm::glm
)

if (MINGW)
    target_link_libraries(atom-architect PRIVATE ws2_32 bcrypt)
endif()

if (WIN32)
    target_sources(atom-architect PRIVATE atom-architect.rc)
endif()