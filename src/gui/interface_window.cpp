/****************************************************************************
 *                                                                          *
 *   ATOM ARCHITECT                                                         *
 *   Copyright (C) 2020-2026 Ivo Filot <i.a.w.filot@tue.nl>                 *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU Lesser General Public License as         *
 *   published by the Free Software Foundation, either version 3 of the     *
 *   License, or (at your option) any later version.                        *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public license      *
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>. *
 *                                                                          *
 ****************************************************************************/

#include "interface_window.h"

#include <QCursor>
#include <QDir>
#include <QFileInfo>
#include <QSettings>

/**
 * @brief      Constructs the object.
 *
 * @param      mw    pointer to MainWindow object
 */
InterfaceWindow::InterfaceWindow(MainWindow *mw)
    : mainWindow(mw)
{
    // ============================================================
    // Root layout
    // ============================================================
    QHBoxLayout *rootLayout = new QHBoxLayout(this);
    setLayout(rootLayout);

    // ============================================================
    // Main horizontal splitter
    // ============================================================
    QSplitter *hSplitter = new QSplitter(Qt::Horizontal, this);
    rootLayout->addWidget(hSplitter);

    // ============================================================
    // LEFT COLUMN (Editor + Geometry Optimization Viewer)
    // ============================================================
    QSplitter *leftSplitter = new QSplitter(Qt::Vertical, this);
    hSplitter->addWidget(leftSplitter);

    // ------------------------------------------------------------
    // Editor panel (TOP-LEFT)
    // ------------------------------------------------------------
    QWidget *editorPanel = new QWidget(this);
    this->editor_panel_ = editorPanel;
    QVBoxLayout *editorLayout = new QVBoxLayout(editorPanel);

    QMenuBar *editorMenuBar = new QMenuBar(editorPanel);
    QMenu *editorMenuFile = editorMenuBar->addMenu(tr("&Load"));
    QMenu *editorMenuView = editorMenuBar->addMenu(tr("&View"));
    QMenu *editorMenuSelect = editorMenuBar->addMenu(tr("&Select"));
    editorMenuBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    editorMenuBar->setStyleSheet(
        "QMenuBar::item { border: 1px solid #7a7a7a; border-radius: 4px; margin: 2px; padding: 3px 10px; background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fbfbfb, stop:1 #d5d5d5); }"
        "QMenuBar::item:selected { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #c7ddff); }"
        "QMenu::item { border: 1px solid transparent; border-radius: 3px; padding: 4px 20px 4px 20px; margin: 1px; }"
        "QMenu::item:selected { border: 1px solid #6f95cc; background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #cfe1ff); }"
    );

    QLabel *editorLabel = new QLabel("EDITOR", editorPanel);
    editorLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    QWidget *editorHeader = new QWidget(editorPanel);
    QHBoxLayout *editorHeaderLayout = new QHBoxLayout(editorHeader);
    editorHeaderLayout->setContentsMargins(0, 0, 0, 0);
    editorHeaderLayout->setSpacing(6);
    editorHeaderLayout->addWidget(editorMenuBar, 0, Qt::AlignLeft);
    editorHeaderLayout->addWidget(editorLabel);
    editorHeaderLayout->addStretch();
    editorLayout->addWidget(editorHeader);

    QAction *editorActionSelectAll = editorMenuSelect->addAction(tr("Select all atoms"));
    editorActionSelectAll->setShortcut(Qt::CTRL | Qt::Key_A);
    QAction *editorActionDeselectAll = editorMenuSelect->addAction(tr("Deselect all atoms"));
    editorActionDeselectAll->setShortcut(Qt::CTRL | Qt::Key_D);
    QAction *editorActionInvertSelection = editorMenuSelect->addAction(tr("Invert selection"));
    editorActionInvertSelection->setShortcut(Qt::CTRL | Qt::Key_I);
    editorMenuSelect->addSeparator();
    QAction *editorActionSetFrozen = editorMenuSelect->addAction(tr("Set frozen"));
    editorActionSetFrozen->setShortcut(Qt::CTRL | Qt::Key_F);
    QAction *editorActionSetUnfrozen = editorMenuSelect->addAction(tr("Set unfrozen"));
    editorActionSetUnfrozen->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_F);

    QAction *editorActionOpen = editorMenuFile->addAction(tr("Open"));
    editorActionOpen->setShortcuts(QKeySequence::Open);
    QAction *editorActionSave = editorMenuFile->addAction(tr("Save"));
    editorActionSave->setShortcuts(QKeySequence::Save);

    const auto scopedShortcutContext = Qt::WindowShortcut;

    QMenu *editorMenuCamera = new QMenu(tr("Camera"), editorMenuView);
    QMenu *editorMenuCameraAlign = new QMenu(tr("Align"), editorMenuCamera);
    QAction *editorActionCameraDefault = new QAction(editorMenuCameraAlign);
    QAction *editorActionCameraTop = new QAction(editorMenuCameraAlign);
    QAction *editorActionCameraBottom = new QAction(editorMenuCameraAlign);
    QAction *editorActionCameraLeft = new QAction(editorMenuCameraAlign);
    QAction *editorActionCameraRight = new QAction(editorMenuCameraAlign);
    QAction *editorActionCameraFront = new QAction(editorMenuCameraAlign);
    QAction *editorActionCameraBack = new QAction(editorMenuCameraAlign);

    QMenu *editorMenuCameraMode = new QMenu(tr("Mode"), editorMenuCamera);
    QAction *editorActionCameraPerspective = new QAction(editorMenuCameraMode);
    QAction *editorActionCameraOrthographic = new QAction(editorMenuCameraMode);
    QAction *editorActionResetView = new QAction(editorMenuView);

    QMenu *editorMenuProjection = new QMenu(tr("Projection"), editorMenuView);
    QAction *editorActionProjectionTwoDimensional = new QAction(editorMenuProjection);
    QAction *editorActionProjectionAnaglyphRedCyan = new QAction(editorMenuProjection);
    QMenu *editorMenuProjectionInterlaced = new QMenu(tr("Interlaced"), editorMenuProjection);
    QAction *editorActionProjectionInterlacedRowsLr = new QAction(editorMenuProjectionInterlaced);
    QAction *editorActionProjectionInterlacedRowsRl = new QAction(editorMenuProjectionInterlaced);
    QAction *editorActionProjectionInterlacedColumnsLr = new QAction(editorMenuProjectionInterlaced);
    QAction *editorActionProjectionInterlacedColumnsRl = new QAction(editorMenuProjectionInterlaced);
    QAction *editorActionProjectionInterlacedCheckerboardLr = new QAction(editorMenuProjectionInterlaced);
    QAction *editorActionProjectionInterlacedCheckerboardRl = new QAction(editorMenuProjectionInterlaced);

    editorActionCameraDefault->setText(tr("Default"));
    editorActionCameraDefault->setData(QVariant((int)CameraAlignment::DEFAULT));
    editorActionCameraDefault->setShortcut(Qt::Key_0);
    editorActionCameraTop->setText(tr("Top"));
    editorActionCameraTop->setData(QVariant((int)CameraAlignment::TOP));
    editorActionCameraTop->setShortcut(Qt::Key_7);
    editorActionCameraBottom->setText(tr("Bottom"));
    editorActionCameraBottom->setData(QVariant((int)CameraAlignment::BOTTOM));
    editorActionCameraBottom->setShortcut(Qt::CTRL | Qt::Key_7);
    editorActionCameraLeft->setText(tr("Left"));
    editorActionCameraLeft->setData(QVariant((int)CameraAlignment::LEFT));
    editorActionCameraLeft->setShortcut(Qt::Key_3);
    editorActionCameraRight->setText(tr("Right"));
    editorActionCameraRight->setData(QVariant((int)CameraAlignment::RIGHT));
    editorActionCameraRight->setShortcut(Qt::CTRL | Qt::Key_3);
    editorActionCameraFront->setText(tr("Front"));
    editorActionCameraFront->setData(QVariant((int)CameraAlignment::FRONT));
    editorActionCameraFront->setShortcut(Qt::Key_1);
    editorActionCameraBack->setText(tr("Back"));
    editorActionCameraBack->setData(QVariant((int)CameraAlignment::BACK));
    editorActionCameraBack->setShortcut(Qt::CTRL | Qt::Key_1);
    editorActionCameraPerspective->setText(tr("Perspective"));
    editorActionCameraPerspective->setData(QVariant((int)CameraMode::PERSPECTIVE));
    editorActionCameraPerspective->setShortcut(Qt::Key_5);
    editorActionCameraOrthographic->setText(tr("Orthographic"));
    editorActionCameraOrthographic->setData(QVariant((int)CameraMode::ORTHOGRAPHIC));
    editorActionCameraOrthographic->setShortcut(Qt::CTRL | Qt::Key_5);
    editorActionResetView->setText(tr("Reset view"));
    editorActionResetView->setShortcut(Qt::CTRL | Qt::Key_0);

    editorActionProjectionTwoDimensional->setText(tr("Two-dimensional"));
    editorActionProjectionAnaglyphRedCyan->setText(tr("Anaglyph (red/cyan)"));
    editorActionProjectionInterlacedRowsLr->setText(tr("Interlaced rows (left first)"));
    editorActionProjectionInterlacedRowsRl->setText(tr("Interlaced rows (right first)"));
    editorActionProjectionInterlacedColumnsLr->setText(tr("Interlaced columns (left first)"));
    editorActionProjectionInterlacedColumnsRl->setText(tr("Interlaced columns (right first)"));
    editorActionProjectionInterlacedCheckerboardLr->setText(tr("Checkerboard (left first)"));
    editorActionProjectionInterlacedCheckerboardRl->setText(tr("Checkerboard (right first)"));
    editorMenuProjectionInterlaced->setIcon(QIcon(":/assets/icon/interlaced_rows_lr_32.png"));
    editorActionProjectionTwoDimensional->setIcon(QIcon(":/assets/icon/two_dimensional_32.png"));
    editorActionProjectionAnaglyphRedCyan->setIcon(QIcon(":/assets/icon/anaglyph_red_cyan_32.png"));
    editorActionProjectionInterlacedRowsLr->setIcon(QIcon(":/assets/icon/interlaced_rows_lr_32.png"));
    editorActionProjectionInterlacedRowsRl->setIcon(QIcon(":/assets/icon/interlaced_rows_rl_32.png"));
    editorActionProjectionInterlacedColumnsLr->setIcon(QIcon(":/assets/icon/interlaced_columns_lr_32.png"));
    editorActionProjectionInterlacedColumnsRl->setIcon(QIcon(":/assets/icon/interlaced_columns_rl_32.png"));
    editorActionProjectionInterlacedCheckerboardLr->setIcon(QIcon(":/assets/icon/interlaced_checkerboard_lr_32.png"));
    editorActionProjectionInterlacedCheckerboardRl->setIcon(QIcon(":/assets/icon/interlaced_checkerboard_rl_32.png"));

    for(QAction *action : {editorActionOpen, editorActionSave,
                           editorActionSelectAll, editorActionDeselectAll, editorActionInvertSelection,
                           editorActionSetFrozen, editorActionSetUnfrozen, editorActionCameraDefault,
                           editorActionCameraTop, editorActionCameraBottom, editorActionCameraLeft,
                           editorActionCameraRight, editorActionCameraFront, editorActionCameraBack,
                           editorActionCameraPerspective, editorActionCameraOrthographic, editorActionResetView,
                           editorActionProjectionTwoDimensional, editorActionProjectionAnaglyphRedCyan,
                           editorActionProjectionInterlacedRowsLr, editorActionProjectionInterlacedRowsRl,
                           editorActionProjectionInterlacedColumnsLr, editorActionProjectionInterlacedColumnsRl,
                           editorActionProjectionInterlacedCheckerboardLr, editorActionProjectionInterlacedCheckerboardRl}) {
        action->setShortcutContext(scopedShortcutContext);
        this->editor_shortcut_actions_.push_back(action);
    }

    editorMenuView->addMenu(editorMenuProjection);
    editorMenuView->addMenu(editorMenuCamera);
    editorMenuView->addSeparator();
    editorMenuView->addAction(editorActionResetView);
    editorMenuCamera->addMenu(editorMenuCameraAlign);
    editorMenuCameraAlign->addAction(editorActionCameraDefault);
    editorMenuCameraAlign->addAction(editorActionCameraTop);
    editorMenuCameraAlign->addAction(editorActionCameraBottom);
    editorMenuCameraAlign->addAction(editorActionCameraLeft);
    editorMenuCameraAlign->addAction(editorActionCameraRight);
    editorMenuCameraAlign->addAction(editorActionCameraFront);
    editorMenuCameraAlign->addAction(editorActionCameraBack);
    editorMenuCamera->addMenu(editorMenuCameraMode);
    editorMenuCameraMode->addAction(editorActionCameraPerspective);
    editorMenuCameraMode->addAction(editorActionCameraOrthographic);
    editorMenuProjection->addAction(editorActionProjectionTwoDimensional);
    editorMenuProjection->addAction(editorActionProjectionAnaglyphRedCyan);
    editorMenuProjection->addMenu(editorMenuProjectionInterlaced);
    editorMenuProjectionInterlaced->addAction(editorActionProjectionInterlacedRowsLr);
    editorMenuProjectionInterlaced->addAction(editorActionProjectionInterlacedRowsRl);
    editorMenuProjectionInterlaced->addAction(editorActionProjectionInterlacedColumnsLr);
    editorMenuProjectionInterlaced->addAction(editorActionProjectionInterlacedColumnsRl);
    editorMenuProjectionInterlaced->addAction(editorActionProjectionInterlacedCheckerboardLr);
    editorMenuProjectionInterlaced->addAction(editorActionProjectionInterlacedCheckerboardRl);

    QWidget *editorViewportRow = new QWidget(editorPanel);
    QHBoxLayout *editorViewportLayout = new QHBoxLayout(editorViewportRow);
    editorViewportLayout->setContentsMargins(0, 0, 0, 0);
    editorViewportLayout->setSpacing(6);

    editor_toolbar = new ToolBarWidget(editorViewportRow, true);
    editor_toolbar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    anaglyph_widget = new AnaglyphWidget(editorViewportRow);
    anaglyph_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    editorViewportLayout->addWidget(editor_toolbar);
    editorViewportLayout->addWidget(anaglyph_widget, 1);
    editorLayout->addWidget(editorViewportRow, 1);

    // Interaction / selection labels
    QWidget *labelWidget = new QWidget(this);
    QHBoxLayout *labelLayout = new QHBoxLayout(labelWidget);

    selection_label = new QLabel("<br>", labelWidget);
    interaction_label = new QLabel("", labelWidget);

    labelLayout->addWidget(selection_label);
    labelLayout->addWidget(interaction_label);
    labelLayout->addStretch();

    editorLayout->addWidget(labelWidget);

    leftSplitter->addWidget(editorPanel);

    // ------------------------------------------------------------
    // Geometry optimization viewer (BOTTOM-LEFT)
    // ------------------------------------------------------------
    structureAnalysis = new StructureAnalysis(this);
    this->analysis_panel_ = structureAnalysis->viewer();

    QMenuBar *analysisMenuBar = new QMenuBar(this);
    analysisMenuBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    analysisMenuBar->setStyleSheet(editorMenuBar->styleSheet());
    QMenu *analysisMenuFile = analysisMenuBar->addMenu(tr("&Load"));
    QMenu *analysisMenuView = analysisMenuBar->addMenu(tr("&View"));

    QMenu *analysisMenuCamera = new QMenu(tr("Camera"), analysisMenuView);
    QMenu *analysisMenuCameraAlign = new QMenu(tr("Align"), analysisMenuCamera);
    QAction *analysisActionCameraDefault = new QAction(analysisMenuCameraAlign);
    QAction *analysisActionCameraTop = new QAction(analysisMenuCameraAlign);
    QAction *analysisActionCameraBottom = new QAction(analysisMenuCameraAlign);
    QAction *analysisActionCameraLeft = new QAction(analysisMenuCameraAlign);
    QAction *analysisActionCameraRight = new QAction(analysisMenuCameraAlign);
    QAction *analysisActionCameraFront = new QAction(analysisMenuCameraAlign);
    QAction *analysisActionCameraBack = new QAction(analysisMenuCameraAlign);

    QMenu *analysisMenuCameraMode = new QMenu(tr("Mode"), analysisMenuCamera);
    QAction *analysisActionCameraPerspective = new QAction(analysisMenuCameraMode);
    QAction *analysisActionCameraOrthographic = new QAction(analysisMenuCameraMode);
    QAction *analysisActionResetView = new QAction(analysisMenuView);

    QMenu *analysisMenuProjection = new QMenu(tr("Projection"), analysisMenuView);
    QAction *analysisActionProjectionTwoDimensional = new QAction(analysisMenuProjection);
    QAction *analysisActionProjectionAnaglyphRedCyan = new QAction(analysisMenuProjection);
    QMenu *analysisMenuProjectionInterlaced = new QMenu(tr("Interlaced"), analysisMenuProjection);
    QAction *analysisActionProjectionInterlacedRowsLr = new QAction(analysisMenuProjectionInterlaced);
    QAction *analysisActionProjectionInterlacedRowsRl = new QAction(analysisMenuProjectionInterlaced);
    QAction *analysisActionProjectionInterlacedColumnsLr = new QAction(analysisMenuProjectionInterlaced);
    QAction *analysisActionProjectionInterlacedColumnsRl = new QAction(analysisMenuProjectionInterlaced);
    QAction *analysisActionProjectionInterlacedCheckerboardLr = new QAction(analysisMenuProjectionInterlaced);
    QAction *analysisActionProjectionInterlacedCheckerboardRl = new QAction(analysisMenuProjectionInterlaced);

    QAction *analysisActionOpen = analysisMenuFile->addAction(tr("Open"));
    analysisActionOpen->setShortcuts(QKeySequence::Open);
    QAction *analysisActionOpenNeb = analysisMenuFile->addAction(tr("Open NEB calculation"));
    QAction *analysisActionSendToEditor = new QAction(tr("Send to editor"), this);

    analysisActionCameraDefault->setText(tr("Default"));
    analysisActionCameraDefault->setData(QVariant((int)CameraAlignment::DEFAULT));
    analysisActionCameraDefault->setShortcut(Qt::Key_0);
    analysisActionCameraTop->setText(tr("Top"));
    analysisActionCameraTop->setData(QVariant((int)CameraAlignment::TOP));
    analysisActionCameraTop->setShortcut(Qt::Key_7);
    analysisActionCameraBottom->setText(tr("Bottom"));
    analysisActionCameraBottom->setData(QVariant((int)CameraAlignment::BOTTOM));
    analysisActionCameraBottom->setShortcut(Qt::CTRL | Qt::Key_7);
    analysisActionCameraLeft->setText(tr("Left"));
    analysisActionCameraLeft->setData(QVariant((int)CameraAlignment::LEFT));
    analysisActionCameraLeft->setShortcut(Qt::Key_3);
    analysisActionCameraRight->setText(tr("Right"));
    analysisActionCameraRight->setData(QVariant((int)CameraAlignment::RIGHT));
    analysisActionCameraRight->setShortcut(Qt::CTRL | Qt::Key_3);
    analysisActionCameraFront->setText(tr("Front"));
    analysisActionCameraFront->setData(QVariant((int)CameraAlignment::FRONT));
    analysisActionCameraFront->setShortcut(Qt::Key_1);
    analysisActionCameraBack->setText(tr("Back"));
    analysisActionCameraBack->setData(QVariant((int)CameraAlignment::BACK));
    analysisActionCameraBack->setShortcut(Qt::CTRL | Qt::Key_1);
    analysisActionCameraPerspective->setText(tr("Perspective"));
    analysisActionCameraPerspective->setData(QVariant((int)CameraMode::PERSPECTIVE));
    analysisActionCameraPerspective->setShortcut(Qt::Key_5);
    analysisActionCameraOrthographic->setText(tr("Orthographic"));
    analysisActionCameraOrthographic->setData(QVariant((int)CameraMode::ORTHOGRAPHIC));
    analysisActionCameraOrthographic->setShortcut(Qt::CTRL | Qt::Key_5);
    analysisActionResetView->setText(tr("Reset view"));
    analysisActionResetView->setShortcut(Qt::CTRL | Qt::Key_0);

    analysisActionProjectionTwoDimensional->setText(tr("Two-dimensional"));
    analysisActionProjectionAnaglyphRedCyan->setText(tr("Anaglyph (red/cyan)"));
    analysisActionProjectionInterlacedRowsLr->setText(tr("Interlaced rows (left first)"));
    analysisActionProjectionInterlacedRowsRl->setText(tr("Interlaced rows (right first)"));
    analysisActionProjectionInterlacedColumnsLr->setText(tr("Interlaced columns (left first)"));
    analysisActionProjectionInterlacedColumnsRl->setText(tr("Interlaced columns (right first)"));
    analysisActionProjectionInterlacedCheckerboardLr->setText(tr("Checkerboard (left first)"));
    analysisActionProjectionInterlacedCheckerboardRl->setText(tr("Checkerboard (right first)"));
    analysisMenuProjectionInterlaced->setIcon(QIcon(":/assets/icon/interlaced_rows_lr_32.png"));
    analysisActionProjectionTwoDimensional->setIcon(QIcon(":/assets/icon/two_dimensional_32.png"));
    analysisActionProjectionAnaglyphRedCyan->setIcon(QIcon(":/assets/icon/anaglyph_red_cyan_32.png"));
    analysisActionProjectionInterlacedRowsLr->setIcon(QIcon(":/assets/icon/interlaced_rows_lr_32.png"));
    analysisActionProjectionInterlacedRowsRl->setIcon(QIcon(":/assets/icon/interlaced_rows_rl_32.png"));
    analysisActionProjectionInterlacedColumnsLr->setIcon(QIcon(":/assets/icon/interlaced_columns_lr_32.png"));
    analysisActionProjectionInterlacedColumnsRl->setIcon(QIcon(":/assets/icon/interlaced_columns_rl_32.png"));
    analysisActionProjectionInterlacedCheckerboardLr->setIcon(QIcon(":/assets/icon/interlaced_checkerboard_lr_32.png"));
    analysisActionProjectionInterlacedCheckerboardRl->setIcon(QIcon(":/assets/icon/interlaced_checkerboard_rl_32.png"));

    for(QAction *action : {analysisActionOpen, analysisActionOpenNeb, analysisActionSendToEditor,
                           analysisActionCameraDefault, analysisActionCameraTop, analysisActionCameraBottom,
                           analysisActionCameraLeft, analysisActionCameraRight, analysisActionCameraFront,
                           analysisActionCameraBack, analysisActionCameraPerspective, analysisActionCameraOrthographic,
                           analysisActionResetView,
                           analysisActionProjectionTwoDimensional, analysisActionProjectionAnaglyphRedCyan,
                           analysisActionProjectionInterlacedRowsLr, analysisActionProjectionInterlacedRowsRl,
                           analysisActionProjectionInterlacedColumnsLr, analysisActionProjectionInterlacedColumnsRl,
                           analysisActionProjectionInterlacedCheckerboardLr, analysisActionProjectionInterlacedCheckerboardRl}) {
        action->setShortcutContext(scopedShortcutContext);
        this->analysis_shortcut_actions_.push_back(action);
    }

    analysisMenuView->addMenu(analysisMenuProjection);
    analysisMenuView->addMenu(analysisMenuCamera);
    analysisMenuView->addSeparator();
    analysisMenuView->addAction(analysisActionResetView);
    analysisMenuCamera->addMenu(analysisMenuCameraAlign);
    analysisMenuCameraAlign->addAction(analysisActionCameraDefault);
    analysisMenuCameraAlign->addAction(analysisActionCameraTop);
    analysisMenuCameraAlign->addAction(analysisActionCameraBottom);
    analysisMenuCameraAlign->addAction(analysisActionCameraLeft);
    analysisMenuCameraAlign->addAction(analysisActionCameraRight);
    analysisMenuCameraAlign->addAction(analysisActionCameraFront);
    analysisMenuCameraAlign->addAction(analysisActionCameraBack);
    analysisMenuCamera->addMenu(analysisMenuCameraMode);
    analysisMenuCameraMode->addAction(analysisActionCameraPerspective);
    analysisMenuCameraMode->addAction(analysisActionCameraOrthographic);
    analysisMenuProjection->addAction(analysisActionProjectionTwoDimensional);
    analysisMenuProjection->addAction(analysisActionProjectionAnaglyphRedCyan);
    analysisMenuProjection->addMenu(analysisMenuProjectionInterlaced);
    analysisMenuProjectionInterlaced->addAction(analysisActionProjectionInterlacedRowsLr);
    analysisMenuProjectionInterlaced->addAction(analysisActionProjectionInterlacedRowsRl);
    analysisMenuProjectionInterlaced->addAction(analysisActionProjectionInterlacedColumnsLr);
    analysisMenuProjectionInterlaced->addAction(analysisActionProjectionInterlacedColumnsRl);
    analysisMenuProjectionInterlaced->addAction(analysisActionProjectionInterlacedCheckerboardLr);
    analysisMenuProjectionInterlaced->addAction(analysisActionProjectionInterlacedCheckerboardRl);

    QWidget *analysisHeaderWidget = new QWidget(this);
    QHBoxLayout *analysisHeaderLayout = new QHBoxLayout(analysisHeaderWidget);
    analysisHeaderLayout->setContentsMargins(0, 0, 0, 0);
    analysisHeaderLayout->setSpacing(6);
    QPushButton *analysisSendToEditorButton = new QPushButton(tr("Send to editor"), analysisHeaderWidget);
    analysisSendToEditorButton->setStyleSheet(
        "QPushButton { border: 1px solid #7a7a7a; border-radius: 4px; margin: 2px; padding: 3px 10px;"
        " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #fbfbfb, stop:1 #d5d5d5); }"
        "QPushButton:hover { border: 1px solid #6f95cc;"
        " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #c7ddff); }"
        "QPushButton:pressed { border: 1px solid #6f95cc;"
        " background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #cfe1ff, stop:1 #ffffff); }"
    );
    analysisHeaderLayout->addWidget(analysisMenuBar, 0, Qt::AlignLeft);
    analysisHeaderLayout->addWidget(analysisSendToEditorButton, 0, Qt::AlignLeft);
    analysisHeaderLayout->addStretch();

    structureAnalysis->viewer()->set_header_widget(analysisHeaderWidget);

    analysis_toolbar = new ToolBarWidget(structureAnalysis->viewer(), false);
    analysis_toolbar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    structureAnalysis->viewer()->set_side_toolbar(analysis_toolbar);

    leftSplitter->addWidget(structureAnalysis->viewer());

    // ============================================================
    // RIGHT COLUMN (Structure info + Optimization graph)
    // ============================================================
    QSplitter *rightSplitter = new QSplitter(Qt::Vertical, this);
    hSplitter->addWidget(rightSplitter);

    // ------------------------------------------------------------
    // Structure info (TOP-RIGHT)
    // ------------------------------------------------------------
    structure_info_widget = new StructureInfoWidget(this);
    structure_info_widget->set_anaglyph_widget(anaglyph_widget);
    structure_info_widget->setSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Expanding);

    rightSplitter->addWidget(structure_info_widget);

    // ------------------------------------------------------------
    // Optimization graph (BOTTOM-RIGHT)
    // ------------------------------------------------------------
    rightSplitter->addWidget(structureAnalysis->graph());

    // ============================================================
    // Initial 50/50/50/50 layout (relative to MainWindow size)
    // ============================================================

    const int totalWidth  = width();
    const int totalHeight = height();

    // Left / Right
    hSplitter->setSizes({ totalWidth / 2, totalWidth / 2 });

    // Top / Bottom (both columns)
    leftSplitter->setSizes({ totalHeight / 2, totalHeight / 2 });
    rightSplitter->setSizes({ totalHeight / 2, totalHeight / 2 });

    // ============================================================
    // Connections (unchanged behavior)
    // ============================================================
    connect(this, SIGNAL(new_file_loaded()),
            structure_info_widget, SLOT(reset()));

    connect(anaglyph_widget, SIGNAL(opengl_ready()),
            this, SLOT(load_default_file()));

    connect(editor_toolbar->get_action("toggle_periodicity_xy"),
            SIGNAL(triggered()),
            anaglyph_widget, SLOT(toggle_periodicity_xy()));

    connect(editor_toolbar->get_action("toggle_periodicity_z"),
            SIGNAL(triggered()),
            anaglyph_widget, SLOT(toggle_periodicity_z()));

    connect(editor_toolbar->get_action("add_fragment"),
            SIGNAL(triggered()),
            this, SLOT(add_fragment()));

    connect(analysis_toolbar->get_action("toggle_periodicity_xy"),
            SIGNAL(triggered()),
            structureAnalysis->viewer()->get_anaglyph_widget(),
            SLOT(toggle_periodicity_xy()));

    connect(analysis_toolbar->get_action("toggle_periodicity_z"),
            SIGNAL(triggered()),
            structureAnalysis->viewer()->get_anaglyph_widget(),
            SLOT(toggle_periodicity_z()));

    connect(anaglyph_widget, SIGNAL(signal_interaction_message(const QString&)),
            this, SLOT(update_interaction_label(const QString&)));

    connect(anaglyph_widget, SIGNAL(signal_selection_message(const QString&)),
            this, SLOT(update_selection_label(const QString&)));

    connect(anaglyph_widget->get_user_action().get(),
            SIGNAL(signal_selection_message(const QString&)),
            this, SLOT(update_selection_label(const QString&)));

    connect(anaglyph_widget->get_user_action().get(),
            SIGNAL(signal_message_statusbar(const QString&)),
            this, SLOT(propagate_message_statusbar(const QString&)));

    connect(structure_info_widget->get_fragment_selector(),
            SIGNAL(signal_new_fragment(const Fragment&)),
            anaglyph_widget->get_user_action().get(),
            SLOT(set_fragment(const Fragment&)));

    connect(structureAnalysis->viewer(), SIGNAL(edit_requested()),
            this, SLOT(load_structure_from_geometry_analysis()));

    connect(editorActionOpen, &QAction::triggered, this, &InterfaceWindow::open_editor_file);
    connect(editorActionSave, &QAction::triggered, this, &InterfaceWindow::save_editor_file);
    connect(analysisActionOpen, &QAction::triggered, this, &InterfaceWindow::open_analysis_file);
    connect(analysisActionOpenNeb, &QAction::triggered, this, &InterfaceWindow::open_analysis_neb_calculation);
    connect(analysisActionSendToEditor, &QAction::triggered, this, &InterfaceWindow::load_structure_from_geometry_analysis);
    connect(analysisSendToEditorButton, &QPushButton::clicked, analysisActionSendToEditor, &QAction::trigger);

    connect(editorActionSelectAll, SIGNAL(triggered()), this, SLOT(select_all_atoms()));
    connect(editorActionDeselectAll, SIGNAL(triggered()), this, SLOT(deselect_all_atoms()));
    connect(editorActionInvertSelection, SIGNAL(triggered()), this, SLOT(invert_selection()));
    connect(editorActionSetFrozen, SIGNAL(triggered()), this, SLOT(set_frozen()));
    connect(editorActionSetUnfrozen, SIGNAL(triggered()), this, SLOT(set_unfrozen()));

    connect(editorMenuCameraAlign, SIGNAL(triggered(QAction*)), this, SLOT(set_camera_align(QAction*)));
    connect(editorMenuCameraMode, SIGNAL(triggered(QAction*)), this, SLOT(set_camera_mode(QAction*)));
    connect(editorActionProjectionTwoDimensional, &QAction::triggered, this, [this]{ this->anaglyph_widget->set_stereo("no_stereo_flat"); });
    connect(editorActionProjectionAnaglyphRedCyan, &QAction::triggered, this, [this]{ this->anaglyph_widget->set_stereo("stereo_anaglyph_red_cyan"); });
    connect(editorActionProjectionInterlacedRowsLr, &QAction::triggered, this, [this]{ this->anaglyph_widget->set_stereo("stereo_interlaced_rows_lr"); });
    connect(editorActionProjectionInterlacedRowsRl, &QAction::triggered, this, [this]{ this->anaglyph_widget->set_stereo("stereo_interlaced_rows_rl"); });
    connect(editorActionProjectionInterlacedColumnsLr, &QAction::triggered, this, [this]{ this->anaglyph_widget->set_stereo("stereo_interlaced_columns_lr"); });
    connect(editorActionProjectionInterlacedColumnsRl, &QAction::triggered, this, [this]{ this->anaglyph_widget->set_stereo("stereo_interlaced_columns_rl"); });
    connect(editorActionProjectionInterlacedCheckerboardLr, &QAction::triggered, this, [this]{ this->anaglyph_widget->set_stereo("stereo_interlaced_checkerboard_lr"); });
    connect(editorActionProjectionInterlacedCheckerboardRl, &QAction::triggered, this, [this]{ this->anaglyph_widget->set_stereo("stereo_interlaced_checkerboard_rl"); });
    connect(editorActionResetView, &QAction::triggered, anaglyph_widget, &AnaglyphWidget::reset_view);

    connect(analysisMenuCameraAlign, SIGNAL(triggered(QAction*)), structureAnalysis, SLOT(set_camera_align(QAction*)));
    connect(analysisMenuCameraMode, SIGNAL(triggered(QAction*)), structureAnalysis, SLOT(set_camera_mode(QAction*)));
    connect(analysisActionProjectionTwoDimensional, &QAction::triggered, this, [this]{ this->structureAnalysis->set_stereo("no_stereo_flat"); });
    connect(analysisActionProjectionAnaglyphRedCyan, &QAction::triggered, this, [this]{ this->structureAnalysis->set_stereo("stereo_anaglyph_red_cyan"); });
    connect(analysisActionProjectionInterlacedRowsLr, &QAction::triggered, this, [this]{ this->structureAnalysis->set_stereo("stereo_interlaced_rows_lr"); });
    connect(analysisActionProjectionInterlacedRowsRl, &QAction::triggered, this, [this]{ this->structureAnalysis->set_stereo("stereo_interlaced_rows_rl"); });
    connect(analysisActionProjectionInterlacedColumnsLr, &QAction::triggered, this, [this]{ this->structureAnalysis->set_stereo("stereo_interlaced_columns_lr"); });
    connect(analysisActionProjectionInterlacedColumnsRl, &QAction::triggered, this, [this]{ this->structureAnalysis->set_stereo("stereo_interlaced_columns_rl"); });
    connect(analysisActionProjectionInterlacedCheckerboardLr, &QAction::triggered, this, [this]{ this->structureAnalysis->set_stereo("stereo_interlaced_checkerboard_lr"); });
    connect(analysisActionProjectionInterlacedCheckerboardRl, &QAction::triggered, this, [this]{ this->structureAnalysis->set_stereo("stereo_interlaced_checkerboard_rl"); });
    connect(analysisActionResetView, &QAction::triggered,
            structureAnalysis->viewer()->get_anaglyph_widget(), &AnaglyphWidget::reset_view);

    this->active_panel_timer_ = new QTimer(this);
    this->active_panel_timer_->setInterval(40);
    connect(this->active_panel_timer_, &QTimer::timeout, this, &InterfaceWindow::update_active_panel_from_cursor);
    this->active_panel_timer_->start();

    this->set_active_panel(true);

    // ============================================================
    // Default fragment
    // ============================================================
    anaglyph_widget->get_user_action()->set_fragment(
        structure_info_widget->get_fragment_selector()->get_current_fragment());

    // ============================================================
    // Structure stack
    // ============================================================
    connect(anaglyph_widget->get_user_action().get(),
            SIGNAL(signal_push_structure()),
            this, SLOT(push_structure()));

    connect(anaglyph_widget->get_user_action().get(),
            SIGNAL(signal_increment_structure_stack_pointer()),
            this, SLOT(increment_structure_stack_pointer()));

    connect(anaglyph_widget->get_user_action().get(),
            SIGNAL(signal_decrement_structure_stack_pointer()),
            this, SLOT(decrement_structure_stack_pointer()));
}

