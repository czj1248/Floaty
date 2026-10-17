// TC-V2-F16-05 混合拖放：合法入、非法拒绝并逐一列出
#include "drop_service.h"
#include "test_helpers.h"

namespace {
void run()
{
    const QString dir = makeTempDir("TC_V2_F16_05");
    const QString ok1 = dir + "/drop1.txt";
    const QString ok2 = dir + "/drop2.txt";
    const QString bad = dir + "/drop_bad.reg";
    TC_CHECK(writeTextFile(ok1, "1"), "前置：drop1");
    TC_CHECK(writeTextFile(ok2, "2"), "前置：drop2");
    TC_CHECK(writeTextFile(bad, "x"), "前置：非法 reg");

    const QStringList input = {ok1, bad, ok2};
    const QList<drop_service::DropVerdict> verdicts =
        drop_service::classifyDroppedPaths(input);

    TC_CHECK(verdicts.size() == 3, "混合拖放应逐项判定且保持顺序");
    TC_CHECK(verdicts.at(0).accepted && !verdicts.at(1).accepted && verdicts.at(2).accepted,
             "合法、非法、合法的判定位置应正确");

    const QStringList accepted = drop_service::acceptedPaths(verdicts);
    const QStringList rejected = drop_service::rejectedReasons(verdicts);
    TC_CHECK(accepted == QStringList({ok1, ok2}), "仅两个合法文件进入接受清单");
    TC_CHECK(rejected.size() == 1, "非法项应被单独列出一次");
    TC_CHECK(rejected.first().contains(QStringLiteral("drop_bad.reg")),
             "拒绝原因应指明具体文件");
}
} // namespace
REGISTER_TEST("TC-V2-F16-05", run);
