// TC-V2-F16-04 非法拖放内容拒绝
// 白名单外扩展名、不存在路径、非本地资源（网址字符串）一律拒绝并给出原因
#include "drop_service.h"
#include "test_helpers.h"

namespace {
void run()
{
    const QString dir = makeTempDir("TC_V2_F16_04");
    const QString bad = dir + "/drop_bad.reg";
    TC_CHECK(writeTextFile(bad, "x"), "前置：创建白名单外文件");

    const QStringList input = {
        bad,                                   // 白名单外扩展名
        dir + "/missing.txt",                  // 不存在
        QStringLiteral("https://example.com"), // 网址不是本地文件
    };
    const QList<drop_service::DropVerdict> verdicts =
        drop_service::classifyDroppedPaths(input);
    TC_CHECK(verdicts.size() == 3, "三项均应得到判定结果");
    for (const drop_service::DropVerdict& v : verdicts) {
        TC_CHECK(!v.accepted, "非法项应被拒绝");
        TC_CHECK(!v.reason.isEmpty(), "拒绝原因不能为空");
    }
    TC_CHECK(drop_service::acceptedPaths(verdicts).isEmpty(), "不应有接受项");
    TC_CHECK(drop_service::rejectedReasons(verdicts).size() == 3, "拒绝原因应逐项列出");
}
} // namespace
REGISTER_TEST("TC-V2-F16-04", run);