/**
 * @brief      Button press event
 *
 * @param      event  The event
 */
void InterfaceWindow::keyPressEvent(QKeyEvent* event) {
    // Only forward when the 3D viewport is active
    if(this->anaglyph_widget->hasFocus()) {
        if(this->anaglyph_widget
               ->get_user_action()
               ->handle_key(event)) {

            event->accept();
            return;
        }
    }

    // Let Qt handle it (menus, shortcuts, text input, etc.)
    QWidget::keyPressEvent(event);
}

/**
 * @brief set_active_panel.
 *
 * @param editor_active Parameter editor_active.
 */
void InterfaceWindow::set_active_panel(bool editor_active)
{
    this->editor_panel_active_ = editor_active;

    if(this->anaglyph_widget != nullptr) {
        this->anaglyph_widget->set_active_highlight(editor_active);
    }

    if(this->structureAnalysis != nullptr && this->structureAnalysis->viewer() != nullptr) {
        auto *analysisAnaglyph = this->structureAnalysis->viewer()->get_anaglyph_widget();
        if(analysisAnaglyph != nullptr) {
            analysisAnaglyph->set_active_highlight(!editor_active);
        }
    }

    for(QAction *action : this->editor_shortcut_actions_) {
        action->setEnabled(editor_active);
    }

    for(QAction *action : this->analysis_shortcut_actions_) {
        action->setEnabled(!editor_active);
    }

}

