#pragma once

#include "clipboardmodel.h"
#include "globalhotkey.h"
#include "systemtray.h"

#include <QStyledItemDelegate>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListView>
#include <QMainWindow>
#include <QProxyStyle>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>

#if __has_include(<KWindowSystem>)
    #include <KWindowSystem>
    #define HAS_KWINDOWSYSTEM 1
#else
    #define HAS_KWINDOWSYSTEM 0
#endif

#include <QProcess>

// Custom delegate for rendering clipboard items
class ClipboardItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit ClipboardItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, 
               const QModelIndex &index) const override;
    [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option, 
                                  const QModelIndex &index) const override;

private:
    static constexpr int Padding = 8;
    static constexpr int IconSize = 32;
    static constexpr int MaxTextLines = 3;
};

// Main application window
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void initialize();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onItemActivated(const QModelIndex &index);
    void onSearchTextChanged(const QString &text);
    void onGlobalHotkeyActivated();
    void onTrayShowRequested();
    void onTrayClearHistory();
    void onTrayQuit();
    void pasteSelectedItem();

private:
    void setupUI();
    void setupConnections();
    void applyStyles();
    void positionWindow();
    void hideWindow();

    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QLineEdit *m_searchEdit;
    QListView *m_listView;

    // Data
    ClipboardModel *m_model;
    SystemTray *m_tray;
    GlobalHotkey *m_hotkey;
    
    int m_currentIndex;
    bool m_ignoreFocusLoss;
};
