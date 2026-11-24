// Хранит все фигуры в SQLite и показывает их в одной таблице.

#pragma once

#include <QList>
#include <QString>
#include <memory>

class QSqlTableModel;

class ShapeStorage {
public:
    enum class Field { Id, Type, LinkedIds, TypeCount };

    ShapeStorage();
    ~ShapeStorage();
    ShapeStorage(const ShapeStorage &) = delete;
    ShapeStorage &operator=(const ShapeStorage &) = delete;

    // Открывает файл БД, очищает таблицу shapes и создаёт модель
    bool open(const QString &dbPath);
    QString lastError() const { return lastError_; }  

    QSqlTableModel *model() const { return model_.get(); }

    // Номер столбца по имени поля в таблице shapes
    int column(Field field) const;

    // Добавляет строку для новой фигуры
    int insertShape(const QString &typeName);

    // Обе фигуры получают id друг друга в linked_ids
    bool addLink(int fromId, int toId);

    // Удаляет фигуру из БД и убирает её id из связей других фигур
    bool removeShape(int id);

    int rowCount() const;

    // Показывает только строки по выбранным условиям
    int setFilter(int id, const QString &typeName, int count);

    // Преобразует linked_ids в список id и обратно
    static QList<int> parseIds(const QString &text);
    static QString joinIds(QList<int> ids);

private:
    void fetchAllRows();  // После select() загружает все строки, а не только первые 256

    std::unique_ptr<QSqlTableModel> model_;
    QString lastError_;
};
