// TC-V2-F10-01 添加时归入指定分组
// 新建自定义分组，向其新增快捷项：项归属正确，默认组不受影响，分组顺序默认在前
#include "config_store.h"
#include "test_helpers.h"

namespace {
void run()
{
    ConfigStore store(makeTempDir("TC_V2_F10_01") + "/config.json");
    QString groupId;
    TC_CHECK(store.createGroup(QStringLiteral("工作"), &groupId) == GroupError::Ok,
             "应能创建工作分组");
    const QString id = store.addItem(QStringLiteral("笔记"), QStringLiteral("D:/note.txt"),
                                     TargetType::Document, groupId);
    TC_CHECK(!id.isEmpty(), "加入自定义分组应返回新 id");

    TC_CHECK(store.itemsInGroup(groupId).size() == 1, "工作分组应有 1 项");
    TC_CHECK(store.itemsInGroup(QString::fromLatin1(kDefaultGroupId)).isEmpty(),
             "默认分组应仍为空");
    TC_CHECK(store.itemsInGroup(groupId).first().groupId == groupId, "项的归属字段应为工作分组");

    TC_CHECK(store.groups().first().id == QLatin1String(kDefaultGroupId),
             "默认分组应始终排在最前");
}
} // namespace
REGISTER_TEST("TC-V2-F10-01", run);
