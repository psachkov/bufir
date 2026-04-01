#pragma once

#include <QStyledItemDelegate>
#include <QPainter>

class ClipboardDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit ClipboardDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};
