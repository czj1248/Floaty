// TC-V2-F10-07 重命名自定义分组：成功、空名拒绝、重名拒绝
#include "config_store.h"
#include "test_helpers.h"

namespace {
void run()
{
    ConfigStore store(makeTempDir("TC_V2_F10_07") + "/config.json");
    QString workId;
    QString lifeId;
    store.createGroup(QStringLiteral("工作"), &workId);
    store.createGroup(QStringLiteral("生活"), &lifeId);

    TC_CHECK(store.renameGroup(workId, QStringLiteral("  学习  ")) == GroupError::Ok,
             "重命名应成功（首尾空白被裁剪）");
    TC_CHECK(store.groups().at(1).name == QStringLiteral("学习"), "新名称应落库且被裁剪");

    TC_CHECK(store.renameGroup(workId, QStringLiteral("   ")) == GroupError::EmptyName,
             "空名称应拒绝");
    TC_CHECK(store.renameGroup(workId, QStringLiteral("生活")) == GroupError::DuplicateName,
             "与其他分组重名应拒绝");
    TC_CHECK(store.renameGroup(lifeId, QStringLiteral("学习")) == GroupError::DuplicateName,
             "反向重名也应拒绝");
    TC_CHECK(store.renameGroup(QStringLiteral("not-exist"), QStringLiteral("X"))
                  == GroupError::NotFound,
             "不存在的分组应返回 NotFound");
}
} // namespace
REGISTER_TEST("TC-V2-F10-07", run);
