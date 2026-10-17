// TC-V2-F10-06 默认分组受保护：不可删除、不可重命名
#include "config_store.h"
#include "test_helpers.h"

namespace {
void run()
{
    ConfigStore store(makeTempDir("TC_V2_F10_06") + "/config.json");
    const QString defaultId = QString::fromLatin1(kDefaultGroupId);

    TC_CHECK(store.deleteGroup(defaultId) == GroupError::ProtectedGroup,
             "删除默认分组应被拒绝");
    TC_CHECK(store.renameGroup(defaultId, QStringLiteral("新名字")) == GroupError::ProtectedGroup,
             "重命名默认分组应被拒绝");
    TC_CHECK(store.groups().size() == 1, "默认分组必须仍然存在且唯一");
    TC_CHECK(store.groups().first().protectedGroup, "默认分组应带保护标记");
    TC_CHECK(store.groups().first().name == kDefaultGroupName, "默认分组名称不应被改变");
}
} // namespace
REGISTER_TEST("TC-V2-F10-06", run);
