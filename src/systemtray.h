#pragma once

#include <QAction>
#include <QMenu>
#include <QSystemTrayIcon>

class SystemTray : public QSystemTrayIcon {
    Q_OBJECT

public:
    explicit SystemTray(QWidget *parent = nullptr);

signals:
    void showRequested();
    void quitRequested();
    void settingsRequested();
    void clearHistoryRequested();

private:
    void setupMenu();
    void onActivated(QSystemTrayIcon::ActivationReason reason);

    QMenu *m_menu;
    QAction *m_showAction;
    QAction *m_settingsAction;
    QAction *m_clearAction;
    QAction *m_quitAction;
};
