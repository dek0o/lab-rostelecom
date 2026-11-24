// Главное окно программы: панель создания, сцена, фильтр и таблица.

#pragma once

#include <QMainWindow>

class Diagram;
class DiagramScene;
class QComboBox;
class QGraphicsView;
class QPushButton;
class QSpinBox;
class QTableView;
class ShapeStorage;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(ShapeStorage &storage, DiagramScene *scene, Diagram &diagram, QWidget *parent = nullptr);

private slots:
    void addShapeFromPanel();
    void updateSidesField();
    void applyFilter();
    void deleteSelected();

private:
    void connectPanel();
    void connectFilter();
    void connectDelete();
    QWidget *buildPanel();
    QWidget *buildTableArea();
    QSpinBox *makeSpin(const char *name, int min, int max, int value);

    ShapeStorage &storage_;
    DiagramScene *scene_;
    Diagram &diagram_;

    // Панель создания фигур
    QComboBox *typeCombo_ = nullptr;
    QSpinBox *widthSpin_ = nullptr;
    QSpinBox *heightSpin_ = nullptr;
    QSpinBox *sidesSpin_ = nullptr;
    QPushButton *addButton_ = nullptr;
    QPushButton *linkButton_ = nullptr;
    QPushButton *deleteButton_ = nullptr;

    // Сцена и таблица
    QGraphicsView *view_ = nullptr;
    QTableView *table_ = nullptr;

    // Поля фильтра
    QSpinBox *filterIdSpin_ = nullptr;
    QComboBox *filterTypeCombo_ = nullptr;
    QSpinBox *filterCountSpin_ = nullptr;
    QPushButton *resetFilterButton_ = nullptr;

};
