#include "globalhotkey.h"

#include <KF6/KGlobalAccel/kglobalaccel.h>
#include <QAction>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QKeySequence>

GlobalHotkey::GlobalHotkey(QObject *parent)
    : QObject(parent)
    , m_action(nullptr)
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

    if (m_action) {
        KGlobalAccel::self()->removeAllShortcuts(m_action);
        m_action->deleteLater();
        m_action = nullptr;
    }

    if (m_gnomeInterface && m_gnomeInterface->isValid()) {
        m_gnomeInterface->call(QStringLiteral("UngrabAccelerators"), QVariant::fromValue(QList<uint>()));
        delete m_gnomeInterface;
        m_gnomeInterface = nullptr;
    }

    m_registered = false;
}

bool GlobalHotkey::registerKdeHotkey(const QKeySequence &shortcut)
{
    // Create action with unique object name
    m_action = new QAction(this);
    m_action->setObjectName(QStringLiteral("toggle_clipboard_manager"));
    m_action->setText(QStringLiteral("Toggle Clipboard Manager"));

    // Register global shortcut using KGlobalAccel
    bool success = KGlobalAccel::setGlobalShortcut(m_action, shortcut);
    if (!success) {
        qWarning() << "Failed to register global hotkey with KGlobalAccel";
        delete m_action;
        m_action = nullptr;
        return false;
    }

    // Connect action trigger to our slot
    connect(m_action, &QAction::triggered, this, &GlobalHotkey::onHotkeyPressed);

    m_registered = true;
    qDebug() << "Successfully registered KDE global hotkey:" << shortcut.toString(QKeySequence::NativeText);
    return true;
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
