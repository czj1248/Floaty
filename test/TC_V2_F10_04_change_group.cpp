// TC-V2-F10-04 编辑时改变分组归属
// 默认组两项、工作组一项，将默认组第二项改到工作组：它排到目标组末尾
#include "config_store.h"
#include "test_helpers.h"

namespace {
void run()
{
    ConfigStore store(makeTempDir("TC_V2_F10_04") + "/config.json");
    QString workId;
    store.createGroup(QStringLiteral("工作"), &workId);

    store.addItem(QStringLiteral("A1"), QStringLiteral("D:/a1.txt"), TargetType::Document);
    const QString a2 =
        store.addItem(QStringLiteral("A2"), QStringLiteral("D:/a2.txt"), TargetType::Document);
    store.addItem(QStringLiteral("B1"), QStringLiteral("D:/b1.txt"), TargetType::Document,
                  workId);

    TC_CHECK(store.updateItem(a2, QStringLiteral("A2改"), QStringLiteral("D:/a2.txt"),
                              TargetType::Document, workId)
                  == GroupError::Ok,
             "编辑改归属应成功");

    const QList<QuickItem> def = store.itemsInGroup(QString::fromLatin1(kDefaultGroupId));
    const QList<QuickItem> work = store.itemsInGroup(workId);
    TC_CHECK(def.size() == 1 && def.first().name == QStringLiteral("A1"),
             "原分组应只剩 A1");
    TC_CHECK(work.size() == 2, "目标分组应有 2 项");
    TC_CHECK(work.at(0).name == QStringLiteral("B1") && work.at(1).name == QStringLiteral("A2改"),
             "移入项应排到目标分组末尾");
}
} // namespace
REGISTER_TEST("TC-V2-F10-04", run);
