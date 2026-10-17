// TC-V2-CFG-02 V2 配置结构整体往返
// 分组、项归属与组内顺序、球位置+显示器、外观设置写盘后完整恢复
#include "config_store.h"
#include "test_helpers.h"

namespace {
void run()
{
    const QString path = makeTempDir("TC_V2_CFG_02") + "/config.json";

    struct Expected {
        QString id;
        QString groupId;
    };
    QList<Expected> defaultOrder;
    QList<Expected> g1Order;
    QString g2Item;

    {
        ConfigStore store(path);
        QString g1;
        QString g2;
        TC_CHECK(store.createGroup(QStringLiteral("工作"), &g1) == GroupError::Ok, "创建工作分组");
        TC_CHECK(store.createGroup(QStringLiteral("生活"), &g2) == GroupError::Ok, "创建生活分组");

        const QString d1 = store.addItem(QStringLiteral("D1"), QStringLiteral("D:/d1.txt"),
                                         TargetType::Document);
        const QString d2 = store.addItem(QStringLiteral("D2"), QStringLiteral("D:/d2.txt"),
                                         TargetType::Document);
        const QString w1 = store.addItem(QStringLiteral("W1"), QStringLiteral("D:/w1.txt"),
                                         TargetType::Document, g1);
        const QString w2 = store.addItem(QStringLiteral("W2"), QStringLiteral("D:/w2.txt"),
                                         TargetType::Document, g1);
        g2Item = store.addItem(QStringLiteral("L1"), QStringLiteral("https://example.com"),
                               TargetType::Url, g2);
        // 默认组逆序排列
        store.reorderGroup(QString::fromLatin1(kDefaultGroupId), {d2, d1});
        defaultOrder = {{d2, QString::fromLatin1(kDefaultGroupId)},
                        {d1, QString::fromLatin1(kDefaultGroupId)}};
        g1Order = {{w1, g1}, {w2, g1}};

        Appearance appr;
        appr.ballSize = 56;
        appr.opacity = 75;
        appr.panelMaxWidth = 240;
        store.setAppearance(appr);
        store.setBallPosition(QPoint(-1200, 320));
        store.setBallScreen(QStringLiteral("\\\\.\\DISPLAY2"));
        store.save();
    }

    {
        ConfigStore reader(path);
        reader.load();

        TC_CHECK(reader.groups().size() == 3, "应恢复默认 + 两个自定义分组");
        TC_CHECK(reader.groups().at(0).id == QLatin1String(kDefaultGroupId),
                 "默认分组恒在首位");
        TC_CHECK(reader.groups().at(1).name == QStringLiteral("工作")
                     && reader.groups().at(2).name == QStringLiteral("生活"),
                 "自定义分组按创建时间排序");

        const QList<QuickItem> def =
            reader.itemsInGroup(QString::fromLatin1(kDefaultGroupId));
        TC_CHECK(def.size() == 2, "默认组项数应为 2");
        TC_CHECK(def.at(0).id == defaultOrder.at(0).id && def.at(1).id == defaultOrder.at(1).id,
                 "默认组自定义顺序应恢复");
        TC_CHECK(def.at(0).order == 0 && def.at(1).order == 1, "默认组顺序字段应连续");

        QString g1 = reader.groups().at(1).id;
        const QList<QuickItem> work = reader.itemsInGroup(g1);
        TC_CHECK(work.size() == 2 && work.at(0).id == g1Order.at(0).id
                      && work.at(1).id == g1Order.at(1).id,
                 "工作组顺序应恢复");
        for (const QuickItem& it : work) {
            TC_CHECK(it.groupId == g1, "归属字段应随项恢复");
        }

        const QList<QuickItem> life = reader.itemsInGroup(reader.groups().at(2).id);
        TC_CHECK(life.size() == 1 && life.first().id == g2Item
                      && life.first().type == TargetType::Url,
                 "生活组的网址项应恢复");

        TC_CHECK(reader.appearance().ballSize == 56 && reader.appearance().opacity == 75
                      && reader.appearance().panelMaxWidth == 240,
                 "外观设置（含面板最大宽度）应完整恢复");
        TC_CHECK(reader.ballPosition() == QPoint(-1200, 320), "球坐标应恢复（可为负坐标）");
        TC_CHECK(reader.ballScreen() == QStringLiteral("\\\\.\\DISPLAY2"),
                 "球所在显示器名称应恢复");
    }
}
} // namespace
REGISTER_TEST("TC-V2-CFG-02", run);
