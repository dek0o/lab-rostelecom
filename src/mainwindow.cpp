#include "mainwindow.h"

#include "counticondelegate.h"
#include "diagram.h"
#include "diagramscene.h"
#include "shapes.h"
#include "shapestorage.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QSqlTableModel>
#include <QStatusBar>
#include <QTableView>
#include <QVBoxLayout>

MainWindow::MainWindow(ShapeStorage &storage, DiagramScene *scene, Diagram &diagram, QWidget *parent)
    : QMainWindow(parent), storage_(storage), scene_(scene), diagram_(diagram) {
    setWindowTitle("qt-paint");
    setObjectName("mainWindow");

    view_ = new QGraphicsView(scene);
    view_->setObjectName("sceneView");
    view_->setRenderHint(QPainter::Antialiasing);

    auto *splitter = new QSplitter(Qt::Vertical);
    splitter->setObjectName("sceneTableSplitter");
    splitter->addWidget(view_);
    splitter->addWidget(buildTableArea());
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    auto *central = new QWidget;
    central->setObjectName("centralWidget");
    auto *root = new QHBoxLayout(central);
    root->addWidget(buildPanel());
    root->addWidget(splitter, 1);
    setCentralWidget(central);
    resize(1200, 820);
    connectPanel();
    connectFilter();
    connectDelete();
}

QWidget *MainWindow::buildPanel() {
    typeCombo_ = new QComboBox;
    typeCombo_->setObjectName("createTypeCombo");
    widthSpin_ = makeSpin("widthSpin", ShapeLimits::MinSize, ShapeLimits::MaxSize, 90);
    heightSpin_ = makeSpin("heightSpin", ShapeLimits::MinSize, ShapeLimits::MaxSize, 60);
    sidesSpin_ = makeSpin("sidesSpin", ShapeLimits::MinSides, ShapeLimits::MaxSides, 5);
    addButton_ = new QPushButton("Добавить");
    addButton_->setObjectName("addButton");
    linkButton_ = new QPushButton("Связь");
    linkButton_->setObjectName("linkButton");
    linkButton_->setCheckable(true);
    deleteButton_ = new QPushButton("Удалить выбранную");
    deleteButton_->setObjectName("deleteButton");

    auto *createBox = new QGroupBox("Новая фигура");
    createBox->setObjectName("createBox");
    auto *form = new QFormLayout(createBox);
    form->addRow("Тип", typeCombo_);
    form->addRow("Ширина", widthSpin_);
    form->addRow("Высота", heightSpin_);
    form->addRow("Число сторон", sidesSpin_);
    form->addRow(addButton_);

    auto *panel = new QWidget;
    panel->setObjectName("panel");
    auto *layout = new QVBoxLayout(panel);
    layout->addWidget(createBox);
    layout->addWidget(linkButton_);
    layout->addWidget(deleteButton_);
    layout->addStretch();
    return panel;
}

QWidget *MainWindow::buildTableArea() {
    filterIdSpin_ = makeSpin("filterIdSpin", 0, 99999, 0);
    filterIdSpin_->setSpecialValueText("все");
    filterTypeCombo_ = new QComboBox;
    filterTypeCombo_->setObjectName("filterTypeCombo");
    filterCountSpin_ = makeSpin("filterCountSpin", 0, 99999, 0);
    filterCountSpin_->setSpecialValueText("все");
    resetFilterButton_ = new QPushButton("Сбросить");
    resetFilterButton_->setObjectName("resetFilterButton");

    auto *filterRow = new QHBoxLayout;
    filterRow->addWidget(new QLabel("Фильтр — id:"));
    filterRow->addWidget(filterIdSpin_);
    filterRow->addWidget(new QLabel("тип:"));
    filterRow->addWidget(filterTypeCombo_);
    filterRow->addWidget(new QLabel("количество:"));
    filterRow->addWidget(filterCountSpin_);
    filterRow->addWidget(resetFilterButton_);
    filterRow->addStretch();

    table_ = new QTableView;
    table_->setObjectName("shapesTable");
    table_->setModel(storage_.model());
    table_->verticalHeader()->hide();
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setItemDelegateForColumn(storage_.column(ShapeStorage::Field::TypeCount), new CountIconDelegate(storage_.column(ShapeStorage::Field::Type), table_));
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);  // В таблице нельзя менять данные вручную

    auto *area = new QWidget;
    area->setObjectName("tableArea");
    auto *layout = new QVBoxLayout(area);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(filterRow);
    layout->addWidget(table_);
    return area;
}

