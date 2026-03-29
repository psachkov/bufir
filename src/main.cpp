#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    // Enable high DPI scaling
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    
    QApplication app(argc, argv);
    
    // Application info
    app.setApplicationName(QStringLiteral("ClipboardManager"));
    app.setApplicationDisplayName(QStringLiteral("Clipboard Manager"));
    app.setOrganizationName(QStringLiteral("clipboard-manager"));
    app.setOrganizationDomain(QStringLiteral("clipboard-manager.local"));
    
    // Set application icon
    app.setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    
    // Ensure data directory exists
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    
    // Create and initialize main window
    MainWindow window;
    window.initialize();
    
    // Don't show window initially - wait for hotkey or tray click
    // window.show();
    
    return app.exec();
}
