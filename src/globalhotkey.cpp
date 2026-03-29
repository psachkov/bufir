#include "globalhotkey.h"

#include <QDBusConnection>
#include <QDebug>
#include <QKeySequence>

GlobalHotkey::GlobalHotkey(QObject *parent)
    : QObject(parent)
    , m_kdeInterface(nullptr)
    , m_gnomeInterface(nullptr)
    , m_registered(false)
{
}

GlobalHotkey::~GlobalHotkey()
{
    unregisterHotkey();
}

bool GlobalHotkey::registerHotkey(const QKeySequence &shortcut)
{
    unregisterHotkey();
    
    m_shortcutString = shortcut.toString(QKeySequence::NativeText);
    
    // Try KDE first, then GNOME
    if (registerKdeHotkey(shortcut)) {
        return true;
    }
    
    if (registerGnomeHotkey(shortcut)) {
        return true;
    }
    
    qWarning() << "Failed to register global hotkey. Desktop environment not supported.";
    return false;
}

void GlobalHotkey::unregisterHotkey()
{
    if (!m_registered) {
        return;
    }
    
    if (m_kdeInterface && m_kdeInterface->isValid()) {
        m_kdeInterface->call(QStringLiteral("unRegister"), QStringLiteral("clipboard-manager-toggle"));
    }
    
    if (m_gnomeInterface && m_gnomeInterface->isValid()) {
        m_gnomeInterface->call(QStringLiteral("UngrabAccelerators"), QVariant::fromValue(QList<uint>()));
    }
    
    m_registered = false;
}

bool GlobalHotkey::registerKdeHotkey(const QKeySequence &shortcut)
{
    m_kdeInterface = new QDBusInterface(
        QStringLiteral("org.kde.kglobalaccel"),
        QStringLiteral("/kglobalaccel"),
        QStringLiteral("org.kde.KGlobalAccel"),
        QDBusConnection::sessionBus(),
        this
    );
    
    if (!m_kdeInterface->isValid()) {
        delete m_kdeInterface;
        m_kdeInterface = nullptr;
        return false;
    }
    
    // Register component
    QDBusInterface componentInterface(
        QStringLiteral("org.kde.kglobalaccel"),
        QStringLiteral("/kglobalaccel"),
        QStringLiteral("org.kde.kglobalaccel.Component"),
        QDBusConnection::sessionBus()
    );
    
    // Try to register the shortcut
    QDBusReply<bool> reply = m_kdeInterface->call(
        QStringLiteral("register"),
        QStringLiteral("clipboard-manager"),
        QStringLiteral("clipboard-manager-toggle"),
        shortcut.toString(),
        shortcut.toString(),
        shortcut.toString(),
        QLatin1String(),
        0
    );
    
    if (reply.isValid() && reply.value()) {
        m_registered = true;
        
        // Connect to the activated signal
        QDBusConnection::sessionBus().connect(
            QStringLiteral("org.kde.kglobalaccel"),
            QStringLiteral("/component/clipboard-manager"),
            QStringLiteral("org.kde.kglobalaccel.Component"),
            QStringLiteral("globalShortcutActivated"),
            this,
            SLOT(onHotkeyPressed())
        );
        
        return true;
    }
    
    return false;
}

bool GlobalHotkey::registerGnomeHotkey(const QKeySequence &shortcut)
{
    m_gnomeInterface = new QDBusInterface(
        QStringLiteral("org.gnome.Shell"),
        QStringLiteral("/org/gnome/Shell"),
        QStringLiteral("org.gnome.Shell"),
        QDBusConnection::sessionBus(),
        this
    );
    
    if (!m_gnomeInterface->isValid()) {
        delete m_gnomeInterface;
        m_gnomeInterface = nullptr;
        return false;
    }
    
    // GNOME uses a different approach - we'd need to use a different method
    // For now, this is a placeholder for GNOME support
    qWarning() << "GNOME global hotkey support not yet fully implemented";
    
    delete m_gnomeInterface;
    m_gnomeInterface = nullptr;
    return false;
}

void GlobalHotkey::onHotkeyPressed()
{
    emit activated();
}