/**
 * @brief update_active_panel_from_cursor.
 *
 */
void InterfaceWindow::update_active_panel_from_cursor()
{
    const QPoint global = QCursor::pos();

    if(this->editor_panel_ != nullptr) {
        const QPoint local = this->editor_panel_->mapFromGlobal(global);
        if(this->editor_panel_->rect().contains(local)) {
            if(!this->editor_panel_active_) {
                this->set_active_panel(true);
            }
            return;
        }
    }

    if(this->analysis_panel_ != nullptr) {
        const QPoint local = this->analysis_panel_->mapFromGlobal(global);
        if(this->analysis_panel_->rect().contains(local)) {
            if(this->editor_panel_active_) {
                this->set_active_panel(false);
            }
        }
    }
}

    /**
     * @brief      Opens a file.
     *
     * @param[in]  filename  The filename
     *
     * @return     loading time of object in seconds
     */
void InterfaceWindow::open_file(const QString& filename)
{
    qDebug() << "Opening file:" << filename;

    // ------------------------------------------------------------
    // Case 1: Multi-structure trajectory or vibrational file (OUTCAR / YAML)
    // ------------------------------------------------------------
    if (filename.contains("OUTCAR", Qt::CaseInsensitive) ||
        filename.endsWith(".yaml", Qt::CaseInsensitive) ||
        filename.endsWith(".yml", Qt::CaseInsensitive)) {

        StructureLoader sl;
        std::vector<std::shared_ptr<Structure>> structures;

        try {
            if(filename.contains("OUTCAR", Qt::CaseInsensitive)) {
                structures = sl.load_outcar(filename.toStdString());
            } else {
                structures = sl.load_yaml(filename.toStdString());
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this,
                tr("Exception encountered"),
                tr(e.what()));
            return;
        }

        if (structures.empty()) {
            QMessageBox::warning(this,
                tr("Empty optimization"),
                tr("No structures found in selected file."));
            return;
        }

        // ---- Update analysis panels ----
        if (structures.size() == 1 && structures.front()->get_nr_eigenmodes() > 0) {
            structureAnalysis->set_frequency_structure(structures.front()->clone_for_view());
        } else {
            std::vector<std::shared_ptr<Structure>> geometry_structures;
            geometry_structures.reserve(structures.size());

            for (const auto& s : structures) {
                geometry_structures.push_back(s->clone_for_view());
            }

            structureAnalysis->set_structures(geometry_structures);
        }

        // ---- Also sync editor + info to first structure ----
        structure_stack.clear();
        structure_stack.push_back(structures.front());
        structure_stack_pointer = 0;

        emit new_file_loaded();

        anaglyph_widget->set_structure(structures.front());
        structure_info_widget->set_structure(structures.front());

        return;
    }

    // ------------------------------------------------------------
    // Case 2: Single structure file (original behavior)
    // ------------------------------------------------------------
    std::shared_ptr<Structure> structure;

    try {
        structure = structure_loader.load_file(filename.toStdString());
    } catch (const std::exception& e) {
        QMessageBox::critical(this,
            tr("Exception encountered"),
            tr(e.what()));
        return;
    }

    structure_stack.clear();
    structure_stack.push_back(structure);
    structure_stack_pointer = 0;

    emit new_file_loaded();

    anaglyph_widget->set_structure(structure);
    structure_info_widget->set_structure(structure);
}

    /**
     * @brief      Saves a file.
     *
     * @param[in]  filename  The filename
     */
