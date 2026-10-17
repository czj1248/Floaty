// TC-F2-05 展开方向（自动化部分）
// 覆盖：屏幕中部在悬浮球上方展开；靠近屏幕上缘时改为向下展开；
// 面板完整显示在屏幕内、不越出左右边界。
// 拖动悬浮球到目标位置的操作为人工步骤。
#include "test_helpers.h"
#include "window_layout.h"

namespace {
void run()
{
    const QRect screen(0, 0, 1920, 1080);
    const QSize panel(260, 200);
    const int gap = 8;

    // 屏幕中部偏下：上方空间充足，面板应在悬浮球上方
    const QRect ballMid(QPoint(900, 700), QSize(48, 48));
    const QPoint posMid = window_layout::calcPanelTopLeft(ballMid, panel, screen, gap);
    TC_CHECK(posMid.y() + panel.height() == ballMid.top() - gap, "中部位置面板应在球上方");
    TC_CHECK(posMid.y() >= screen.top(), "面板不得超出屏幕上缘");

    // 屏幕顶部附近：上方放不下，改为向下展开且完整可见
    const QRect ballTop(QPoint(900, 0), QSize(48, 48));
    const QPoint posTop = window_layout::calcPanelTopLeft(ballTop, panel, screen, gap);
    TC_CHECK(posTop.y() == ballTop.bottom() + 1 + gap, "顶部位置面板应改为向下展开");
    TC_CHECK(posTop.y() + panel.height() <= screen.bottom() + 1, "向下展开面板应完整显示在屏幕内");

    // 球在左边缘：面板左缘不越出屏幕
    const QRect ballLeft(QPoint(0, 500), QSize(48, 48));
    const QPoint posLeft = window_layout::calcPanelTopLeft(ballLeft, panel, screen, gap);
    TC_CHECK(posLeft.x() == screen.left(), "球靠左边缘时面板左缘不得越界");

    // 球在右边缘：面板右缘不越出屏幕
    const QRect ballRight(QPoint(1920 - 48, 500), QSize(48, 48));
    const QPoint posRight = window_layout::calcPanelTopLeft(ballRight, panel, screen, gap);
    TC_CHECK(posRight.x() + panel.width() == screen.right() + 1, "球靠右边缘时面板右缘不得越界");

    // 水平方向以球为中心
    const QRect ballCenter(QPoint(900, 500), QSize(48, 48));
    const QPoint posCenter = window_layout::calcPanelTopLeft(ballCenter, panel, screen, gap);
    TC_CHECK(posCenter.x() == ballCenter.center().x() - panel.width() / 2, "面板应以悬浮球水平居中");
}
} // namespace
REGISTER_TEST("TC-F2-05", run);
