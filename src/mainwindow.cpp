#include "mainwindow.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QFontDatabase>
#include <QGraphicsDropShadowEffect>
#include <QKeySequence>
#include <QMimeData>
#include <QPainter>
#include <QScreen>
#include <QShortcut>
#include <QTextDocument>
#include <QTimer>
#include <QWindow>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QRegularExpression>
#include <QGuiApplication>

// ClipboardItemDelegate implementation
ClipboardItemDelegate::ClipboardItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ClipboardItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, 
                                   const QModelIndex &index) const
{
    painter->save();
    
    QRect rect = option.rect.adjusted(Padding, Padding, -Padding, -Padding);
    
    // Background
    if (option.state & QStyle::State_Selected) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0, 120, 215, 180));
        painter->drawRoundedRect(option.rect.adjusted(2, 1, -2, -1), 4, 4);
    } else if (option.state & QStyle::State_MouseOver) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 255, 255, 30));
        painter->drawRoundedRect(option.rect.adjusted(2, 1, -2, -1), 4, 4);
    }
    
    // Icon or content preview
    bool isImage = index.data(ClipboardModel::IsImageRole).toBool();
    QString previewText = index.data(ClipboardModel::PreviewTextRole).toString();
    
    if (isImage) {
        QPixmap pixmap = index.data(ClipboardModel::PreviewPixmapRole).value<QPixmap>();
        if (!pixmap.isNull()) {
        QRect iconRect(rect.left(), rect.top() + (rect.height() - IconSize) / 2, 
                          IconSize, IconSize);
            painter->drawPixmap(iconRect, pixmap);
        }
        
        // Draw text label for image
        QRect textRect(rect.left() + IconSize + Padding, rect.top(),
                      rect.width() - IconSize - Padding, rect.height());
        painter->setPen(QColor(200, 200, 200));
        QFont font = painter->font();
        font.setPointSize(9);
        painter->setFont(font);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::TextSingleLine, previewText);
    } else {
        // Draw text content
        painter->setPen(QColor(220, 220, 220));
        QFont font = painter->font();
        font.setPointSize(9);
        painter->setFont(font);
        
        QRect textRect = rect;
        QString text = previewText;
        QTextOption textOption;
        textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        textOption.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        
        painter->drawText(textRect, text, textOption);
    }
    
    // Draw keyboard shortcut indicator
    int row = index.row();
    if (row < 9) {
        QString shortcut = QStringLiteral("⌘%1").arg(row + 1);
        painter->setPen(QColor(150, 150, 150));
        QFont smallFont = painter->font();
        smallFont.setPointSize(7);
        painter->setFont(smallFont);
        painter->drawText(option.rect.adjusted(option.rect.width() - 30, 0, -Padding, 0), 
                         Qt::AlignRight | Qt::AlignVCenter, shortcut);
    }
    
    painter->restore();
}

QSize ClipboardItemDelegate::sizeHint(const QStyleOptionViewItem &option, 
                                       const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    
    bool isImage = index.data(ClipboardModel::IsImageRole).toBool();
    if (isImage) {
        return QSize(260, IconSize + Padding * 2 + 2);
    }

    QFontMetrics fm(option.font);
    int lineHeight = fm.height();
    return QSize(260, lineHeight * MaxTextLines + Padding * 2 + 2);
}

// MainWindow implementation
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
    , m_currentIndex(0)
    , m_ignoreFocusLoss(false)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating, false);
    
    setupUI();
    applyStyles();
    setupConnections();
}

MainWindow::~MainWindow() = default;

