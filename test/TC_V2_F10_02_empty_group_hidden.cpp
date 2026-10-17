// TC-V2-F10-02 空分组不在面板显示分区标题
// 空自定义分组不显示；有项后显示；项移空后重新隐藏；默认组同理
#include "config_store.h"
#include "test_helpers.h"

namespace {
void run()
{
    ConfigStore store(makeTempDir("TC_V2_F10_02") + "/config.json");
    QString workId;
    store.createGroup(QStringLiteral("工作"), &workId);

    TC_CHECK(store.visibleGroups().isEmpty(), "全部分组为空时面板无分区标题");

    const QString workItem =
        store.addItem(QStringLiteral("W"), QStringLiteral("D:/w.txt"),
                      TargetType::Document, workId);
    TC_CHECK(store.visibleGroups().size() == 1, "工作分组有项后应只显示它");
    TC_CHECK(store.visibleGroups().first().id == workId, "可见分区应为工作分组");

    store.addItem(QStringLiteral("D"), QStringLiteral("D:/d.txt"), TargetType::Document);
    TC_CHECK(store.visibleGroups().size() == 2, "默认组也有项后应显示两个分区");

    store.removeItem(workItem);
    TC_CHECK(store.visibleGroups().size() == 1, "工作分组清空后其分区标题应消失");
    TC_CHECK(store.visibleGroups().first().id == QLatin1String(kDefaultGroupId),
             "剩余分区应为默认分组");
}
} // namespace
REGISTER_TEST("TC-V2-F10-02", run);
