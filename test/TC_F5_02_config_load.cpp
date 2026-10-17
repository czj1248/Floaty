// TC-F5-02 重启恢复状态（自动化部分）
// 覆盖：重新加载后快捷项列表（含顺序、类型）与悬浮球位置与保存时完全一致。
// 启动后展开面板的界面恢复为人工观察项。
#include <QList>

#include "config_store.h"
#include "test_helpers.h"

namespace {
void run()
{
    const QString dir = makeTempDir("TC_F5_02");
    const QString path = dir + "/config.json";

    {
        ConfigStore writer(path);
        writer.addItem(QStringLiteral("我的笔记"), QStringLiteral("D:/docs/note.txt"),
                       TargetType::Document);
        writer.addItem(QStringLiteral("测试网址"), QStringLiteral("https://example.com"),
                       TargetType::Url);
        writer.addItem(QStringLiteral("工具箱"), QStringLiteral("D:/tools"), TargetType::Folder);
        writer.setBallPosition(QPoint(777, 333));
        writer.save();
    }
    {
        // 模拟重启：新建存储并读取同一份配置
        ConfigStore reader(path);
        reader.load();
        const QList<QuickItem> items = reader.itemsInGroup(QString::fromLatin1(kDefaultGroupId));
        TC_CHECK(items.size() == 3, "快捷项数量应恢复一致");
        TC_CHECK(items.at(0).name == QStringLiteral("我的笔记"), "第 1 项名称应恢复");
        TC_CHECK(items.at(0).target == QStringLiteral("D:/docs/note.txt"),
                 "第 1 项目标应恢复");
        TC_CHECK(items.at(0).type == TargetType::Document, "第 1 项类型应恢复");
        TC_CHECK(items.at(1).target == QStringLiteral("https://example.com"),
                 "第 2 项目标应恢复");
        TC_CHECK(items.at(1).type == TargetType::Url, "第 2 项类型应恢复");
        TC_CHECK(items.at(2).type == TargetType::Folder, "第 3 项类型应恢复");
        TC_CHECK(reader.ballPosition() == QPoint(777, 333), "悬浮球坐标应恢复");
        TC_CHECK(reader.hasBallPosition(), "位置标记应恢复");
    }
}
} // namespace
REGISTER_TEST("TC-F5-02", run);