void MainWindow::initialize()
{
    // Initialize clipboard model
    QClipboard *clipboard = QApplication::clipboard();
    Database *db = new Database(this);
    
    if (!db->initialize()) {
        qWarning() << "Failed to initialize database";
    }
    
    m_model = new ClipboardModel(clipboard, db, this);
    // Initialize proxy model for filtering/sorting
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_listView->setModel(m_proxyModel);
    // Configure proxy model defaults
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterRole(Qt::DisplayRole);
    // Bind searchEdit changes to proxy filter (live search)
    connect(m_searchEdit, &QLineEdit::textChanged, this, [this](const QString& text){
        QRegularExpression re(text, QRegularExpression::CaseInsensitiveOption);
        m_proxyModel->setFilterRegularExpression(re);
    });

    // Connect model selection to paste action
    connect(m_model, &ClipboardModel::itemSelected, this, [this](const ClipboardItem &item) {
        Q_UNUSED(item)
        pasteSelectedItem();
    });
    
    // Initialize system tray
    m_tray = new SystemTray(this);
    m_tray->show();
    
    connect(m_tray, &SystemTray::showRequested, this, &MainWindow::onTrayShowRequested);
    connect(m_tray, &SystemTray::clearHistoryRequested, this, &MainWindow::onTrayClearHistory);
    connect(m_tray, &SystemTray::quitRequested, this, &MainWindow::onTrayQuit);
    
    // Initialize global hotkey
    m_hotkey = new GlobalHotkey(this);
    if (m_hotkey->registerHotkey(QKeySequence("Ctrl+Alt+G"))) {
        connect(m_hotkey, &GlobalHotkey::activated, this, &MainWindow::onGlobalHotkeyActivated);
        qDebug() << "Global hotkey registered: " << m_hotkey->shortcutString();
    } else {
        qWarning() << "Failed to register global hotkey Ctrl+Alt+G";
    }
    
    // Hide initially
    hide();
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    m_centralWidget->setObjectName(QStringLiteral("centralWidget"));
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    // Reduce margins and spacing for compact layout
    m_mainLayout->setContentsMargins(6, 6, 6, 6);
    m_mainLayout->setSpacing(2);
    
    // Search box
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName(QStringLiteral("searchEdit"));
    m_searchEdit->setPlaceholderText(QStringLiteral("начните печатать для поиска..."));
    m_searchEdit->setClearButtonEnabled(true);
    // Make search edit slightly smaller
    m_searchEdit->setFixedHeight(26);
    m_mainLayout->addWidget(m_searchEdit);
    
    // List view
    m_listView = new QListView(this);
    m_listView->setObjectName(QStringLiteral("listView"));
    m_listView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_listView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listView->setItemDelegate(new ClipboardItemDelegate(m_listView));
    m_listView->setFrameShape(QFrame::NoFrame);
    
    // Custom style for scroll bar
    m_listView->verticalScrollBar()->setStyleSheet(R"(
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: rgba(255, 255, 255, 60);
            border-radius: 4px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: rgba(255, 255, 255, 100);
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )");
    
    m_mainLayout->addWidget(m_listView);

    // Bottom action panel
    QWidget *bottomPanel = new QWidget(this);
    bottomPanel->setObjectName(QStringLiteral("bottomPanel"));
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomPanel);
    bottomLayout->setContentsMargins(0, 4, 0, 0);
    bottomLayout->setSpacing(6);

    m_btnClearAll = new QPushButton(tr("Очистить всё"), bottomPanel);
    m_btnSettings = new QPushButton(tr("Настройки"), bottomPanel);
    m_btnAbout = new QPushButton(tr("О приложении"), bottomPanel);
    m_btnQuit = new QPushButton(tr("Завершить"), bottomPanel);

    // Make buttons compact
    const int btnH = 24;
    m_btnClearAll->setFixedHeight(btnH);
    m_btnSettings->setFixedHeight(btnH);
    m_btnAbout->setFixedHeight(btnH);
    m_btnQuit->setFixedHeight(btnH);

    m_btnClearAll->setObjectName(QStringLiteral("btnClearAll"));
    m_btnSettings->setObjectName(QStringLiteral("btnSettings"));
    m_btnAbout->setObjectName(QStringLiteral("btnAbout"));
    m_btnQuit->setObjectName(QStringLiteral("btnQuit"));

    bottomLayout->addWidget(m_btnClearAll);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_btnSettings);
    bottomLayout->addWidget(m_btnAbout);
    bottomLayout->addWidget(m_btnQuit);

    m_mainLayout->addWidget(bottomPanel);
    
    // Make window narrower and full-height
    const int desiredWidth = 320;
    setFixedWidth(desiredWidth);
    int screenH = QGuiApplication::primaryScreen()->availableGeometry().height();
    // Slightly inset from top/bottom to respect system bars
    setFixedHeight(screenH - 40);
    
    // Install event filter for focus loss detection
    qApp->installEventFilter(this);
}

void MainWindow::applyStyles()
{
    setStyleSheet(R"(
        QMainWindow {
            /* Сделано менее прозрачным (более плотный фон) */
            background: rgba(36, 36, 40, 255);
            border-radius: 12px;
        }
        
        #centralWidget {
            background: transparent;
        }
        
        #searchEdit {
            background: rgba(60, 60, 65, 230);
            border: 1px solid rgba(90, 90, 90, 180);
            border-radius: 6px;
            padding: 6px 10px;
            color: #e0e0e0;
            font-size: 13px;
            selection-background-color: #0078d4;
        }
        
        #searchEdit:focus {
            border: 1px solid rgba(0, 120, 212, 200);
        }
        
        #searchEdit::placeholder {
            color: #888888;
        }
        
        #listView {
            background: transparent;
            border: none;
            outline: none;
        }
        
        #listView::item {
            background: transparent;
            border: none;
            padding: 2px;
        }
    )");
}

