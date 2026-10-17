// TC-V2-F10-05 删除自定义分组：组内项归入默认分组
#include "config_store.h"
#include "test_helpers.h"

namespace {
void run()
{
    ConfigStore store(makeTempDir("TC_V2_F10_05") + "/config.json");
    QString workId;
    store.createGroup(QStringLiteral("工作"), &workId);
    store.addItem(QStringLiteral("W1"), QStringLiteral("D:/w1.txt"), TargetType::Document,
                  workId);
    store.addItem(QStringLiteral("W2"), QStringLiteral("D:/w2.txt"), TargetType::Document,
                  workId);
    store.addItem(QStringLiteral("D1"), QStringLiteral("D:/d1.txt"), TargetType::Document);

    TC_CHECK(store.deleteGroup(workId) == GroupError::Ok, "删除自定义分组应成功");
    TC_CHECK(!store.groupExists(workId), "分组记录应被删除");

    const QList<QuickItem> def = store.itemsInGroup(QString::fromLatin1(kDefaultGroupId));
    TC_CHECK(def.size() == 3, "组内两项应并入默认分组");
    QStringList names;
    for (const QuickItem& it : def) {
        names.append(it.name);
        TC_CHECK(it.groupId == QLatin1String(kDefaultGroupId), "并入项归属应改为默认分组");
    }
    TC_CHECK(names.contains(QStringLiteral("W1")) && names.contains(QStringLiteral("W2")),
             "并入的项不应丢失");
}
} // namespace
REGISTER_TEST("TC-V2-F10-05", run);
