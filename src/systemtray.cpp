#include "systemtray.h"

#include <QApplication>
#include <QStyle>

SystemTray::SystemTray(QWidget *parent)
    : QSystemTrayIcon(parent)
{
    setToolTip(QStringLiteral("Clipboard Manager"));
    
    // Set icon (will use default if custom not available)
    setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    
    setupMenu();
    
    connect(this, &QSystemTrayIcon::activated, this, &SystemTray::onActivated);
}

void SystemTray::setupMenu()
{
    m_menu = new QMenu();
    
    m_showAction = new QAction(QStringLiteral("Показать историю"), this);
    connect(m_showAction, &QAction::triggered, this, &SystemTray::showRequested);
    m_menu->addAction(m_showAction);
    
    m_menu->addSeparator();
    
    m_clearAction = new QAction(QStringLiteral("Очистить историю"), this);
    connect(m_clearAction, &QAction::triggered, this, &SystemTray::clearHistoryRequested);
    m_menu->addAction(m_clearAction);
    
    m_settingsAction = new QAction(QStringLiteral("Настройки..."), this);
    connect(m_settingsAction, &QAction::triggered, this, &SystemTray::settingsRequested);
    m_menu->addAction(m_settingsAction);
    
    m_menu->addSeparator();
    
    m_quitAction = new QAction(QStringLiteral("Завершить"), this);
    connect(m_quitAction, &QAction::triggered, this, &SystemTray::quitRequested);
    m_menu->addAction(m_quitAction);
    
    setContextMenu(m_menu);
}

void SystemTray::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        emit showRequested();
    }
}
