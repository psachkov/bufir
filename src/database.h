#pragma once

#include "clipboarditem.h"

#include <QList>
#include <QObject>
#include <QSqlDatabase>

class Database : public QObject {
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database() override;

    bool initialize();
    void close();

    bool saveItem(const ClipboardItem &item);
    bool deleteItem(const QString &id);
    bool clearAll();
    
    [[nodiscard]] QList<ClipboardItem> loadItems(int limit = 1000);
    [[nodiscard]] QList<ClipboardItem> searchItems(const QString &query, int limit = 100);
    
    bool incrementUseCount(const QString &id);
    bool itemExists(const ClipboardItem &item);

signals:
    void error(const QString &message);

private:
    bool createTables();
    bool migrateDatabase();
    
    QSqlDatabase m_db;
    QString m_dbPath;
};