void InterfaceWindow::save_file(const QString& filename) {
    this->structure_saver.save_poscar(this->anaglyph_widget->get_structure(), filename.toStdString());
}

/**
 * @brief open_editor_file.
 *
 */
void InterfaceWindow::open_editor_file()
{
    QSettings settings;
    const QString start_dir = settings.value(
        "ui/lastLoadDir",
        QDir::homePath()
    ).toString();

    QString filename = QFileDialog::getOpenFileName(this, tr("Open file"), start_dir, tr("All supported files (*.geo *.xyz *.yaml *.yml *.vasp OUTCAR* CONTCAR* POSCAR*);;VASP files (*.vasp POSCAR* CONTCAR* OUTCAR*);;YAML frequency files (*.yaml *.yml);;ADF geometry files (*.geo);;XYZ files (*.xyz)"));
    if(filename.isEmpty()) {
        return;
    }

    settings.setValue("ui/lastLoadDir", QFileInfo(filename).absolutePath());

    std::shared_ptr<Structure> structure;
    try {
        structure = structure_loader.load_file(filename.toStdString());
    } catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Exception encountered"), tr(e.what()));
        return;
    }

    structure_stack.clear();
    structure_stack.push_back(structure);
    structure_stack_pointer = 0;

    emit new_file_loaded();

    anaglyph_widget->set_structure(structure);
    structure_info_widget->set_structure(structure);
}

