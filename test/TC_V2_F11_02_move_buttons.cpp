// TC-V2-F11-02 上移/下移按钮
// 中间项移动交换相邻位置；首行上移、末行下移不回绕且顺序不变
#include "config_store.h"
#include "test_helpers.h"

namespace {
QStringList namesIn(ConfigStore& store)
{
    QStringList names;
    for (const QuickItem& it : store.itemsInGroup(QString::fromLatin1(kDefaultGroupId))) {
        names.append(it.name);
    }
    return names;
}

void run()
{
    ConfigStore store(makeTempDir("TC_V2_F11_02") + "/config.json");
    const QString a = store.addItem(QStringLiteral("A"), QStringLiteral("D:/a.txt"),
                                    TargetType::Document);
    const QString b = store.addItem(QStringLiteral("B"), QStringLiteral("D:/b.txt"),
                                    TargetType::Document);
    const QString c = store.addItem(QStringLiteral("C"), QStringLiteral("D:/c.txt"),
                                    TargetType::Document);

    TC_CHECK(store.moveItem(b, -1), "中间项上移应成功");
    TC_CHECK(namesIn(store) == QStringList({QStringLiteral("B"), QStringLiteral("A"),
                                            QStringLiteral("C")}),
             "上移后顺序应为 B,A,C");

    TC_CHECK(!store.moveItem(b, -1), "首行继续上移应失败（不回绕）");
    TC_CHECK(namesIn(store) == QStringList({QStringLiteral("B"), QStringLiteral("A"),
                                            QStringLiteral("C")}),
             "首行上移失败时顺序不变");

    TC_CHECK(!store.moveItem(c, 1), "末行下移应失败（不回绕）");
    TC_CHECK(namesIn(store) == QStringList({QStringLiteral("B"), QStringLiteral("A"),
                                            QStringLiteral("C")}),
             "末行下移失败时顺序不变");

    TC_CHECK(store.moveItem(a, 1), "中间项下移应成功");
    TC_CHECK(namesIn(store) == QStringList({QStringLiteral("B"), QStringLiteral("C"),
                                            QStringLiteral("A")}),
             "下移后顺序应为 B,C,A");
}
} // namespace
REGISTER_TEST("TC-V2-F11-02", run);
