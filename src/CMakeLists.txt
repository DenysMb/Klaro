# SPDX-FileCopyrightText: 2025 Denys Madureira
# SPDX-License-Identifier: GPL-3.0-or-later

add_executable(klaro)

ecm_add_qml_module(klaro
    URI
    io.github.denysmb.klaro
)

target_sources(klaro
    PRIVATE
    main.cpp
)

ecm_target_qml_sources(klaro
    SOURCES
    qml/Main.qml
)

target_link_libraries(klaro
    PRIVATE
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::QuickControls2
    Qt6::Widgets
    KF6::I18n
    KF6::CoreAddons
    KF6::IconThemes
)

install(TARGETS klaro ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