/**
 * @brief open_analysis_file.
 *
 */
void InterfaceWindow::open_analysis_file()
{
    QSettings settings;
    const QString start_dir = settings.value(
        "ui/lastLoadDir",
        QDir::homePath()
    ).toString();

    QString filename = QFileDialog::getOpenFileName(this, tr("Open file"), start_dir, tr("Trajectory / Frequency files (OUTCAR* *.yaml *.yml);;VASP OUTCAR (OUTCAR*);;YAML frequency files (*.yaml *.yml)"));
    if(filename.isEmpty()) {
        return;
    }

    settings.setValue("ui/lastLoadDir", QFileInfo(filename).absolutePath());

    structureAnalysis->load_file(filename);
}

/**
 * @brief open_analysis_neb_calculation.
 *
 */
void InterfaceWindow::open_analysis_neb_calculation()
{
    QSettings settings;
    const QString start_dir = settings.value(
        "ui/lastLoadDir",
        QDir::homePath()
    ).toString();

    const QString folder = QFileDialog::getExistingDirectory(
        this,
        tr("Open NEB calculation"),
        start_dir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if(folder.isEmpty()) {
        return;
    }

    settings.setValue("ui/lastLoadDir", folder);

    NebCalculationLoader neb_loader;
    QString error_message;
    if(!neb_loader.load(folder, &error_message)) {
        QMessageBox message_box(this);
        message_box.setIcon(QMessageBox::Critical);
        message_box.setWindowTitle(tr("Could not open NEB calculation"));
        message_box.setText(tr("The selected folder is not a valid VASP NEB calculation."));
        message_box.setInformativeText(error_message);
        message_box.setStyleSheet("QLabel{min-width: 420px; font-weight: normal;}");
        message_box.exec();
        return;
    }

    std::vector<std::shared_ptr<Structure>> geometry_structures;
    const auto& loaded_structures = neb_loader.structures();
    geometry_structures.reserve(loaded_structures.size());

    for(const auto& structure : loaded_structures) {
        geometry_structures.push_back(structure->clone_for_view());
    }

    structureAnalysis->set_structures(geometry_structures, StructureAnalysisViewer::SeriesKind::NEB);
}

/**
 * @brief save_editor_file.
 *
 */
void InterfaceWindow::save_editor_file()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save file"), "", tr("POSCAR"));
    if(filename.isEmpty()) {
        return;
    }

    this->save_file(filename);
}

    /**
     * @brief      Sets the camera align.
     *
     * @param      action  The action
     */
