// TC-V2-F12-03 面板宽度上限（V2.1：上限取外观设置值，默认 320）
// 最宽内容（含行缘）超过 maxWidth - 24 时，面板宽度封顶 maxWidth
#include "test_helpers.h"
#include "theme.h"
#include "window_layout.h"

namespace {
void run()
{
    TC_CHECK(window_layout::calcPanelWidth(1000, theme::kPanelMinWidth,
                                           theme::kDefaultPanelMaxWidth)
                 == 320,
             "默认设置下超长内容应封顶到 320");
    // 临界点：内容 296 + 24 = 320
    TC_CHECK(window_layout::calcPanelWidth(296, theme::kPanelMinWidth,
                                           theme::kDefaultPanelMaxWidth)
                 == 320,
             "内容 296 时恰好为 320");
    TC_CHECK(window_layout::calcPanelWidth(295, theme::kPanelMinWidth,
                                           theme::kDefaultPanelMaxWidth)
                 == 319,
             "内容 295 时应为 319（未封顶）");

    // 封顶后行文字可用宽度有限，超长名进入跑马灯判定
    const int avail = window_layout::rowTextAvailableWidth(320, 20);
    TC_CHECK(avail == 264, "最大面板下默认口径行文字可用宽度应为 264");
    TC_CHECK(window_layout::needMarquee(400, avail), "超出可用宽度的名称需要跑马灯");

    // 上限跟随设置值：68 下限与 480 上限（与 TC-V2-F13-05 联动）
    TC_CHECK(window_layout::calcPanelWidth(1000, theme::kPanelMinWidth,
                                           theme::kPanelMaxWidthMin)
                 == theme::kPanelMaxWidthMin,
             "最大宽度设为下限时应封顶到下限");
    TC_CHECK(window_layout::calcPanelWidth(1000, theme::kPanelMinWidth, 480) == 480,
             "最大宽度设为 480 时应封顶到 480");
}
} // namespace
REGISTER_TEST("TC-V2-F12-03", run);
