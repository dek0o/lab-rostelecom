# qt-paint: фигуры и связи на сцене, хранимые в SQLite и показанные в таблице (лабораторная работа по ООП).
# Вне исходного дерева: service_scripts/run.sh или mkdir build && cd build && qmake ../qt-paint.pro && make
QT += widgets sql
CONFIG += c++17
CONFIG -= app_bundle
TARGET = qt-paint

INCLUDEPATH += src

HEADERS += \
    src/shapes.h \
    src/link.h \
    src/diagramscene.h \
    src/diagram.h \
    src/shapestorage.h \
    src/counticondelegate.h \
    src/mainwindow.h

SOURCES += \
    src/main.cpp \
    src/shapes.cpp \
    src/link.cpp \
    src/diagramscene.cpp \
    src/diagram.cpp \
    src/shapestorage.cpp \
    src/counticondelegate.cpp \
    src/mainwindow.cpp
