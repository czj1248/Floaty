#include "config_store.h"

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSaveFile>
#include <QUuid>

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
    // 默认分组是结构常量，任何状态下都存在
    m_groups.append(QuickGroup{QString::fromLatin1(kDefaultGroupId), kDefaultGroupName, true});
}

void ConfigStore::load()
{
    m_groups.clear();
    m_items.clear();
    m_hasBallPos = false;
    m_ballPos = QPoint();
    m_ballScreen.clear();
    m_appearance = Appearance();
    m_groups.append(QuickGroup{QString::fromLatin1(kDefaultGroupId), kDefaultGroupName, true});

    QFile file(m_filePath);
    if (!file.exists()) {
        return; // 首次启动：V2 默认空状态
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return;
    }
    const QJsonObject root = doc.object();

    // 分组：默认分组始终以结构常量为准，自定义分组按文件顺序
    const QJsonArray groupArr = root.value(QLatin1String("groups")).toArray();
    for (const QJsonValue& v : groupArr) {
        const QJsonObject o = v.toObject();
        const QString id = o.value(QLatin1String("id")).toString();
        if (id == QLatin1String(kDefaultGroupId) || id.isEmpty()) {
            continue;
        }
        m_groups.append(QuickGroup{id, o.value(QLatin1String("name")).toString(), false});
    }

    // 快捷项
    const QJsonArray itemArr = root.value(QLatin1String("items")).toArray();
    for (const QJsonValue& v : itemArr) {
        const QJsonObject o = v.toObject();
        const std::optional<TargetType> type =
            targetTypeFromKey(o.value(QLatin1String("type")).toString());
        const QString groupId = o.value(QLatin1String("groupId")).toString(
            QLatin1String(kDefaultGroupId));
        if (!type.has_value() || !groupExists(groupId)) {
            continue;
        }
        QuickItem item;
        item.id = o.value(QLatin1String("id")).toString();
        item.name = o.value(QLatin1String("name")).toString();
        item.target = o.value(QLatin1String("target")).toString();
        item.type = *type;
        item.groupId = groupId;
        item.order = o.value(QLatin1String("order")).toInt(0);
        m_items.append(item);
    }
    // 恢复后统一重排顺序，避免顺序字段不连续
    for (const QuickGroup& g : m_groups) {
        renumberGroup(g.id);
    }

    // 悬浮球位置与显示器
    const QJsonObject ball = root.value(QLatin1String("ball")).toObject();
    if (ball.contains(QLatin1String("x")) && ball.contains(QLatin1String("y"))) {
        m_ballPos = QPoint(ball.value(QLatin1String("x")).toInt(),
                           ball.value(QLatin1String("y")).toInt());
        m_hasBallPos = true;
    }
    m_ballScreen = ball.value(QLatin1String("screen")).toString();

    // 外观设置
    const QJsonObject appr = root.value(QLatin1String("appearance")).toObject();
    if (appr.contains(QLatin1String("ballSize"))) {
        m_appearance.ballSize = appr.value(QLatin1String("ballSize")).toInt();
    }
    if (appr.contains(QLatin1String("opacity"))) {
        m_appearance.opacity = appr.value(QLatin1String("opacity")).toInt();
    }
    if (appr.contains(QLatin1String("panelMaxWidth"))) {
        m_appearance.panelMaxWidth = appr.value(QLatin1String("panelMaxWidth")).toInt();
    }
}

