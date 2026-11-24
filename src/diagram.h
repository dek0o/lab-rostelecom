// Добавляет и удаляет фигуры, создаёт связи и синхронизирует сцену с БД.

#pragma once

#include "shapes.h"

#include <QHash>
#include <QObject>
#include <QPointF>

class DiagramScene;
class ShapeStorage;

// Допустимые размеры новой фигуры
namespace ShapeLimits {
constexpr int MinSize = 10;
constexpr int MaxSize = 400;
constexpr int MinSides = 3;
constexpr int MaxSides = 12;
}  // ShapeLimits

class Diagram : public QObject {
    Q_OBJECT
public:
    enum class LinkRefusal { None, UnknownShape, SameShape, AlreadyLinked, StorageError };
    Q_ENUM(LinkRefusal)

    Diagram(ShapeStorage &storage, DiagramScene *scene, QObject *parent = nullptr);
    ~Diagram() override;  // Удаляет оставшиеся фигуры и их связи

    // Добавляет фигуру в БД и на сцену
    int addShape(ShapeType type, qreal width, qreal height, int sides);

    // Удаляет фигуру из БД, таблицы и сцены вместе со всеми её линиями
    bool removeShape(int id);

public slots:
    // Создаёт в БД связь и рисует линию
    bool addLink(int fromId, int toId);

signals:
    void linkRefused(Diagram::LinkRefusal reason);

private:
    QPointF nextSlotPos();

    ShapeStorage &storage_;
    DiagramScene *scene_;
    QHash<int, Shape *> shapes_;  // Фигуры на сцене по их id
    int nextSlot_ = 0;
};
