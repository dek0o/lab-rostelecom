#include "link.h"

#include "shapes.h"

#include <QPen>

Link::Link(Shape *from, Shape *to) : from_(from), to_(to) {
    from_->addLink(this);
    to_->addLink(this);
    setZValue(-1);                          // Линия находится под фигурами
    setAcceptedMouseButtons(Qt::NoButton);  // Нажатия получает фигура, а не линия
    setPen(QPen(QColor(60, 60, 60), 2));
    updatePosition();
}

Link::~Link() {
    from_->removeLink(this);
    to_->removeLink(this);
}

// Рамка фигуры расположена вокруг точки (0, 0), поэтому ее центр scenePos() 
void Link::updatePosition() {
    setLine(QLineF(from_->scenePos(), to_->scenePos()));
}
