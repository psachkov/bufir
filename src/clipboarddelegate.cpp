#include "clipboarddelegate.h"
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>

ClipboardDelegate::ClipboardDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void ClipboardDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Custom rendering for clipboard items: white text on dark background with blue highlight on selection
    painter->save();

    // Determine colors based on state
    QColor textColor = QColor("#e8e8f0");
    QColor bgColor = QColor("#2a2a38");
    QColor highlightColor = QColor("#1565c0");

    if (option.state & QStyle::State_Selected) {
        // selection background
        painter->fillRect(option.rect, highlightColor.lighter(60));
        textColor = QColor("#ffffff");
    } else {
        painter->fillRect(option.rect, bgColor);
    }

    // Retrieve text from model
    QString text = index.data(Qt::DisplayRole).toString();

    // Optional icon (DecorationRole)
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));

    // Margins
    QRect textRect = option.rect.adjusted(8, 0, -8, 0);
    QFontMetrics fm = painter->fontMetrics();
    int elide = textRect.width() - 4; // a little padding
    QString elided = fm.elidedText(text, Qt::ElideRight, elide);

    // If there is an icon, reserve space on the left
    int iconSize = 0;
    if (!icon.isNull()) {
        iconSize = option.rect.height() - 6;
        QRect iconRect = QRect(option.rect.left() + 6, option.rect.top() + 3, iconSize, iconSize);
        icon.paint(painter, iconRect, Qt::AlignVCenter | Qt::AlignLeft);
    }
    // Adjust text rect if icon exists
    if (iconSize > 0) {
        textRect = textRect.adjusted(iconSize + 6, 0, 0, 0);
    }
    painter->setPen(textColor);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elided);

    painter->restore();
}

QSize ClipboardDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}
