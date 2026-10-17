// NF-03 单实例（自动化部分）
// 覆盖：同一互斥体的第二次获取判定为"已有实例在运行"。
// 注意：本用例要求 Floaty 主程序未在运行（与用例前置条件一致），
// 否则首个断言会因主程序持有互斥体而失败。
// 双击两个实例只出现一个悬浮球为人工观察项。
#include "single_instance.h"
#include "test_helpers.h"

namespace {
void run()
{
    TC_CHECK(acquireSingleInstance(), "首个实例应成功获取互斥体");
    TC_CHECK(!acquireSingleInstance(), "第二次获取应判定已有实例在运行");
}
} // namespace
REGISTER_TEST("NF-03", run);
