#include "clipboarditem.h"

#include <QBuffer>
#include <QImage>

ClipboardItem::ClipboardItem()
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_type(Type::Text)
    , m_useCount(0)
    , m_timestamp(QDateTime::currentDateTime())
{
}

ClipboardItem::ClipboardItem(const QString &text, Type type)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_type(type)
    , m_text(text)
    , m_useCount(0)
    , m_timestamp(QDateTime::currentDateTime())
{
}

ClipboardItem::ClipboardItem(const QByteArray &imageData, const QString &format)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_type(Type::Image)
    , m_imageData(imageData)
    , m_imageFormat(format)
    , m_useCount(0)
    , m_timestamp(QDateTime::currentDateTime())
{
}

void ClipboardItem::setText(const QString &text)
{
    m_text = text;
    m_type = Type::Text;
    m_imageData.clear();
}

void ClipboardItem::setImageData(const QByteArray &data, const QString &format)
{
    m_imageData = data;
    m_imageFormat = format;
    m_type = Type::Image;
    m_text.clear();
}

void ClipboardItem::incrementUseCount()
{
    ++m_useCount;
}

void ClipboardItem::setTimestamp(const QDateTime &timestamp)
{
    m_timestamp = timestamp;
}

void ClipboardItem::setId(const QString &id)
{
    m_id = id;
}

void ClipboardItem::setType(Type type)
{
    m_type = type;
}

void ClipboardItem::setUseCount(int count)
{
    m_useCount = count;
}

QString ClipboardItem::displayText() const
{
    if (isImage()) {
        return QStringLiteral("[Изображение]");
    }
    return m_text;
}

QString ClipboardItem::previewText(int maxLength) const
{
    if (isImage()) {
        return QStringLiteral("[Изображение %1x%2]")
            .arg(m_imageData.size() / 1024)
            .arg(m_imageFormat.toUpper());
    }
    
    QString preview = m_text.simplified();
    if (preview.length() > maxLength) {
        preview = preview.left(maxLength - 3) + QStringLiteral("...");
    }
    return preview;
}

QPixmap ClipboardItem::previewPixmap(const QSize &size) const
{
    if (!isImage() || m_imageData.isEmpty()) {
        return QPixmap();
    }
    
    QImage image;
    image.loadFromData(m_imageData, m_imageFormat.toUtf8().constData());
    
    if (image.isNull()) {
        return QPixmap();
    }
    
    QImage scaled = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return QPixmap::fromImage(scaled);
}

bool ClipboardItem::operator==(const ClipboardItem &other) const
{
    if (m_type != other.m_type) {
        return false;
    }
    
    if (isImage()) {
        return m_imageData == other.m_imageData;
    }
    
    return m_text == other.m_text;
}
