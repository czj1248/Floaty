// TC-F4-05 添加白名单外类型被拒绝（自动化部分）
// 覆盖：.reg/.msi/.ps1/.vbs/.js/.scr 等白名单外扩展名不被支持，
// 白名单内代表类型正常通过。管理窗口的拒绝提示框与列表不变为人工步骤。
#include "test_helpers.h"
#include "types.h"

namespace {
void run()
{
    const QString dir = makeTempDir("TC_F4_05");

    // 白名单外类型一律拒绝
    const char* rejected[] = { "reg", "msi", "ps1", "vbs", "js", "scr", "dll", "iso" };
    for (const char* ext : rejected) {
        const QString path = dir + "/test." + ext;
        TC_CHECK(writeTextFile(path, "x"), std::string("测试文件应写入成功: ") + ext);
        TC_CHECK(!isSupportedTarget(path),
                 (QStringLiteral("不应支持类型 .%1").arg(QLatin1String(ext))).toStdString());
    }

    // 白名单内代表类型正常通过
    TC_CHECK(isSupportedTarget(dir + "/a.exe"), "exe 应在白名单");
    TC_CHECK(isSupportedTarget(dir + "/a.lnk"), "lnk 应在白名单");
    TC_CHECK(isSupportedTarget(dir + "/a.bat"), "bat 应在白名单");
    TC_CHECK(isSupportedTarget(dir + "/a.cmd"), "cmd 应在白名单");
    TC_CHECK(isSupportedTarget(dir + "/a.txt"), "txt 应在白名单");
    TC_CHECK(isSupportedTarget(dir + "/a.pdf"), "pdf 应在白名单");
    TC_CHECK(isSupportedTarget(dir + "/a.mp4"), "mp4 应在白名单");
    TC_CHECK(isSupportedTarget(dir + "/a.7z"), "7z 应在白名单");
}
} // namespace
REGISTER_TEST("TC-F4-05", run);