void ConfigStore::save()
{
    QJsonArray groupArr;
    for (const QuickGroup& g : m_groups) {
        QJsonObject o;
        o.insert(QLatin1String("id"), g.id);
        o.insert(QLatin1String("name"), g.name);
        groupArr.append(o);
    }

    QJsonArray itemArr;
    for (const QuickItem& item : m_items) {
        QJsonObject o;
        o.insert(QLatin1String("id"), item.id);
        o.insert(QLatin1String("name"), item.name);
        o.insert(QLatin1String("target"), item.target);
        o.insert(QLatin1String("type"), targetTypeKey(item.type));
        o.insert(QLatin1String("groupId"), item.groupId);
        o.insert(QLatin1String("order"), item.order);
        itemArr.append(o);
    }

    QJsonObject ball;
    ball.insert(QLatin1String("x"), m_ballPos.x());
    ball.insert(QLatin1String("y"), m_ballPos.y());
    ball.insert(QLatin1String("screen"), m_ballScreen);

    QJsonObject appr;
    appr.insert(QLatin1String("ballSize"), m_appearance.ballSize);
    appr.insert(QLatin1String("opacity"), m_appearance.opacity);
    appr.insert(QLatin1String("panelMaxWidth"), m_appearance.panelMaxWidth);

    QJsonObject root;
    root.insert(QLatin1String("groups"), groupArr);
    root.insert(QLatin1String("items"), itemArr);
    root.insert(QLatin1String("ball"), ball);
    root.insert(QLatin1String("appearance"), appr);

    QDir().mkpath(QFileInfo(m_filePath).absolutePath());
    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.commit();
}

GroupError ConfigStore::createGroup(const QString& name, QString* newId)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        return GroupError::EmptyName;
    }
    for (const QuickGroup& g : m_groups) {
        if (g.name == trimmed) {
            return GroupError::DuplicateName;
        }
    }
    QuickGroup group;
    group.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    group.name = trimmed;
    m_groups.append(group);
    if (newId) {
        *newId = group.id;
    }
    return GroupError::Ok;
}

GroupError ConfigStore::renameGroup(const QString& id, const QString& name)
{
    QuickGroup* g = findGroup(id);
    if (!g) {
        return GroupError::NotFound;
    }
    if (g->protectedGroup) {
        return GroupError::ProtectedGroup;
    }
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        return GroupError::EmptyName;
    }
    for (const QuickGroup& other : m_groups) {
        if (other.id != id && other.name == trimmed) {
            return GroupError::DuplicateName;
        }
    }
    g->name = trimmed;
    return GroupError::Ok;
}

GroupError ConfigStore::deleteGroup(const QString& id)
{
    if (id == QLatin1String(kDefaultGroupId)) {
        return GroupError::ProtectedGroup;
    }
    QuickGroup* g = findGroup(id);
    if (!g) {
        return GroupError::NotFound;
    }
    // 组内快捷项移入默认分组末尾
    for (QuickItem& item : m_items) {
        if (item.groupId == id) {
            item.groupId = QLatin1String(kDefaultGroupId);
        }
    }
    renumberGroup(QLatin1String(kDefaultGroupId));
    for (int i = 0; i < m_groups.size(); ++i) {
        if (m_groups.at(i).id == id) {
            m_groups.removeAt(i);
            break;
        }
    }
    return GroupError::Ok;
}

QList<QuickGroup> ConfigStore::visibleGroups() const
{
    QList<QuickGroup> result;
    for (const QuickGroup& g : m_groups) {
        for (const QuickItem& item : m_items) {
            if (item.groupId == g.id) {
                result.append(g);
                break;
            }
        }
    }
    return result;
}

bool ConfigStore::groupExists(const QString& id) const
{
    for (const QuickGroup& g : m_groups) {
        if (g.id == id) {
            return true;
        }
    }
    return false;
}

QList<QuickItem> ConfigStore::itemsInGroup(const QString& groupId) const
{
    QList<QuickItem> result;
    for (const QuickItem& item : m_items) {
        if (item.groupId == groupId) {
            result.append(item);
        }
    }
    std::sort(result.begin(), result.end(),
              [](const QuickItem& a, const QuickItem& b) { return a.order < b.order; });
    return result;
}

QString ConfigStore::addItem(const QString& name, const QString& target, TargetType type,
                             const QString& groupId)
{
    if (!groupExists(groupId)) {
        return QString();
    }
    const QList<QuickItem> groupItems = itemsInGroup(groupId);
    QuickItem item;
    item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.name = name;
    item.target = target;
    item.type = type;
    item.groupId = groupId;
    item.order = groupItems.size();
    m_items.append(item);
    return item.id;
}

