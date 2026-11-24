// Описание всех фигур программы.

#pragma once

#include <QGraphicsItem>
#include <QList>
#include <QPainterPath>
#include <QString>
#include <vector>

class Link;

enum class ShapeType { Ellipse, Rectangle, RegularPolygon };

struct ShapeTypeInfo {
    ShapeType type;
    const char *dbName;  // Название типа в БД
    const char *title;   // Название в списке
    bool hasSides;       // Нужно ли число сторон
};

const std::vector<ShapeTypeInfo> &shapeTypes();
const ShapeTypeInfo *shapeTypeByName(const QString &dbName);  // nullptr, если тип не найден
const ShapeTypeInfo &shapeTypeInfo(ShapeType type);

class Shape : public QGraphicsItem {
public:
    enum { Type = UserType + 1 };

    Shape(int id, ShapeType type, qreal width, qreal height);
    ~Shape() override;

    int type() const override { return Type; }
    int id() const { return id_; }
    ShapeType shapeType() const { return shapeType_; }

    // Рисует контур фигуры.
    virtual QPainterPath outline(const QRectF &rect) const = 0;

    QRectF frame() const;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void addLink(Link *link);
    void removeLink(Link *link);
    const QList<Link *> &links() const { return links_; }
    bool isLinkedTo(int otherId) const;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    int id_;
    ShapeType shapeType_;
    qreal width_;
    qreal height_;
    QList<Link *> links_;  // Связи этой фигуры
};

class EllipseShape : public Shape {
public:
    EllipseShape(int id, qreal width, qreal height);
    QPainterPath outline(const QRectF &rect) const override;
};

class RectangleShape : public Shape {
public:
    RectangleShape(int id, qreal width, qreal height);
    QPainterPath outline(const QRectF &rect) const override;
};

class RegularPolygonShape : public Shape {
public:
    RegularPolygonShape(int id, qreal width, qreal height, int sides);
    QPainterPath outline(const QRectF &rect) const override;

private:
    int sides_;
};

// Создаёт нужную фигуру по её типу
Shape *createShape(ShapeType type, int id, qreal width, qreal height, int sides);

// Рисует значок фигуры в таблице
void paintShapeIcon(ShapeType type, QPainter *painter, const QRectF &rect);
