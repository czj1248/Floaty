// TC-F4-04 添加文件夹快捷项（自动化部分）
// 覆盖：真实存在的文件夹识别为文件夹类型；文件按扩展名识别而非误判为文件夹；
// 不存在的路径不能识别为文件夹。
// 目录对话框选择与文件夹图标显示为人工步骤。
#include <QDir>
#include <QFileInfo>

#include "test_helpers.h"
#include "types.h"

namespace {
void run()
{
    const QString dir = makeTempDir("TC_F4_04");
    const QString sub = dir + "/subfolder";
    TC_CHECK(QDir().mkpath(sub), "测试子文件夹应创建成功");

    const auto type = detectTargetType(sub);
    TC_CHECK(type.has_value() && *type == TargetType::Folder, "存在的文件夹应识别为文件夹类型");

    // 文件按扩展名识别，不误判为文件夹
    TC_CHECK(writeTextFile(dir + "/a.txt", "x"), "测试文件应写入成功");
    const auto fileType = detectTargetType(dir + "/a.txt");
    TC_CHECK(fileType.has_value() && *fileType == TargetType::Document,
             "文件应按扩展名识别而非当作文件夹");

    // 不存在的路径不能识别为文件夹
    TC_CHECK(detectTargetType(dir + "/no_such_dir") != std::optional<TargetType>(TargetType::Folder),
             "不存在的路径不应识别为文件夹");
    TC_CHECK(!QFileInfo::exists(dir + "/no_such_dir"), "前置确认：该路径不应存在");
}
} // namespace
REGISTER_TEST("TC-F4-04", run);
