// Связь двух фигур в виде линии между их центрами.

#pragma once

#include <QGraphicsLineItem>

class Shape;

class Link : public QGraphicsLineItem {
public:
    enum { Type = UserType + 2 };

    Link(Shape *from, Shape *to);
    ~Link() override;

    int type() const override { return Type; }
    Shape *from() const { return from_; }
    Shape *to() const { return to_; }
    Shape *other(const Shape *shape) const { return shape == from_ ? to_ : from_; }

    void updatePosition();

private:
    Shape *from_;
    Shape *to_;
};
