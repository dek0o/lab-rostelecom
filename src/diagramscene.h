// Обрабатывает действия мышью на сцене: выбор двух фигур для связи и перетаскивание.

#pragma once

#include <QGraphicsScene>
#include <QPointF>

class QGraphicsLineItem;
class Shape;

class DiagramScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit DiagramScene(QObject *parent = nullptr);

    void setLinkMode(bool on);
    bool linkMode() const { return linkMode_; }

    // При удалении фигуры отменяет начатую от неё связь
    void forgetShape(int id);

    // Выделяет фигуру по id. Значение -1 снимает выделение
    void selectOnly(int id);
    int selectedShapeId() const;

    // Расширяет область сцены так, чтобы все фигуры оставались доступными
    void growToItems();

signals:
    void linkRequested(int fromId, int toId);
    void linkModeFinished();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    Shape *shapeAt(const QPointF &scenePos) const;
    Shape *shapeById(int id) const;
    void cancelGesture();

    bool linkMode_ = false;
    int startId_ = -1;  // Id первой фигуры для новой связи, либо -1
    QPointF startPos_;
    QGraphicsLineItem *preview_ = nullptr;
};