GroupError ConfigStore::updateItem(const QString& id, const QString& name, const QString& target,
                                   TargetType type, const QString& groupId)
{
    QuickItem* item = findItem(id);
    if (!item) {
        return GroupError::NotFound;
    }
    if (!groupExists(groupId)) {
        return GroupError::NotFound;
    }
    item->name = name;
    item->target = target;
    item->type = type;
    if (item->groupId != groupId) {
        // 改变归属：排到目标分组末尾
        item->groupId = groupId;
        item->order = itemsInGroup(groupId).size();
        renumberGroup(groupId);
    }
    return GroupError::Ok;
}

GroupError ConfigStore::removeItem(const QString& id)
{
    const QString groupId = [&] {
        for (const QuickItem& item : m_items) {
            if (item.id == id) {
                return item.groupId;
            }
        }
        return QString();
    }();
    if (groupId.isEmpty()) {
        return GroupError::NotFound;
    }
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == id) {
            m_items.removeAt(i);
            break;
        }
    }
    renumberGroup(groupId);
    return GroupError::Ok;
}

bool ConfigStore::moveItem(const QString& id, int delta)
{
    QuickItem* item = findItem(id);
    if (!item || (delta != -1 && delta != 1)) {
        return false;
    }
    QList<QuickItem> groupItems = itemsInGroup(item->groupId);
    const int index = [&] {
        for (int i = 0; i < groupItems.size(); ++i) {
            if (groupItems.at(i).id == id) {
                return i;
            }
        }
        return -1;
    }();
    const int target = index + delta;
    if (index < 0 || target < 0 || target >= groupItems.size()) {
        return false; // 边界处不回绕
    }
    std::swap(groupItems[index], groupItems[target]);
    reorderGroup(item->groupId, [&] {
        QStringList ids;
        for (const QuickItem& it : groupItems) {
            ids.append(it.id);
        }
        return ids;
    }());
    return true;
}

GroupError ConfigStore::reorderGroup(const QString& groupId, const QStringList& orderedIds)
{
    if (!groupExists(groupId)) {
        return GroupError::NotFound;
    }
    QList<QuickItem> groupItems = itemsInGroup(groupId);
    if (orderedIds.size() != groupItems.size()) {
        return GroupError::NotFound; // 顺序清单必须覆盖且仅覆盖该组
    }
    QList<QuickItem> reordered;
    for (const QString& id : orderedIds) {
        bool found = false;
        for (const QuickItem& item : groupItems) {
            if (item.id == id) {
                reordered.append(item);
                found = true;
                break;
            }
        }
        if (!found) {
            return GroupError::NotFound;
        }
    }
    for (int i = 0; i < reordered.size(); ++i) {
        if (QuickItem* p = findItem(reordered.at(i).id)) {
            p->order = i;
        }
    }
    return GroupError::Ok;
}

void ConfigStore::setBallPosition(const QPoint& pos)
{
    m_ballPos = pos;
    m_hasBallPos = true;
}

QuickItem* ConfigStore::findItem(const QString& id)
{
    for (QuickItem& item : m_items) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}

QuickGroup* ConfigStore::findGroup(const QString& id)
{
    for (QuickGroup& g : m_groups) {
        if (g.id == id) {
            return &g;
        }
    }
    return nullptr;
}

void ConfigStore::renumberGroup(const QString& groupId)
{
    QList<QuickItem*> groupItems;
    for (QuickItem& item : m_items) {
        if (item.groupId == groupId) {
            groupItems.append(&item);
        }
    }
    std::sort(groupItems.begin(), groupItems.end(),
              [](const QuickItem* a, const QuickItem* b) { return a->order < b->order; });
    for (int i = 0; i < groupItems.size(); ++i) {
        groupItems.at(i)->order = i;
    }
}
