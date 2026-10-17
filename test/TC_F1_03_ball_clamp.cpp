// TC-F1-03 拖动悬浮球（自动化部分）
// 覆盖"不应发生：悬浮球被拖出屏幕可视范围后无法找回"：拖动位置被夹取在屏幕内；
// 同时覆盖 TC-F1-05 不吸附：夹取仅防止越界，不产生吸附对齐修正。
// 拖动跟随鼠标与松开停留为人工观察项（见 spec 用例）。
#include "test_helpers.h"
#include "window_layout.h"

namespace {
void run()
{
    const QRect screen(0, 0, 1920, 1080);
    const QSize ball(48, 48);

    // 正常位置不改变
    TC_CHECK(window_layout::clampBallTopLeft(QPoint(500, 300), ball, screen) == QPoint(500, 300),
             "正常位置应保持不变");

    // 左上越界被拉回屏幕内
    TC_CHECK(window_layout::clampBallTopLeft(QPoint(-100, -100), ball, screen) == QPoint(0, 0),
             "左上越界应夹取到屏幕原点");

    // 右下越界被拉回，球完整可见
    TC_CHECK(window_layout::clampBallTopLeft(QPoint(5000, 5000), ball, screen)
                 == QPoint(1920 - 48, 1080 - 48),
             "右下越界应夹取到屏幕内");

    // 紧贴边缘是允许的：只夹取越界，不额外修正（无贴边吸附）
    TC_CHECK(window_layout::clampBallTopLeft(QPoint(10, 1080 - 48), ball, screen)
                 == QPoint(10, 1080 - 48),
             "贴下边缘不应被吸附修正");
    TC_CHECK(window_layout::clampBallTopLeft(QPoint(1920 - 48, 200), ball, screen)
                 == QPoint(1920 - 48, 200),
             "贴右边缘不应被吸附修正");
}
} // namespace
REGISTER_TEST("TC-F1-03", run);
