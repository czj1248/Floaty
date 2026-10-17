// TC-V2-F15-01 多显示器所在屏幕判定
// 点落在哪个屏幕几何内返回对应下标；都不包含返回 -1；
// 边缘重叠时选包含该点的最小屏幕
#include <QList>
#include <QRect>

#include "test_helpers.h"
#include "window_layout.h"

namespace {
void run()
{
    // 主屏 + 右侧外接屏，坐标相邻不重叠
    const QList<QRect> sideBySide = {QRect(0, 0, 1920, 1080), QRect(1920, 0, 800, 600)};
    TC_CHECK(window_layout::screenIndexAt(sideBySide, QPoint(100, 100)) == 0,
             "主屏内的点应返回 0");
    TC_CHECK(window_layout::screenIndexAt(sideBySide, QPoint(2000, 300)) == 1,
             "外接屏内的点应返回 1");
    TC_CHECK(window_layout::screenIndexAt(sideBySide, QPoint(3000, 300)) == -1,
             "不在任何屏幕内应返回 -1");

    // 重叠区域：大屏内嵌一块更小的屏，重叠点应归属小屏
    const QList<QRect> overlapping = {QRect(0, 0, 1000, 1000), QRect(900, 900, 200, 200)};
    TC_CHECK(window_layout::screenIndexAt(overlapping, QPoint(950, 950)) == 1,
             "重叠区域的点应归属面积更小的屏幕");
    TC_CHECK(window_layout::screenIndexAt(overlapping, QPoint(100, 100)) == 0,
             "仅大屏包含的点应返回 0");
}
} // namespace
REGISTER_TEST("TC-V2-F15-01", run);
