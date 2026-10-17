// TC-V2-F12-02 面板宽度自适应（V2.1 精确口径）
// 面板宽 = 12（容器内边距）+ 内容行宽（含行左右各 2px 边距）+ 12；
// 内容很窄时取最小宽 80；宽度与内容高度无关（垂直滚动条悬浮不占位）
#include "test_helpers.h"
#include "theme.h"
#include "window_layout.h"

namespace {
void run()
{
    // 无内容：宽度夹取到最小 80
    TC_CHECK(window_layout::calcPanelWidth(0, theme::kPanelMinWidth,
                                           theme::kDefaultPanelMaxWidth)
                 == 80,
             "空内容应取最小宽度 80");

    // 内容 200：200 + 24 = 224
    TC_CHECK(window_layout::calcPanelWidth(200, theme::kPanelMinWidth,
                                           theme::kDefaultPanelMaxWidth)
                 == 224,
             "面板宽度应为内容宽（含行缘）加两侧 12px 内边距");

    // 行文字可用宽 = 面板宽 - 容器内边距 24 - 行左右缘 4 - 图标 20 - 间距 8
    TC_CHECK(window_layout::rowTextAvailableWidth(224, 20) == 168,
             "行文字可用宽度应为 168（扣掉行左右缘各 2px）");

    // 快捷项内容口径：2+20+8+100+2 = 132，加 24 = 156，超过最小宽 80
    constexpr int kRowContent = 2 + 20 + 8 + 100 + 2;
    TC_CHECK(window_layout::calcPanelWidth(kRowContent, theme::kPanelMinWidth,
                                           theme::kDefaultPanelMaxWidth)
                 == 156,
             "短名称行（内容 132）宽度应为 156");

    // 分区标题内容口径：2+180+2 = 184，加 24 = 208
    TC_CHECK(window_layout::calcPanelWidth(2 + 180 + 2, theme::kPanelMinWidth,
                                           theme::kDefaultPanelMaxWidth)
                 == 208,
             "分区标题行宽度应为 208");
}
} // namespace
REGISTER_TEST("TC-V2-F12-02", run);
