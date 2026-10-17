// TC-V2-F12-05 跑马灯触发门控
// 仅文字宽度严格大于可用宽度才跑马灯；等宽与更短均原位显示
#include "test_helpers.h"
#include "window_layout.h"

namespace {
void run()
{
    TC_CHECK(!window_layout::needMarquee(100, 100), "文字与可用宽度等宽不应跑马灯");
    TC_CHECK(!window_layout::needMarquee(80, 100), "短文字不应跑马灯");
    TC_CHECK(window_layout::needMarquee(101, 100), "超出 1px 即应跑马灯");

    // 可用宽度与面板宽度联动：面板越宽，能容纳的文字越多
    TC_CHECK(window_layout::rowTextAvailableWidth(160, 20) <
                 window_layout::rowTextAvailableWidth(320, 20),
             "面板越宽，行文字可用宽度越大");
}
} // namespace
REGISTER_TEST("TC-V2-F12-05", run);
