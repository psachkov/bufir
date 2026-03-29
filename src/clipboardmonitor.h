#pragma once

#include <QObject>
#include <QClipboard>

// Wrapper around QClipboard for monitoring changes
class ClipboardMonitor : public QObject {
    Q_OBJECT

public:
    explicit ClipboardMonitor(QClipboard *clipboard, QObject *parent = nullptr);

signals:
    void textCopied(const QString &text);
    void imageCopied(const QByteArray &imageData, const QString &format);
    void dataChanged();

private slots:
    void onClipboardChanged(QClipboard::Mode mode);
    void pollClipboard();

private:
    QClipboard *m_clipboard;
    QString m_lastText;
    QByteArray m_lastImageHash;
};
