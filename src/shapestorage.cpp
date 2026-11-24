#include "shapestorage.h"

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QStringList>
#include <algorithm>

ShapeStorage::ShapeStorage() = default;

ShapeStorage::~ShapeStorage() {
    model_.reset();
    QSqlDatabase::database().close();
}

bool ShapeStorage::open(const QString &dbPath) {
    lastError_.clear();
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        lastError_ = db.lastError().text();
        return false;
    }

    // Id не повторяются, поэтому старая связь не укажет на новую фигуру
    QSqlQuery query(db);
    const bool created = query.exec("DROP TABLE IF EXISTS shapes")
                         && query.exec("CREATE TABLE shapes ("
                                       "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                       "type TEXT NOT NULL, "
                                       "linked_ids TEXT NOT NULL DEFAULT '', "
                                       "type_count INTEGER NOT NULL DEFAULT 0)");

    if (!created) {
        lastError_ = query.lastError().text();
        return false;
    }

    // В таблице нельзя менять данные вручную
    model_ = std::make_unique<QSqlTableModel>(nullptr, db);
    model_->setTable("shapes");
    model_->setEditStrategy(QSqlTableModel::OnManualSubmit);
    const bool selected = model_->select();
    fetchAllRows();

    return selected;
}

int ShapeStorage::column(Field field) const {
    static const char *const names[] = {"id", "type", "linked_ids", "type_count"};  // Порядок полей Field
    return model_ ? model_->fieldIndex(names[int(field)]) : -1;
}

// QSQLITE сразу загружает только первые 256 строк
void ShapeStorage::fetchAllRows() {
    while (model_->canFetchMore()) model_->fetchMore();
}

namespace {

// Пересчитывает количество фигур каждого типа
bool recountTypeCounts(QSqlQuery &query) {
    return query.exec("UPDATE shapes SET type_count = (SELECT COUNT(*) FROM shapes AS t WHERE t.type = shapes.type)");
}

// Читает linked_ids одной строки
enum class RowRead { Found, Missing, Failed };
RowRead readLinkedIds(QSqlQuery &query, int id, QString &text) {
    query.prepare("SELECT linked_ids FROM shapes WHERE id = ?");
    query.addBindValue(id);

    if (!query.exec()) return RowRead::Failed;
    if (!query.next()) return RowRead::Missing;

    text = query.value(0).toString();

    return RowRead::Found;
}

// Пустой список нужно записать как пустую строку
bool writeLinkedIds(QSqlQuery &query, int id, const QString &text) {
    query.prepare("UPDATE shapes SET linked_ids = ? WHERE id = ?");
    query.addBindValue(text.isNull() ? QStringLiteral("") : text);
    query.addBindValue(id);
    return query.exec() && query.numRowsAffected() == 1;
}

}  // Вспомогательные функции

int ShapeStorage::insertShape(const QString &typeName) {
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    int id = -1;
    bool ok = db.transaction();
    const bool inTransaction = ok;  // Если начать операцию не удалось, откат не нужен

    if (ok) {
        query.prepare("INSERT INTO shapes (type) VALUES (?)");
        query.addBindValue(typeName);
        ok = query.exec();
        if (ok) id = query.lastInsertId().toInt();
    }

    ok = ok && recountTypeCounts(query) && db.commit();
    if (!ok) {
        if (inTransaction) db.rollback();
        return -1;
    }

    model_->select();
    fetchAllRows();
    return id;
}

bool ShapeStorage::addLink(int fromId, int toId) {
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    QString fromText;
    QString toText;
    bool ok = fromId != toId && db.transaction();
    const bool inTransaction = ok;
    ok = ok && readLinkedIds(query, fromId, fromText) == RowRead::Found && readLinkedIds(query, toId, toText) == RowRead::Found;
    
    if (ok) {
        QList<int> fromIds = parseIds(fromText);
        QList<int> toIds = parseIds(toText);
        fromIds.append(toId);
        toIds.append(fromId);
        ok = writeLinkedIds(query, fromId, joinIds(fromIds)) && writeLinkedIds(query, toId, joinIds(toIds)) && db.commit();
    }

    if (!ok) {
        if (inTransaction) db.rollback();
        return false;
    }

    model_->select();
    fetchAllRows();

    return true;
}

bool ShapeStorage::removeShape(int id) {
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    QString ownText;
    QList<int> neighbours;
    bool ok = db.transaction();
    const bool inTransaction = ok;
    ok = ok && readLinkedIds(query, id, ownText) == RowRead::Found;  // Если строки нет, удалить её нельзя

    if (ok) neighbours = parseIds(ownText);

    for (int neighbour : neighbours) {
        QString text;
        if (!ok) break;
        const RowRead read = readLinkedIds(query, neighbour, text);
        if (read == RowRead::Missing) continue;  // В несуществующей строке нечего менять
        ok = read == RowRead::Found;              // При ошибке чтения удаление отменяется
        if (!ok) break;
        QList<int> ids = parseIds(text);
        ids.removeAll(id);
        ok = writeLinkedIds(query, neighbour, joinIds(ids));
    }

    if (ok) {
        query.prepare("DELETE FROM shapes WHERE id = ?");
        query.addBindValue(id);
        ok = query.exec() && query.numRowsAffected() == 1;
    }

    ok = ok && recountTypeCounts(query) && db.commit();
    if (!ok) {
        if (inTransaction) db.rollback();
        return false;
    }

    model_->select();
    fetchAllRows();

    return true;
}

int ShapeStorage::rowCount() const {
    QSqlQuery query(QSqlDatabase::database());
    return query.exec("SELECT COUNT(*) FROM shapes") && query.next() ? query.value(0).toInt() : -1;
}

QList<int> ShapeStorage::parseIds(const QString &text) {
    QList<int> ids;

    for (const QString &part : text.split(',', Qt::SkipEmptyParts)) {
        bool isNumber = false;
        const int id = part.trimmed().toInt(&isNumber);
        if (isNumber) ids.append(id);
    }

    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());

    return ids;
}

QString ShapeStorage::joinIds(QList<int> ids) {
    std::sort(ids.begin(), ids.end());
    ids.erase(std::unique(ids.begin(), ids.end()), ids.end());
    QStringList parts;
    for (int id : ids) parts << QString::number(id);

    return parts.join(',');
}

int ShapeStorage::setFilter(int id, const QString &typeName, int count) {
    QStringList conditions;

    if (id > 0) conditions << QStringLiteral("id = %1").arg(id);

    if (!typeName.isEmpty()) {
        QSqlField field("type", QMetaType(QMetaType::QString));
        field.setValue(typeName);
        conditions << QStringLiteral("type = %1").arg(QSqlDatabase::database().driver()->formatValue(field));
    }

    if (count > 0) conditions << QStringLiteral("type_count = %1").arg(count);

    const QString where = conditions.join(" AND ");
    model_->setFilter(where);  // Модель сама обновляет список строк
    fetchAllRows();
    
    return model_->rowCount();
}
