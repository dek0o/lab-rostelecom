#include "shapes.h"

#include "link.h"

#include <QPainter>
#include <QtMath>
#include <algorithm>
#include <memory>

const std::vector<ShapeTypeInfo> &shapeTypes() {
    static const std::vector<ShapeTypeInfo> types = {
        {ShapeType::Ellipse, "ellipse", "Эллипс", false},
        {ShapeType::Rectangle, "rectangle", "Прямоугольник", false},
        {ShapeType::RegularPolygon, "polygon", "Правильный многоугольник", true},
    };
    return types;
}

const ShapeTypeInfo *shapeTypeByName(const QString &dbName) {
    for (const ShapeTypeInfo &info : shapeTypes())
        if (dbName == QLatin1String(info.dbName)) return &info;
    return nullptr;
}

const ShapeTypeInfo &shapeTypeInfo(ShapeType type) {
    for (const ShapeTypeInfo &info : shapeTypes())
        if (info.type == type) return info;
    return shapeTypes().front();  // Запасной вариант
}

Shape::Shape(int id, ShapeType type, qreal width, qreal height)
    : id_(id), shapeType_(type), width_(width), height_(height) {
    // Нужно, чтобы линии двигались вместе с фигурой
    setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
}

Shape::~Shape() {
    const QList<Link *> copy = links_;  // При удалении связь убирается из списка
    for (Link *link : copy) delete link;
}

QRectF Shape::frame() const {
    return QRectF(-width_ / 2, -height_ / 2, width_, height_);
}

QRectF Shape::boundingRect() const {
    return frame().adjusted(-2, -2, 2, 2); 
}

QPainterPath Shape::shape() const {
    return outline(frame());  // Нажатия учитываются только внутри фигуры
}

void Shape::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 1.5));
    painter->setBrush(isSelected() ? QColor(210, 225, 245) : QColor(Qt::white));
    painter->drawPath(outline(frame()));
}

void Shape::addLink(Link *link) {
    links_.append(link);
}

void Shape::removeLink(Link *link) {
    links_.removeAll(link);
}

bool Shape::isLinkedTo(int otherId) const {
    for (Link *link : links_)
        if (link->other(this)->id() == otherId) return true;
    return false;
}

QVariant Shape::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemPositionHasChanged)
        for (Link *link : links_) link->updatePosition();
    return QGraphicsItem::itemChange(change, value);
}

EllipseShape::EllipseShape(int id, qreal width, qreal height) : Shape(id, ShapeType::Ellipse, width, height) {}

QPainterPath EllipseShape::outline(const QRectF &rect) const {
    QPainterPath path;
    path.addEllipse(rect);
    return path;
}

RectangleShape::RectangleShape(int id, qreal width, qreal height) : Shape(id, ShapeType::Rectangle, width, height) {}

QPainterPath RectangleShape::outline(const QRectF &rect) const {
    QPainterPath path;
    path.addRect(rect);
    return path;
}

RegularPolygonShape::RegularPolygonShape(int id, qreal width, qreal height, int sides)
    : Shape(id, ShapeType::RegularPolygon, width, height), sides_(sides) {}

QPainterPath RegularPolygonShape::outline(const QRectF &rect) const {
    const qreal radius = std::min(rect.width(), rect.height()) / 2;
    QPolygonF polygon;

    for (int i = 0; i < sides_; ++i) {
        const qreal angle = qDegreesToRadians(-90.0 + 360.0 * i / sides_);  // Первая вершина расположена сверху
        polygon << rect.center() + QPointF(radius * qCos(angle), radius * qSin(angle));
    }

    QPainterPath path;
    path.addPolygon(polygon);
    path.closeSubpath();
    return path;
}

Shape *createShape(ShapeType type, int id, qreal width, qreal height, int sides) {
    switch (type) {
    case ShapeType::Ellipse: return new EllipseShape(id, width, height);
    case ShapeType::Rectangle: return new RectangleShape(id, width, height);
    case ShapeType::RegularPolygon: return new RegularPolygonShape(id, width, height, sides);
    }
    return nullptr;  // Тип не найден
}

void paintShapeIcon(ShapeType type, QPainter *painter, const QRectF &rect) {
    const int iconSides = 5; 
    std::unique_ptr<Shape> sample(createShape(type, 0, rect.width(), rect.height(), iconSides));
    if (sample) painter->drawPath(sample->outline(rect));  // Если тип не найден, ячейка остаётся пустой
}
