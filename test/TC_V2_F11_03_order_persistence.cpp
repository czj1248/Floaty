// TC-V2-F11-03 自定义顺序持久化
// 排序后写盘，重新加载，组内顺序与 id 一致
#include "config_store.h"
#include "test_helpers.h"

namespace {
void run()
{
    const QString path = makeTempDir("TC_V2_F11_03") + "/config.json";
    QStringList expectedIds;
    {
        ConfigStore writer(path);
        const QString a = writer.addItem(QStringLiteral("A"), QStringLiteral("D:/a.txt"),
                                         TargetType::Document);
        const QString b = writer.addItem(QStringLiteral("B"), QStringLiteral("D:/b.txt"),
                                         TargetType::Document);
        const QString c = writer.addItem(QStringLiteral("C"), QStringLiteral("D:/c.txt"),
                                         TargetType::Document);
        writer.reorderGroup(QString::fromLatin1(kDefaultGroupId), {c, a, b});
        writer.save();
        expectedIds = {c, a, b};
    }
    {
        ConfigStore reader(path);
        reader.load();
        const QList<QuickItem> items =
            reader.itemsInGroup(QString::fromLatin1(kDefaultGroupId));
        TC_CHECK(items.size() == 3, "重启后项数应为 3");
        QStringList actualIds;
        for (const QuickItem& it : items) {
            actualIds.append(it.id);
        }
        TC_CHECK(actualIds == expectedIds, "重启后组内自定义顺序应保持 C,A,B");
        TC_CHECK(items.at(0).order == 0 && items.at(1).order == 1 && items.at(2).order == 2,
                 "顺序字段应为连续的 0,1,2");
    }
}
} // namespace
REGISTER_TEST("TC-V2-F11-03", run);
