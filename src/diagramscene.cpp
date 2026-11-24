#include "diagramscene.h"

#include "shapes.h"

#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPen>

DiagramScene::DiagramScene(QObject *parent) : QGraphicsScene(0, 0, 820, 380, parent) {
    setBackgroundBrush(Qt::white);  // Одинаковый вид в светлой и тёмной системных темах
}

void DiagramScene::setLinkMode(bool on) {
    if (on == linkMode_) return;  // Режим уже установлен
    linkMode_ = on;
    if (!on) cancelGesture();
}

void DiagramScene::forgetShape(int id) {
    if (startId_ == id) cancelGesture();
}

void DiagramScene::selectOnly(int id) {
    clearSelection();
    if (Shape *shape = id >= 0 ? shapeById(id) : nullptr) shape->setSelected(true);
}

int DiagramScene::selectedShapeId() const {
    for (QGraphicsItem *item : selectedItems())
        if (auto *shape = qgraphicsitem_cast<Shape *>(item)) return shape->id();
    return -1;
}

void DiagramScene::growToItems() {
    const QRectF grown = sceneRect().united(itemsBoundingRect().adjusted(-20, -20, 20, 20));
    if (grown != sceneRect()) setSceneRect(grown);
}

Shape *DiagramScene::shapeAt(const QPointF &scenePos) const {
    for (QGraphicsItem *item : items(scenePos))
        if (auto *shape = qgraphicsitem_cast<Shape *>(item)) return shape;
    return nullptr;
}

Shape *DiagramScene::shapeById(int id) const {
    for (QGraphicsItem *item : items())
        if (auto *shape = qgraphicsitem_cast<Shape *>(item); shape && shape->id() == id) return shape;
    return nullptr;
}

void DiagramScene::cancelGesture() {
    delete preview_;
    preview_ = nullptr;
    startId_ = -1;
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    Shape *hit = shapeAt(event->scenePos());

    if (!linkMode_) {
        QGraphicsScene::mousePressEvent(event);  // Обычный выбор и перетаскивание
        return;
    }

    if (event->button() != Qt::LeftButton) return;  // Связь создаётся левой кнопкой мыши

    if (startId_ < 0) {
        if (!hit) return;  // Режим связи остаётся включённым
        startId_ = hit->id();
        startPos_ = hit->scenePos();
        preview_ = addLine(QLineF(startPos_, event->scenePos()), QPen(Qt::gray, 1.5, Qt::DashLine));
        preview_->setZValue(-2);
        return;
    }

    const int fromId = startId_;
    const int toId = hit ? hit->id() : -1;
    setLinkMode(false);  // Убирает линию выбора

    if (toId >= 0 && toId != fromId) emit linkRequested(fromId, toId);  // Нажатие мимо или по той же фигуре отменяет связь

    emit linkModeFinished();
}

void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (preview_) preview_->setLine(QLineF(startPos_, event->scenePos()));
    else QGraphicsScene::mouseMoveEvent(event);
}

void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseReleaseEvent(event);
    growToItems();  // Обновляет размер сцены
}

void DiagramScene::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape && linkMode_) {
        setLinkMode(false);
        emit linkModeFinished();
        return;
    }
    QGraphicsScene::keyPressEvent(event);
}
