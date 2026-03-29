#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QMetaType>
#include <QPixmap>
#include <QString>
#include <QUuid>

class ClipboardItem {
public:
    enum class Type {
        Text,
        Image,
        Html,
        Url
    };

    ClipboardItem();
    ClipboardItem(const QString &text, Type type = Type::Text);
    ClipboardItem(const QByteArray &imageData, const QString &format);

    [[nodiscard]] QString id() const { return m_id; }
    [[nodiscard]] Type type() const { return m_type; }
    [[nodiscard]] QString text() const { return m_text; }
    [[nodiscard]] QByteArray imageData() const { return m_imageData; }
    [[nodiscard]] QString imageFormat() const { return m_imageFormat; }
    [[nodiscard]] QDateTime timestamp() const { return m_timestamp; }
    [[nodiscard]] int useCount() const { return m_useCount; }

    void setText(const QString &text);
    void setImageData(const QByteArray &data, const QString &format);
    void incrementUseCount();
    void setTimestamp(const QDateTime &timestamp);
    void setId(const QString &id);
    void setType(Type type);
    void setUseCount(int count);

    [[nodiscard]] bool isImage() const { return m_type == Type::Image; }
    [[nodiscard]] bool isText() const { return m_type == Type::Text || m_type == Type::Html || m_type == Type::Url; }
    [[nodiscard]] QString displayText() const;
    [[nodiscard]] QString previewText(int maxLength = 100) const;
    [[nodiscard]] QPixmap previewPixmap(const QSize &size = QSize(64, 64)) const;

    bool operator==(const ClipboardItem &other) const;

private:
    QString m_id;
    Type m_type;
    QString m_text;
    QByteArray m_imageData;
    QString m_imageFormat;
    QDateTime m_timestamp;
    int m_useCount;
};

Q_DECLARE_METATYPE(ClipboardItem)
