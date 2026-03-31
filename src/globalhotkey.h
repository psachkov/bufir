#pragma once

#include <QObject>
#include <QKeySequence>

class QAction;
class QDBusInterface;

class GlobalHotkey : public QObject {
    Q_OBJECT

public:
    explicit GlobalHotkey(QObject *parent = nullptr);
    ~GlobalHotkey() override;

    bool registerHotkey(const QKeySequence &shortcut);
    void unregisterHotkey();
    
    QString shortcutString() const { return m_shortcutString; }

signals:
    void activated();

private slots:
    void onHotkeyPressed();

private:
    bool registerKdeHotkey(const QKeySequence &shortcut);
    bool registerGnomeHotkey(const QKeySequence &shortcut);
    
    QAction *m_action;
    QDBusInterface *m_gnomeInterface;
    QString m_shortcutString;
    bool m_registered;
};
