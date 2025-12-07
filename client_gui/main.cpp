/*
 * MIT License
 * Copyright (c) 2025 OS Chat Project
 *
 * Qt5 Chat Client Application Entry Point
 */

#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