void MainWindow::setupConnections()
{
    connect(m_listView, &QListView::activated, this, &MainWindow::onItemActivated);
    connect(m_listView, &QListView::clicked, this, &MainWindow::onItemActivated);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    // Bottom panel actions
    connect(m_btnClearAll, &QPushButton::clicked, this, &MainWindow::onClearAllRequested);
    connect(m_btnSettings, &QPushButton::clicked, this, &MainWindow::onSettingsRequested);
    connect(m_btnAbout, &QPushButton::clicked, this, &MainWindow::onAboutRequested);
    connect(m_btnQuit, &QPushButton::clicked, this, &MainWindow::onQuitRequested);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        hideWindow();
        break;
        
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if (m_listView->currentIndex().isValid()) {
            onItemActivated(m_listView->currentIndex());
        }
        break;
        
    case Qt::Key_Down:
        if (m_listView->currentIndex().row() < m_model->rowCount() - 1) {
            m_listView->setCurrentIndex(m_model->index(m_listView->currentIndex().row() + 1));
        }
        break;
        
    case Qt::Key_Up:
        if (m_listView->currentIndex().row() > 0) {
            m_listView->setCurrentIndex(m_model->index(m_listView->currentIndex().row() - 1));
        } else {
            m_searchEdit->setFocus();
        }
        break;
        
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        if (event->modifiers() & Qt::MetaModifier) {
            int index = event->key() - Qt::Key_1;
            if (index < m_model->rowCount()) {
                onItemActivated(m_model->index(index));
            }
        }
        break;
        
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    
    positionWindow();
    
    m_searchEdit->clear();
    m_searchEdit->setFocus();
    
    if (m_model->rowCount() > 0) {
        m_listView->setCurrentIndex(m_model->index(0));
    }
    
    activateWindow();
    raise();
}

void MainWindow::hideEvent(QHideEvent *event)
{
    QMainWindow::hideEvent(event);
    m_model->clearSearch();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::FocusOut && !m_ignoreFocusLoss) {
        // Hide when focus is lost (click outside)
        QFocusEvent *focusEvent = static_cast<QFocusEvent*>(event);
        if (focusEvent->reason() == Qt::MouseFocusReason) {
            // Delay to allow click events to process
            QTimer::singleShot(100, this, [this]() {
                if (!isActiveWindow()) {
                    hideWindow();
                }
            });
        }
    }
    
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::positionWindow()
{
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    
    int x = (screenGeometry.width() - width()) / 2;
    int y = screenGeometry.height() / 4;
    
    move(screenGeometry.x() + x, screenGeometry.y() + y);
}

void MainWindow::hideWindow()
{
    hide();
    m_model->clearSearch();
}

void MainWindow::onItemActivated(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    
    m_model->selectItem(index.row());
}

void MainWindow::pasteSelectedItem()
{
    hideWindow();
    
    // Simulate paste command
    QKeyEvent *press = new QKeyEvent(QEvent::KeyPress, Qt::Key_V, Qt::ControlModifier);
    QKeyEvent *release = new QKeyEvent(QEvent::KeyRelease, Qt::Key_V, Qt::ControlModifier);
    
    QApplication::postEvent(QApplication::focusWidget(), press);
    QApplication::postEvent(QApplication::focusWidget(), release);
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    m_model->search(text);
    
    if (m_model->rowCount() > 0) {
        m_listView->setCurrentIndex(m_model->index(0));
    }
}

void MainWindow::onGlobalHotkeyActivated()
{
    if (isVisible()) {
        hideWindow();
    } else {
        show();
        raise();
        activateWindow();
    }
}

void MainWindow::onTrayShowRequested()
{
    show();
    raise();
    activateWindow();
}

void MainWindow::onTrayClearHistory()
{
    m_model->clear();
}

void MainWindow::onTrayQuit()
{
    QApplication::quit();
}

// Phase 4: Bottom action panel slots implementations (simple stubs for now)
void MainWindow::onClearAllRequested()
{
    // Clear clipboard history in model
    if (m_model) {
        m_model->clear();
    }
}

void MainWindow::onSettingsRequested()
{
    // Show settings placeholder
    qInfo() << "Settings requested";
}

void MainWindow::onAboutRequested()
{
    // Show about placeholder
    qInfo() << "About requested";
}

void MainWindow::onQuitRequested()
{
    QApplication::quit();
}
