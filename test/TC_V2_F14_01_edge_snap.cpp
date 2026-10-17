// TC-V2-F14-01 松开贴边吸附（含四角双向吸附）
// 距边 <=24px 吸附并留 8px 间隙；中部不修正；拖动跟随不跳由人工用例覆盖
#include "test_helpers.h"
#include "theme.h"
#include "window_layout.h"

namespace {
void run()
{
    const QRect screen(0, 0, 1920, 1080);
    const QSize ball(48, 48);
    const int threshold = theme::kSnapDistance; // 24
    const int gap = theme::kSnapGap;             // 8

    // 中部：四个方向都不吸附
    const QPoint center =
        window_layout::snapToEdge(QPoint(500, 400), ball, screen, threshold, gap);
    TC_CHECK(center == QPoint(500, 400), "屏幕中部不应被吸附修正");

    // 左边缘内 24px：吸到左边留 8px
    TC_CHECK(window_layout::snapToEdge(QPoint(20, 400), ball, screen, threshold, gap)
                 == QPoint(gap, 400),
             "靠近左边缘应吸附到 x=8");
    // 阈值外 1px 不吸附
    TC_CHECK(window_layout::snapToEdge(QPoint(25, 400), ball, screen, threshold, gap)
                 == QPoint(25, 400),
             "距边 25px 不应吸附");
    // 右边缘
    TC_CHECK(window_layout::snapToEdge(QPoint(1920 - 48 - 20, 400), ball, screen, threshold, gap)
                 == QPoint(1920 - 48 - gap, 400),
             "靠近右边缘应吸附到右侧留 8px");
    // 上、下边缘
    TC_CHECK(window_layout::snapToEdge(QPoint(500, 20), ball, screen, threshold, gap)
                 == QPoint(500, gap),
             "靠近上边缘应吸附到 y=8");
    TC_CHECK(window_layout::snapToEdge(QPoint(500, 1080 - 48 - 20), ball, screen, threshold, gap)
                 == QPoint(500, 1080 - 48 - gap),
             "靠近下边缘应吸附到底侧留 8px");

    // 左上角：水平与竖直同时吸附（四角双向）
    TC_CHECK(window_layout::snapToEdge(QPoint(20, 20), ball, screen, threshold, gap)
                 == QPoint(gap, gap),
             "左上角应双向吸附到 (8,8)");
    // 右下角
    TC_CHECK(window_layout::snapToEdge(QPoint(1920 - 48 - 20, 1080 - 48 - 20), ball, screen,
                                       threshold, gap)
                 == QPoint(1920 - 48 - gap, 1080 - 48 - gap),
             "右下角应双向吸附");
}
} // namespace
REGISTER_TEST("TC-V2-F14-01", run);
