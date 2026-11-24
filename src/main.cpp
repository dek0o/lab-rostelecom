#include "diagram.h"
#include "diagramscene.h"
#include "mainwindow.h"
#include "shapestorage.h"

#include <QApplication>
#include <QDir>
#include <QMessageBox>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    const QString dbPath = QDir(QCoreApplication::applicationDirPath()).filePath("qt-paint.db");
    ShapeStorage storage;
    if (!storage.open(dbPath)) {
        QMessageBox::critical(nullptr, "qt-paint", "Не удалось открыть базу данных:\n" + dbPath + "\n\n" + storage.lastError());
        return 1;
    }

    DiagramScene scene;
    Diagram diagram(storage, &scene);
    MainWindow window(storage, &scene, diagram);
    window.show();
    return app.exec();
}
