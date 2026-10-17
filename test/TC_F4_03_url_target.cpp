// TC-F4-03 添加网址快捷项（自动化部分）
// 覆盖：http:// 与 https:// 开头的输入识别为网址；
// 不带 http(s) 前缀的输入不被静默接受（按规则拒绝），非 http(s) 协议同样拒绝。
// 输入框交互与默认浏览器打开为人工步骤。
#include "test_helpers.h"
#include "types.h"

namespace {
void run()
{
    TC_CHECK(detectTargetType(QStringLiteral("https://example.com"))
                 == std::optional<TargetType>(TargetType::Url),
             "https 网址应识别为网址类型");
    TC_CHECK(detectTargetType(QStringLiteral("http://example.com"))
                 == std::optional<TargetType>(TargetType::Url),
             "http 网址应识别为网址类型");
    TC_CHECK(detectTargetType(QStringLiteral("HTTP://EXAMPLE.COM"))
                 == std::optional<TargetType>(TargetType::Url),
             "协议前缀应不区分大小写");

    TC_CHECK(!detectTargetType(QStringLiteral("www.example.com")).has_value(),
             "无协议前缀的输入应被拒绝");
    TC_CHECK(!detectTargetType(QStringLiteral("example.com")).has_value(),
             "无协议前缀的输入应被拒绝");
    TC_CHECK(!detectTargetType(QStringLiteral("ftp://example.com")).has_value(),
             "非 http(s) 协议应被拒绝");
    TC_CHECK(!detectTargetType(QStringLiteral("file:///C:/x.txt")).has_value(),
             "file 协议应被拒绝");
}
} // namespace
REGISTER_TEST("TC-F4-03", run);
