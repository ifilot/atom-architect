####################################################################################################
 #
 #
 #   Atom Architect
 #   Copyright (C) 2020-2024 Ivo Filot <i.a.w.filot@tue.nl>
 #
####################################################################################################

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

HEADERS       = src/gui/anaglyph_widget.h \
                src/gui/interface_window.h \
                src/gui/mainwindow.h \
                src/gui/shader_program_types.h \
                src/gui/shader_program.h \
                src/gui/shader_program_manager.h \
                src/gui/structure_info_widget.h \
                src/gui/structure_info_tab.h \
                src/gui/structure_info_basic_tab.h \
                src/gui/structure_renderer.h \
                src/gui/fragment_selector.h \
                src/gui/toolbar.h \
                src/gui/user_action.h \
                src/gui/periodic_table.h \
                src/gui/analysis_geometry_optimization.h \
                src/gui/analysis_neb.h \
                src/gui/scene.h \
                src/data/atom.h \
                src/data/bond.h \
                src/data/atom_settings.h \
                src/data/fragment.h \
                src/data/model.h \
                src/data/model_loader.h \
                src/data/structure.h \
                src/data/structure_loader.h \
                src/data/structure_saver.h \
                src/data/structure_operator.h \
                src/data/matrixmath.h \
                src/config.h

SOURCES       = src/main.cpp \
                src/gui/anaglyph_widget.cpp \
                src/gui/interface_window.cpp \
                src/gui/mainwindow.cpp \
                src/gui/periodic_table.cpp \
                src/gui/shader_program.cpp \
                src/gui/shader_program_manager.cpp \
                src/gui/structure_renderer.cpp \
                src/gui/structure_info_widget.cpp \
                src/gui/structure_info_basic_tab.cpp \
                src/gui/fragment_selector.cpp \
                src/gui/toolbar.cpp \
                src/gui/user_action.cpp \
                src/gui/scene.cpp \
                src/gui/analysis_geometry_optimization.cpp \
                src/gui/analysis_neb.cpp \
                src/data/atom_settings.cpp \
                src/data/atom.cpp \
                src/data/bond.cpp \
                src/data/fragment.cpp \
                src/data/model.cpp \
                src/data/model_loader.cpp \
                src/data/structure.cpp \
                src/data/structure_loader.cpp \
                src/data/structure_saver.cpp \
                src/data/structure_operator.cpp

QT           += core gui widgets charts
CONFIG       += force_debug_info

win32 {
    DEFINES += _USE_MATH_DEFINES
    INCLUDEPATH += src/vendor/eigen-3.4.0
    INCLUDEPATH += src/vendor/glm-1.0.1
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS += -fopenmp
}

unix {
    DEFINES += _USE_MATH_DEFINES
    INCLUDEPATH += src/vendor/eigen-3.4.0
    INCLUDEPATH += src/vendor/glm-1.0.1
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS += -fopenmp
}

RESOURCES += resources.qrc