void InterfaceWindow::set_camera_align(QAction* action) {
    this->anaglyph_widget->get_user_action()->set_camera_alignment(action->data().toInt());
}

    /**
     * @brief      Sets the camera mode (orthogonal or perspective).
     *
     * @param      action  The action
     */
void InterfaceWindow::set_camera_mode(QAction* action) {
    this->anaglyph_widget->get_user_action()->set_camera_mode(action->data().toInt());
}

    /**
     * @brief      Loads a default structure file.
     */
void InterfaceWindow::load_default_file() {
    // do not load default file if a file is already loaded (via CLI)
    if(this->structure_stack.size() != 0) {
        return;
    }

    qDebug() << "Opening default file";
    const std::string filename = "OUTCAR";
    QTemporaryDir tmp_dir;
    QFile::copy(":/assets/structures/" + tr(filename.c_str()), tmp_dir.path() + "/" + filename.c_str());
    this->open_file(tmp_dir.path() + "/" + filename.c_str());
}

    /**
     * @brief      Add a fragment to the selection
     */
void InterfaceWindow::add_fragment() {
    this->anaglyph_widget
        ->get_user_action()
        ->cmd_add_fragment();
}

    /**
     * @brief      Select all atoms
     */
void InterfaceWindow::select_all_atoms() {
    this->anaglyph_widget
        ->get_user_action()
        ->cmd_select_all();
}

    /**
     * @brief      Deselect all atoms
     */
