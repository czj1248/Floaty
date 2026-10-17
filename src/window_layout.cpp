#include "window_layout.h"

namespace window_layout {

QPoint calcPanelTopLeft(const QRect& ball, const QSize& panel, const QRect& screen, int gap)
{
    // 水平：以悬浮球水平中心为面板中心，再夹取进屏幕
    int x = ball.center().x() - panel.width() / 2;
    if (x < screen.left()) {
        x = screen.left();
    }
    if (x + panel.width() > screen.right() + 1) {
        x = screen.right() + 1 - panel.width();
    }

    // 垂直：悬浮球上方放得下则向上展开，否则改为向下展开
    int y = ball.top() - gap - panel.height();
    if (y < screen.top()) {
        y = ball.bottom() + 1 + gap;
    }
    // 向下展开也夹取一次，保证面板完整显示在屏幕内
    if (y + panel.height() > screen.bottom() + 1) {
        y = screen.bottom() + 1 - panel.height();
    }
    return QPoint(x, y);
}

QPoint clampBallTopLeft(const QPoint& pos, const QSize& ball, const QRect& screen)
{
    const int maxX = screen.right() + 1 - ball.width();
    const int maxY = screen.bottom() + 1 - ball.height();
    int x = pos.x();
    int y = pos.y();
    if (x < screen.left()) {
        x = screen.left();
    }
    if (x > maxX) {
        x = maxX;
    }
    if (y < screen.top()) {
        y = screen.top();
    }
    if (y > maxY) {
        y = maxY;
    }
    return QPoint(x, y);
}

QPoint defaultBallTopLeft(const QSize& ball, const QRect& screen)
{
    const int x = screen.right() + 1 - ball.width() - 60;
    const int y = screen.center().y() - ball.height() / 2;
    return clampBallTopLeft(QPoint(x, y), ball, screen);
}

} // namespace window_layout