QSpinBox *MainWindow::makeSpin(const char *name, int min, int max, int value) {
    auto *spin = new QSpinBox;
    spin->setObjectName(name);
    spin->setRange(min, max);
    spin->setValue(value);
    return spin;
}

void MainWindow::connectPanel() {
    for (const ShapeTypeInfo &info : shapeTypes()) typeCombo_->addItem(info.title, info.dbName);
    connect(typeCombo_, &QComboBox::currentIndexChanged, this, &MainWindow::updateSidesField);
    updateSidesField();
    connect(addButton_, &QPushButton::clicked, this, &MainWindow::addShapeFromPanel);
    static const QString linkPrompt = QStringLiteral("Связь: кликните первую фигуру, затем вторую (Esc — отмена)");

    connect(linkButton_, &QPushButton::toggled, this, [this](bool on) {
        scene_->setLinkMode(on);
        if (on) statusBar()->showMessage(linkPrompt);
        else if (statusBar()->currentMessage() == linkPrompt) statusBar()->clearMessage();  // Сообщение об ошибке не стирается
    });
    connect(scene_, &DiagramScene::linkModeFinished, this, [this] { linkButton_->setChecked(false); });
    connect(&diagram_, &Diagram::linkRefused, this, [this](Diagram::LinkRefusal reason) {
        statusBar()->showMessage(reason == Diagram::LinkRefusal::AlreadyLinked  ? QStringLiteral("Эти фигуры уже связаны")
                                 : reason == Diagram::LinkRefusal::StorageError ? QStringLiteral("Не удалось сохранить связь в базе")
                                                                                : QStringLiteral("Связь не создана"),
                                 3000);
    });
}

void MainWindow::updateSidesField() {
    const ShapeTypeInfo *info = shapeTypeByName(typeCombo_->currentData().toString());
    sidesSpin_->setEnabled(info && info->hasSides);
}

void MainWindow::addShapeFromPanel() {
    const ShapeTypeInfo *info = shapeTypeByName(typeCombo_->currentData().toString());

    if (!info) return;

    const int id = diagram_.addShape(info->type, widthSpin_->value(), heightSpin_->value(), sidesSpin_->value());
    statusBar()->showMessage(id < 0 ? QStringLiteral("Не удалось добавить фигуру") : QStringLiteral("Добавлена фигура %1").arg(id), 3000);
}

void MainWindow::connectFilter() {
    filterTypeCombo_->addItem("Все", QString());
    for (const ShapeTypeInfo &info : shapeTypes()) filterTypeCombo_->addItem(info.title, info.dbName);
    connect(filterIdSpin_, &QSpinBox::valueChanged, this, &MainWindow::applyFilter);
    connect(filterCountSpin_, &QSpinBox::valueChanged, this, &MainWindow::applyFilter);
    connect(filterTypeCombo_, &QComboBox::currentIndexChanged, this, &MainWindow::applyFilter);
    connect(resetFilterButton_, &QPushButton::clicked, this, [this] {
        const QSignalBlocker blockId(filterIdSpin_), blockType(filterTypeCombo_), blockCount(filterCountSpin_);
        filterIdSpin_->setValue(0);
        filterTypeCombo_->setCurrentIndex(0);
        filterCountSpin_->setValue(0);
        applyFilter();  // Применяет фильтр один раз
    });
}

void MainWindow::applyFilter() {
    const int shown = storage_.setFilter(filterIdSpin_->value(), filterTypeCombo_->currentData().toString(), filterCountSpin_->value());
    statusBar()->showMessage(QStringLiteral("Показано строк: %1 из %2").arg(shown).arg(storage_.rowCount()), 3000);
}

void MainWindow::connectDelete() {
    connect(deleteButton_, &QPushButton::clicked, this, &MainWindow::deleteSelected);
}

void MainWindow::deleteSelected() {
    const QModelIndexList rows = table_->selectionModel()->selectedRows(storage_.column(ShapeStorage::Field::Id));
    const int id = rows.isEmpty() ? -1 : rows.first().data().toInt();
    linkButton_->setChecked(false);  // Удаление выключает режим связи

    if (id < 0) {
        statusBar()->showMessage("Выберите строку в таблице", 3000);
        return;
    }
    
    const bool removed = diagram_.removeShape(id);
    statusBar()->showMessage(removed ? QStringLiteral("Удалена фигура %1").arg(id) : QStringLiteral("Не удалось удалить фигуру %1").arg(id), 3000);
}
