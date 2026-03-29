#pragma once

#include <QObject>
#include <QDBusInterface>

class GlobalHotkey : public QObject {
    Q_OBJECT

public:
    explicit GlobalHotkey(QObject *parent = nullptr);
    ~GlobalHotkey() override;

    bool registerHotkey(const QKeySequence &shortcut);
    void unregisterHotkey();

signals:
    void activated();

private slots:
    void onHotkeyPressed();

private:
    bool registerKdeHotkey(const QKeySequence &shortcut);
    bool registerGnomeHotkey(const QKeySequence &shortcut);
    
    QDBusInterface *m_kdeInterface;
    QDBusInterface *m_gnomeInterface;
    QString m_shortcutString;
    bool m_registered;
};
