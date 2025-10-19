#include "catalog.hpp"
#include "mainwindow.hpp"

#include <QApplication>

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    Catalog catalog;  // GUI keeps its own catalog instance but shares the same core code.
    if (argc > 1) {
        catalog.load(argv[1]);  // Optional preload lets the CLI hand off the active file.
    }

    MainWindow window(std::move(catalog));
    window.resize(960, 600);
    window.show();

    return app.exec();
}
