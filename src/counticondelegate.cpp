#include "counticondelegate.h"

#include "shapes.h"

#include <QApplication>
#include <QPainter>
#include <algorithm>

int CountIconDelegate::iconCount(int count) {
    if (count <= 0) return 0;
    if (count <= 3) return 1;
    if (count <= 10) return 2;
    return 3;
}

void CountIconDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.text.clear();  // Только фон и выделение
    const QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    const int count = index.data().toInt();
    const ShapeTypeInfo *info = shapeTypeByName(index.siblingAtColumn(typeColumn_).data().toString());
    const int icons = info ? iconCount(count) : 0;
    const int side = std::min(kIconSide, option.rect.height() - 4);
    const int total = icons * side + std::max(0, icons - 1) * kGap;
    int x = option.rect.left() + (option.rect.width() - total) / 2;
    const int y = option.rect.top() + (option.rect.height() - side) / 2;

    painter->save();
    painter->setClipRect(option.rect);  // Узкий столбец обрезает значки и не затрагивает соседнюю ячейку
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(opt.palette.color(QPalette::Text), 1));
    painter->setBrush(opt.palette.color(QPalette::Base).lighter(110));
    for (int i = 0; i < icons; ++i, x += side + kGap) paintShapeIcon(info->type, painter, QRectF(x, y, side, side));
    painter->restore();
}

QSize CountIconDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const {
    return QSize(3 * kIconSide + 2 * kGap + 12, kIconSide + 6);  // Всегда помещаются три значка
}
