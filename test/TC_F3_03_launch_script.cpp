// TC-F3-03 双击脚本类快捷项（.bat / .cmd）（自动化部分）
// 覆盖：脚本被系统默认方式执行，标记文件被创建（脚本确实运行而非被当作文本打开）。
// "与资源管理器双击行为一致"的人工对照与面板收起观察见 spec 用例。
#include <QFileInfo>

#include "launch_service.h"
#include "test_helpers.h"

namespace {
void run()
{
    const QString dir = makeTempDir("TC_F3_03");
    const QString marker = dir + "/marker_tc_f3_03.txt";
    const QString content =
        QStringLiteral("@echo off\r\necho executed> \"%1\"\r\n").arg(QDir::toNativeSeparators(marker));

    for (const QString ext : { QStringLiteral("bat"), QStringLiteral("cmd") }) {
        if (QFileInfo::exists(marker)) {
            QFile::remove(marker);
        }
        const QString script = dir + "/hello." + ext;
        TC_CHECK(writeTextFile(script, content), "脚本文件应写入成功");

        QuickItem item;
        item.name = QStringLiteral("测试脚本");
        item.target = script;
        item.type = TargetType::Script;

        const LaunchResult result = LaunchService().launch(item);
        TC_CHECK(result == LaunchResult::Success, "脚本启动应返回成功");
        TC_CHECK(waitFor([&]() { return QFileInfo::exists(marker); }, 8000),
                 "脚本应被执行并生成标记文件");
    }
}
} // namespace
REGISTER_TEST("TC-F3-03", run);
