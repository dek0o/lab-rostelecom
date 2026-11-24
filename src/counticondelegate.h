// Показывает количество фигур значками: 1–3 — один, 4–10 — два, больше 10 — три.

#pragma once

#include <QStyledItemDelegate>

class CountIconDelegate : public QStyledItemDelegate {
public:
    // typeColumn — номер столбца с типом фигуры в модели
    explicit CountIconDelegate(int typeColumn, QObject *parent = nullptr) : QStyledItemDelegate(parent), typeColumn_(typeColumn) {}

    static constexpr int kIconSide = 18;
    static constexpr int kGap = 4;

    // От 1 до 3 - один значок, от 4 до 10 - два, больше 10 - три
    static int iconCount(int count);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int typeColumn_;
};
