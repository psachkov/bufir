#include "clipboardmodel.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QDebug>
#include <QImage>
#include <QMimeData>

ClipboardModel::ClipboardModel(QClipboard *clipboard, Database *db, QObject *parent)
    : QAbstractListModel(parent)
    , m_clipboard(clipboard)
    , m_db(db)
    , m_searchActive(false)
{
    // Monitor clipboard changes
    connect(m_clipboard, &QClipboard::changed, this, &ClipboardModel::onClipboardChanged);
    
    // Also check periodically for Wayland compatibility
    m_checkTimer = new QTimer(this);
    m_checkTimer->setInterval(500);
    connect(m_checkTimer, &QTimer::timeout, this, &ClipboardModel::checkClipboard);
    m_checkTimer->start();
    
    loadFromDatabase();
}

int ClipboardModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_items.size();
}

QVariant ClipboardModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size()) {
        return QVariant();
    }
    
    const ClipboardItem &item = m_items.at(index.row());
    
    switch (role) {
    case Qt::DisplayRole:
    case TextRole:
        return item.text();
    case IdRole:
        return item.id();
    case TypeRole:
        return static_cast<int>(item.type());
    case TimestampRole:
        return item.timestamp();
    case IsImageRole:
        return item.isImage();
    case PreviewTextRole:
        return item.previewText(80);
    case PreviewPixmapRole:
        return item.previewPixmap(QSize(48, 48));
    case UseCountRole:
        return item.useCount();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ClipboardModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[TextRole] = "text";
    roles[TypeRole] = "type";
    roles[TimestampRole] = "timestamp";
    roles[IsImageRole] = "isImage";
    roles[PreviewTextRole] = "previewText";
    roles[PreviewPixmapRole] = "previewPixmap";
    roles[UseCountRole] = "useCount";
    return roles;
}

void ClipboardModel::addItem(const ClipboardItem &item)
{
    // Check for duplicates
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i] == item) {
            // Move to top
            if (i > 0) {
                beginMoveRows(QModelIndex(), i, i, QModelIndex(), 0);
                ClipboardItem existing = m_items.takeAt(i);
                existing.incrementUseCount();
                m_items.prepend(existing);
                endMoveRows();
                emit dataChanged(index(0), index(0));
            }
            return;
        }
    }
    
    // Save to database
    m_db->saveItem(item);
    
    // Add to model
    beginInsertRows(QModelIndex(), 0, 0);
    m_items.prepend(item);
    if (!m_searchActive) {
        m_allItems.prepend(item);
    }
    endInsertRows();
    
    emit countChanged(m_items.size());
}

void ClipboardModel::removeItem(int index)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }
    
    const QString id = m_items[index].id();
    m_db->deleteItem(id);
    
    beginRemoveRows(QModelIndex(), index, index);
    m_items.removeAt(index);
    if (!m_searchActive) {
        for (int i = 0; i < m_allItems.size(); ++i) {
            if (m_allItems[i].id() == id) {
                m_allItems.removeAt(i);
                break;
            }
        }
    }
    endRemoveRows();
    
    emit countChanged(m_items.size());
}

void ClipboardModel::clear()
{
    beginResetModel();
    m_items.clear();
    m_allItems.clear();
    m_db->clearAll();
    endResetModel();
    emit countChanged(0);
}

void ClipboardModel::search(const QString &query)
{
    if (query.isEmpty()) {
        clearSearch();
        return;
    }
    
    if (!m_searchActive) {
        m_allItems = m_items;
    }
    
    beginResetModel();
    m_items.clear();
    
    for (const ClipboardItem &item : m_allItems) {
        if (item.previewText().contains(query, Qt::CaseInsensitive)) {
            m_items.append(item);
        }
    }
    
    m_searchActive = true;
    endResetModel();
    emit countChanged(m_items.size());
}

void ClipboardModel::clearSearch()
{
    if (!m_searchActive) {
        return;
    }
    
    beginResetModel();
    m_items = m_allItems;
    m_searchActive = false;
    endResetModel();
    emit countChanged(m_items.size());
}

ClipboardItem ClipboardModel::itemAt(int index) const
{
    if (index < 0 || index >= m_items.size()) {
        return ClipboardItem();
    }
    return m_items.at(index);
}

void ClipboardModel::selectItem(int index)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }
    
    ClipboardItem item = m_items[index];
    item.incrementUseCount();
    m_db->incrementUseCount(item.id());
    
    // Move to top
    if (index > 0) {
        beginMoveRows(QModelIndex(), index, index, QModelIndex(), 0);
        m_items.move(index, 0);
        endMoveRows();
    }
    
    emit dataChanged(this->index(0), this->index(0));
    emit itemSelected(item);
}

void ClipboardModel::selectItem(const QString &id)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id() == id) {
            selectItem(i);
            return;
        }
    }
}

void ClipboardModel::loadFromDatabase()
{
    beginResetModel();
    m_items = m_db->loadItems(500);
    m_allItems = m_items;
    endResetModel();
    emit countChanged(m_items.size());
}

void ClipboardModel::onClipboardChanged(QClipboard::Mode mode)
{
    if (mode != QClipboard::Clipboard) {
        return;
    }
    
    checkClipboard();
}

void ClipboardModel::checkClipboard()
{
    const QMimeData *mimeData = m_clipboard->mimeData();
    if (!mimeData) {
        return;
    }
    
    // Check for image
    if (mimeData->hasImage()) {
        QImage image = qvariant_cast<QImage>(mimeData->imageData());
        if (!image.isNull()) {
            QByteArray ba;
            QBuffer buffer(&ba);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "PNG");
            
            QByteArray hash = QCryptographicHash::hash(ba, QCryptographicHash::Md5);
            if (hash != m_lastImageHash) {
                m_lastImageHash = hash;
                ClipboardItem item(ba, QStringLiteral("PNG"));
                if (!shouldIgnore(item)) {
                    addItem(item);
                }
            }
        }
        return;
    }
    
    // Check for text
    if (mimeData->hasText()) {
        QString text = mimeData->text();
        if (!text.isEmpty() && text != m_lastText) {
            m_lastText = text;
            
            ClipboardItem::Type type = ClipboardItem::Type::Text;
            if (mimeData->hasHtml()) {
                type = ClipboardItem::Type::Html;
            } else if (text.startsWith(QStringLiteral("http://")) || 
                       text.startsWith(QStringLiteral("https://"))) {
                type = ClipboardItem::Type::Url;
            }
            
            ClipboardItem item(text, type);
            if (!shouldIgnore(item)) {
                addItem(item);
            }
        }
    }
}

bool ClipboardModel::shouldIgnore(const ClipboardItem &item) const
{
    // Ignore passwords or sensitive data (simple heuristic)
    if (item.isText()) {
        const QString text = item.text();
        // Skip if text is too short (likely password)
        if (text.length() < 3) {
            return true;
        }
        // Skip if looks like a password field
        if (text.contains(QStringLiteral("password"), Qt::CaseInsensitive) &&
            text.length() < 50) {
            return true;
        }
    }
    return false;
}