void InterfaceWindow::deselect_all_atoms() {
    this->anaglyph_widget
        ->get_user_action()
        ->cmd_deselect_all();
}

    /**
     * @brief      Select all atoms
     */
void InterfaceWindow::invert_selection() {
    this->anaglyph_widget
        ->get_user_action()
        ->cmd_invert_selection();
}

    /**
     * @brief      Toggle frozen
     */
void InterfaceWindow::set_frozen() {
    this->anaglyph_widget
        ->get_user_action()
        ->cmd_set_frozen();
}
    /**
     * @brief      Set unfrozen
     */
void InterfaceWindow::set_unfrozen() {
    this->anaglyph_widget
        ->get_user_action()
        ->cmd_set_unfrozen();
}

    /**
     * @brief      Update the inform label
     *
     * @param[in]  text  The text
     */
void InterfaceWindow::update_interaction_label(const QString& text) {
    this->interaction_label->setText(text);
}

    /**
     * @brief      Update the inform label
     *
     * @param[in]  text  The text
     */
void InterfaceWindow::update_selection_label(const QString& text) {
    QStringList atomlists = text.split("<br>");
    QStringList pieces1 = atomlists[0].split(";");
    QStringList pieces2 = atomlists[1].split(";");

    // maximum size of the atomlist string
    static const unsigned int sz = 120;

    if(pieces1[0].length() > sz) {
        pieces1[0] = pieces1[0].left(sz) + "...";

    }

    if(pieces2[0].length() > sz) {
        pieces2[0] = pieces2[0].left(sz) + "...";

    }

    atomlists[0] = pieces1.join(";");
    atomlists[1] = pieces2.join(";");

    this->selection_label->setText(atomlists.join("<br>"));
}

    /**
     * @brief Grab the latest structure and push it to the stack and
     *        create a new copy on the stack
     */
