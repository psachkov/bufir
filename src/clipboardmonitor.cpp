#include "clipboardmonitor.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QImage>
#include <QMimeData>
#include <QTimer>

ClipboardMonitor::ClipboardMonitor(QClipboard *clipboard, QObject *parent)
    : QObject(parent)
    , m_clipboard(clipboard)
{
    connect(m_clipboard, &QClipboard::changed, this, &ClipboardMonitor::onClipboardChanged);
    
    // Poll every 500ms for Wayland compatibility
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ClipboardMonitor::pollClipboard);
    timer->start(500);
}

void ClipboardMonitor::onClipboardChanged(QClipboard::Mode mode)
{
    if (mode == QClipboard::Clipboard) {
        pollClipboard();
    }
}

void ClipboardMonitor::pollClipboard()
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
                emit imageCopied(ba, QStringLiteral("PNG"));
                emit dataChanged();
            }
        }
        return;
    }
    
    // Check for text
    if (mimeData->hasText()) {
        QString text = mimeData->text();
        if (!text.isEmpty() && text != m_lastText) {
            m_lastText = text;
            emit textCopied(text);
            emit dataChanged();
        }
    }
}
