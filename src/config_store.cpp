#include "config_store.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSaveFile>
#include <QStandardPaths>

QString ConfigStore::defaultConfigFilePath()
{
    return qEnvironmentVariable("APPDATA") + QStringLiteral("/Floaty/config.json");
}

ConfigStore::ConfigStore()
    : ConfigStore(defaultConfigFilePath())
{
}

ConfigStore::ConfigStore(const QString& filePath)
    : m_filePath(filePath)
{
}

void ConfigStore::load()
{
    m_items.clear();
    m_hasBallPos = false;
    m_ballPos = QPoint();

    QFile file(m_filePath);
    // 首次启动（文件不存在）：保持空快捷项与无位置状态
    if (!file.exists()) {
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return;
    }
    const QJsonObject root = doc.object();

    // 悬浮球位置
    const QJsonObject ball = root.value(QLatin1String("ball")).toObject();
    if (ball.contains(QLatin1String("x")) && ball.contains(QLatin1String("y"))) {
        m_ballPos = QPoint(ball.value(QLatin1String("x")).toInt(),
                           ball.value(QLatin1String("y")).toInt());
        m_hasBallPos = true;
    }

    // 快捷项列表
    const QJsonArray arr = root.value(QLatin1String("items")).toArray();
    for (const QJsonValue& v : arr) {
        const QJsonObject o = v.toObject();
        const std::optional<TargetType> type =
            targetTypeFromKey(o.value(QLatin1String("type")).toString());
        if (!type.has_value()) {
            continue; // 类型串无效的条目直接丢弃
        }
        QuickItem item;
        item.name = o.value(QLatin1String("name")).toString();
        item.target = o.value(QLatin1String("target")).toString();
        item.type = *type;
        m_items.append(item);
    }
}

void ConfigStore::save()
{
    QJsonObject ball;
    ball.insert(QLatin1String("x"), m_ballPos.x());
    ball.insert(QLatin1String("y"), m_ballPos.y());

    QJsonArray arr;
    for (const QuickItem& item : m_items) {
        QJsonObject o;
        o.insert(QLatin1String("name"), item.name);
        o.insert(QLatin1String("target"), item.target);
        o.insert(QLatin1String("type"), targetTypeKey(item.type));
        arr.append(o);
    }

    QJsonObject root;
    root.insert(QLatin1String("ball"), ball);
    root.insert(QLatin1String("items"), arr);

    QDir().mkpath(QFileInfo(m_filePath).absolutePath());
    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.commit();
}

void ConfigStore::addItem(const QuickItem& item)
{
    m_items.append(item);
}

void ConfigStore::updateItem(int row, const QuickItem& item)
{
    if (row < 0 || row >= m_items.size()) {
        return;
    }
    m_items[row] = item;
}

void ConfigStore::removeItem(int row)
{
    if (row < 0 || row >= m_items.size()) {
        return;
    }
    m_items.removeAt(row);
}

void ConfigStore::setBallPosition(const QPoint& pos)
{
    m_ballPos = pos;
    m_hasBallPos = true;
}
