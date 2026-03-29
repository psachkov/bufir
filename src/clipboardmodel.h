#pragma once

#include "clipboarditem.h"
#include "database.h"

#include <QAbstractListModel>
#include <QClipboard>
#include <QTimer>

class ClipboardModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit ClipboardModel(QClipboard *clipboard, Database *db, QObject *parent = nullptr);

    enum Roles {
        IdRole = Qt::UserRole + 1,
        TextRole,
        TypeRole,
        TimestampRole,
        IsImageRole,
        PreviewTextRole,
        PreviewPixmapRole,
        UseCountRole
    };

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void addItem(const ClipboardItem &item);
    void removeItem(int index);
    void clear();
    void search(const QString &query);
    void clearSearch();
    
    [[nodiscard]] ClipboardItem itemAt(int index) const;
    void selectItem(int index);
    void selectItem(const QString &id);

    void loadFromDatabase();

signals:
    void itemSelected(const ClipboardItem &item);
    void countChanged(int count);

private slots:
    void onClipboardChanged(QClipboard::Mode mode);
    void checkClipboard();

private:
    bool shouldIgnore(const ClipboardItem &item) const;
    
    QClipboard *m_clipboard;
    Database *m_db;
    QList<ClipboardItem> m_items;
    QList<ClipboardItem> m_allItems; // For search restoration
    QTimer *m_checkTimer;
    QString m_lastText;
    QByteArray m_lastImageHash;
    bool m_searchActive;
};
