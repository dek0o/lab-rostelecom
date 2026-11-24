#include "diagram.h"

#include "diagramscene.h"
#include "link.h"
#include "shapestorage.h"

#include <QList>

Diagram::Diagram(ShapeStorage &storage, DiagramScene *scene, QObject *parent)
    : QObject(parent), storage_(storage), scene_(scene) {
    connect(scene_, &DiagramScene::linkRequested, this, &Diagram::addLink);
}

// Удаляет фигуры перед удалением сцены
Diagram::~Diagram() {
    const QList<Shape *> shapes = shapes_.values();
    shapes_.clear();
    for (Shape *shape : shapes) delete shape;  // Вместе с фигурой удаляются ее связи
}

int Diagram::addShape(ShapeType type, qreal width, qreal height, int sides) {
    const ShapeTypeInfo &info = shapeTypeInfo(type);
    using namespace ShapeLimits;
    const bool sizesOk = width >= MinSize && width <= MaxSize && height >= MinSize && height <= MaxSize;
    const bool sidesOk = !info.hasSides || (sides >= MinSides && sides <= MaxSides);

    if (!sizesOk || !sidesOk) return -1;

    const int id = storage_.insertShape(info.dbName);

    if (id < 0) return -1;  // Ошибка БД: на сцену ничего не добавляется
    
    Shape *shape = createShape(type, id, width, height, sides);

    if (!shape) {  // Если тип не найден, строка из БД удаляется
        storage_.removeShape(id);
        return -1;
    }

    shape->setPos(nextSlotPos());
    scene_->addItem(shape);
    scene_->growToItems();  // Обновляет размер сцены
    shapes_.insert(id, shape);

    return id;
}

bool Diagram::removeShape(int id) {
    Shape *shape = shapes_.value(id);

    if (!shape) return false;
    if (!storage_.removeShape(id)) return false;  // При ошибке БД фигура и линии остаются на сцене

    scene_->forgetShape(id);                      // Отменяет начатую связь
    shapes_.remove(id);
    delete shape;  // Фигура удаляется со сцены вместе со своими связями

    return true;
}

bool Diagram::addLink(int fromId, int toId) {
    Shape *from = shapes_.value(fromId);
    Shape *to = shapes_.value(toId);
    const LinkRefusal refusal = !from || !to ? LinkRefusal::UnknownShape
                                : fromId == toId ? LinkRefusal::SameShape
                                : from->isLinkedTo(toId) ? LinkRefusal::AlreadyLinked
                                : LinkRefusal::None;

    if (refusal != LinkRefusal::None) {
        emit linkRefused(refusal);  // Окно показывает причину, почему линия не создана
        return false;
    }

    if (!storage_.addLink(fromId, toId)) {  // При ошибке БД линия не рисуется
        emit linkRefused(LinkRefusal::StorageError);
        return false;
    }

    auto *link = new Link(from, to);
    scene_->addItem(link);

    return true;
}

// На сетке 15 мест и каждые следующие 15 фигур сдвигаются на 20 пикселей
QPointF Diagram::nextSlotPos() {
    const int slot = nextSlot_ % 15;
    const int shift = 20 * (nextSlot_ / 15);
    ++nextSlot_;

    return QPointF(100 + 160 * (slot % 5) + shift, 80 + 120 * (slot / 5) + shift);
}
