// TC-F4-02 添加文件类快捷项（自动化部分）
// 覆盖：类型按扩展名自动识别——.exe/.lnk 为程序，.bat/.cmd 为脚本，
// 文档类白名单全部 25 种扩展名识别为文档，扩展名不区分大小写。
// 文件对话框选择、名称输入与图标显示为人工步骤。
#include "test_helpers.h"
#include "types.h"

namespace {
void run()
{
    const QString dir = makeTempDir("TC_F4_02");

    // 程序类
    for (const char* ext : { "exe", "lnk" }) {
        const auto type = detectTargetType(dir + "/app." + ext);
        TC_CHECK(type.has_value() && *type == TargetType::Program,
                 std::string("应为程序类型: ") + ext);
    }
    // 脚本类
    for (const char* ext : { "bat", "cmd" }) {
        const auto type = detectTargetType(dir + "/run." + ext);
        TC_CHECK(type.has_value() && *type == TargetType::Script,
                 std::string("应为脚本类型: ") + ext);
    }
    // 文档类：白名单全部 25 种扩展名
    const char* docs[] = {
        "txt", "md", "pdf", "doc", "docx", "xls", "xlsx", "ppt", "pptx",
        "jpg", "jpeg", "png", "gif", "bmp", "webp",
        "mp3", "wav", "flac", "mp4", "mkv", "avi", "mov",
        "zip", "rar", "7z"
    };
    for (const char* ext : docs) {
        const auto type = detectTargetType(dir + "/doc." + ext);
        TC_CHECK(type.has_value() && *type == TargetType::Document,
                 std::string("应为文档类型: ") + ext);
    }
    // 扩展名不区分大小写
    TC_CHECK(detectTargetType(dir + "/PIC.PNG").has_value(), "扩展名应不区分大小写");
}
} // namespace
REGISTER_TEST("TC-F4-02", run);