void InterfaceWindow::push_structure() {
    // Current structure as used by the renderer/user action
    auto current = this->anaglyph_widget->get_structure();
    if(!current) return;

    // If we undid before, drop redo history
    this->structure_stack.resize(this->structure_stack_pointer + 1);

    // Push a NEW snapshot of the current state
    this->structure_stack.push_back(std::make_shared<Structure>(*current));
    this->structure_stack_pointer++;

    // Make the newly pushed snapshot the active one everywhere
    this->anaglyph_widget->set_structure_conservative(this->structure_stack[this->structure_stack_pointer]);
    this->structure_info_widget->set_structure(this->structure_stack[this->structure_stack_pointer]);

    // Also ensure UserAction uses the same shared_ptr (critical!)
    this->anaglyph_widget->get_user_action()->set_structure(this->structure_stack[this->structure_stack_pointer]);
}

    /**
     * @brief Increment the stack pointer
     */
void InterfaceWindow::increment_structure_stack_pointer() {
    if(this->structure_stack_pointer < (this->structure_stack.size() - 1)) {
        this->structure_stack_pointer++;

        auto s = this->structure_stack[this->structure_stack_pointer];
        this->anaglyph_widget->set_structure_conservative(s);
        this->structure_info_widget->set_structure(s);
        this->anaglyph_widget->get_user_action()->set_structure(s);
    }
}

    /**
     * @brief Decrement the stack pointer
     */
void InterfaceWindow::decrement_structure_stack_pointer() {
    if(this->structure_stack_pointer > 0) {
        this->structure_stack_pointer--;

        auto s = this->structure_stack[this->structure_stack_pointer];
        this->anaglyph_widget->set_structure_conservative(s);
        this->structure_info_widget->set_structure(s);
        this->anaglyph_widget->get_user_action()->set_structure(s);
    }
}

    /**
     * @brief Copy structure from GeometryAnalysis window to Editor
     */
void InterfaceWindow::load_structure_from_geometry_analysis() {
    auto structure = this->structureAnalysis->viewer()->get_anaglyph_widget()->get_structure();
    if(!structure) {
        return;
    }

    auto editor_structure = structure->clone_for_view();

    this->structure_stack.clear();
    this->structure_stack.push_back(editor_structure);
    this->structure_stack_pointer = 0;

    emit new_file_loaded();

    this->anaglyph_widget->set_structure(editor_structure);
    this->structure_info_widget->set_structure(editor_structure);
}
