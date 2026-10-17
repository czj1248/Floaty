// TC-F3-09 启动目标不存在时的提示（自动化部分）
// 覆盖：目标已被移动或删除时返回"目标不存在"结果，提示文案包含失效路径，
// 进程不崩溃不退出（运行至用例结束即未崩溃）。
// 界面提示框的弹出与悬浮球存活为人工观察项。
#include "launch_service.h"
#include "test_helpers.h"

namespace {
void run()
{
    const QString dir = makeTempDir("TC_F3_09");
    LaunchService launcher;

    QuickItem missingFile;
    missingFile.name = QStringLiteral("已失效项");
    missingFile.target = dir + "/removed_note.txt";
    missingFile.type = TargetType::Document;
    TC_CHECK(launcher.launch(missingFile) == LaunchResult::TargetMissing,
             "不存在的文件应返回目标不存在");

    QuickItem missingFolder;
    missingFolder.name = QStringLiteral("已失效文件夹");
    missingFolder.target = dir + "/removed_folder";
    missingFolder.type = TargetType::Folder;
    TC_CHECK(launcher.launch(missingFolder) == LaunchResult::TargetMissing,
             "不存在的文件夹应返回目标不存在");

    // 提示文案应包含目标路径，便于用户定位问题
    const QString msg = LaunchService::failureMessage(LaunchResult::TargetMissing, missingFile);
    TC_CHECK(msg.contains(missingFile.target), "提示应包含失效目标的路径");
    TC_CHECK(msg.contains(QStringLiteral("目标不存在")), "提示应包含目标不存在说明");
}
} // namespace
REGISTER_TEST("TC-F3-09", run);
