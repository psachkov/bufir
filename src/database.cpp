#include "database.h"

#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

Database::Database(QObject *parent)
    : QObject(parent)
{
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    m_dbPath = dataDir + QStringLiteral("/clipboard.db");
}

Database::~Database()
{
    close();
}

bool Database::initialize()
{
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        emit error(QStringLiteral("Failed to open database: %1").arg(m_db.lastError().text()));
        return false;
    }

    if (!createTables()) {
        return false;
    }

    return true;
}

void Database::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool Database::createTables()
{
    QSqlQuery query(m_db);
    
    const QString createTable = QStringLiteral(R"(
        CREATE TABLE IF NOT EXISTS clipboard_items (
            id TEXT PRIMARY KEY,
            type INTEGER NOT NULL,
            text_content TEXT,
            image_data BLOB,
            image_format TEXT,
            timestamp INTEGER NOT NULL,
            use_count INTEGER DEFAULT 0,
            checksum TEXT
        )
    )");
    
    if (!query.exec(createTable)) {
        emit error(QStringLiteral("Failed to create table: %1").arg(query.lastError().text()));
        return false;
    }
    
    // Create index on timestamp for faster sorting
    query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_timestamp ON clipboard_items(timestamp DESC)"));
    query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_text ON clipboard_items(text_content)"));
    
    return true;
}

bool Database::saveItem(const ClipboardItem &item)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(R"(
        INSERT OR REPLACE INTO clipboard_items 
        (id, type, text_content, image_data, image_format, timestamp, use_count, checksum)
        VALUES (:id, :type, :text, :image_data, :image_format, :timestamp, :use_count, :checksum)
    )"));
    
    query.bindValue(QStringLiteral(":id"), item.id());
    query.bindValue(QStringLiteral(":type"), static_cast<int>(item.type()));
    query.bindValue(QStringLiteral(":text"), item.text());
    query.bindValue(QStringLiteral(":image_data"), item.imageData());
    query.bindValue(QStringLiteral(":image_format"), item.imageFormat());
    query.bindValue(QStringLiteral(":timestamp"), item.timestamp().toSecsSinceEpoch());
    query.bindValue(QStringLiteral(":use_count"), item.useCount());
    
    // Simple checksum for deduplication
    QString checksum;
    if (item.isImage()) {
        checksum = QString::number(qHash(item.imageData()));
    } else {
        checksum = QString::number(qHash(item.text()));
    }
    query.bindValue(QStringLiteral(":checksum"), checksum);
    
    if (!query.exec()) {
        emit error(QStringLiteral("Failed to save item: %1").arg(query.lastError().text()));
        return false;
    }
    
    return true;
}

bool Database::deleteItem(const QString &id)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("DELETE FROM clipboard_items WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), id);
    return query.exec();
}

bool Database::clearAll()
{
    QSqlQuery query(m_db);
    return query.exec(QStringLiteral("DELETE FROM clipboard_items"));
}

QList<ClipboardItem> Database::loadItems(int limit)
{
    QList<ClipboardItem> items;
    
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(R"(
        SELECT id, type, text_content, image_data, image_format, timestamp, use_count 
        FROM clipboard_items 
        ORDER BY timestamp DESC 
        LIMIT :limit
    )"));
    query.bindValue(QStringLiteral(":limit"), limit);
    
    if (!query.exec()) {
        emit error(QStringLiteral("Failed to load items: %1").arg(query.lastError().text()));
        return items;
    }
    
    while (query.next()) {
        ClipboardItem::Type type = static_cast<ClipboardItem::Type>(query.value(1).toInt());
        ClipboardItem item;
        
        item.setId(query.value(0).toString());
        item.setTimestamp(QDateTime::fromSecsSinceEpoch(query.value(5).toLongLong()));
        item.setUseCount(query.value(6).toInt());
        item.setType(type);
        
        if (type == ClipboardItem::Type::Image) {
            item.setImageData(query.value(3).toByteArray(), query.value(4).toString());
        } else {
            item.setText(query.value(2).toString());
        }
        
        items.append(item);
    }
    
    return items;
}

QList<ClipboardItem> Database::searchItems(const QString &queryStr, int limit)
{
    QList<ClipboardItem> items;
    
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral(R"(
        SELECT id, type, text_content, image_data, image_format, timestamp, use_count 
        FROM clipboard_items 
        WHERE text_content LIKE :query
        ORDER BY timestamp DESC 
        LIMIT :limit
    )"));
    query.bindValue(QStringLiteral(":query"), QStringLiteral("%%%1%%").arg(queryStr));
    query.bindValue(QStringLiteral(":limit"), limit);
    
    if (!query.exec()) {
        emit error(QStringLiteral("Failed to search items: %1").arg(query.lastError().text()));
        return items;
    }
    
    while (query.next()) {
        // Build items from query results
    }
    
    return items;
}

bool Database::incrementUseCount(const QString &id)
{
    QSqlQuery query(m_db);
    query.prepare(QStringLiteral("UPDATE clipboard_items SET use_count = use_count + 1 WHERE id = :id"));
    query.bindValue(QStringLiteral(":id"), id);
    return query.exec();
}

bool Database::itemExists(const ClipboardItem &item)
{
    QSqlQuery query(m_db);
    
    QString checksum;
    if (item.isImage()) {
        checksum = QString::number(qHash(item.imageData()));
        query.prepare(QStringLiteral("SELECT 1 FROM clipboard_items WHERE checksum = :checksum AND image_data = :data"));
        query.bindValue(QStringLiteral(":data"), item.imageData());
    } else {
        checksum = QString::number(qHash(item.text()));
        query.prepare(QStringLiteral("SELECT 1 FROM clipboard_items WHERE checksum = :checksum AND text_content = :text"));
        query.bindValue(QStringLiteral(":text"), item.text());
    }
    
    query.bindValue(QStringLiteral(":checksum"), checksum);
    
    if (query.exec() && query.next()) {
        return true;
    }
    
    return false;
}

bool Database::migrateDatabase()
{
    // Future database migrations go here
    return true;
}
