// TC-V2-F11-01 拖拽排序（纯逻辑等价验证）
// reorderGroup 按给定 id 顺序重排；非法清单（数量不符、含别组 id）拒绝
#include "config_store.h"
#include "test_helpers.h"

namespace {
void run()
{
    ConfigStore store(makeTempDir("TC_V2_F11_01") + "/config.json");
    const QString a = store.addItem(QStringLiteral("A"), QStringLiteral("D:/a.txt"),
                                    TargetType::Document);
    const QString b = store.addItem(QStringLiteral("B"), QStringLiteral("D:/b.txt"),
                                    TargetType::Document);
    const QString c = store.addItem(QStringLiteral("C"), QStringLiteral("D:/c.txt"),
                                    TargetType::Document);
    QString other;
    store.createGroup(QStringLiteral("其他"), &other);
    const QString x = store.addItem(QStringLiteral("X"), QStringLiteral("D:/x.txt"),
                                    TargetType::Document, other);

    TC_CHECK(store.reorderGroup(QString::fromLatin1(kDefaultGroupId), {a, c, b})
                  == GroupError::Ok,
             "三项完整顺序清单应接受");
    QStringList names;
    for (const QuickItem& it : store.itemsInGroup(QString::fromLatin1(kDefaultGroupId))) {
        names.append(it.name);
    }
    TC_CHECK(names == QStringList({QStringLiteral("A"), QStringLiteral("C"),
                                   QStringLiteral("B")}),
             "组内顺序应为 A,C,B");

    TC_CHECK(store.reorderGroup(QString::fromLatin1(kDefaultGroupId), {a, c})
                  == GroupError::NotFound,
             "顺序清单缺项应拒绝");
    TC_CHECK(store.reorderGroup(QString::fromLatin1(kDefaultGroupId), {a, c, x})
                  == GroupError::NotFound,
             "清单含别组项应拒绝");
    TC_CHECK(store.reorderGroup(other, {a, b, x}) == GroupError::NotFound,
             "含别组项的跨组排序应拒绝");
}
} // namespace
REGISTER_TEST("TC-V2-F11-01", run);
