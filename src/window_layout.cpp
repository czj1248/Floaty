#include "window_layout.h"

#include <algorithm>

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

QPoint snapToEdge(const QPoint& pos, const QSize& ball, const QRect& screen,
                  int threshold, int gap)
{
    QPoint result = pos;
    const int leftDist = pos.x() - screen.left();
    const int rightDist = screen.right() + 1 - (pos.x() + ball.width());
    const int topDist = pos.y() - screen.top();
    const int bottomDist = screen.bottom() + 1 - (pos.y() + ball.height());

    // 水平方向：离哪边近且进入阈值范围就吸附哪边，中部保持不动
    if (leftDist <= threshold) {
        result.setX(screen.left() + gap);
    } else if (rightDist <= threshold) {
        result.setX(screen.right() + 1 - ball.width() - gap);
    }
    // 竖直方向独立判定，与水平组合即四角吸附
    if (topDist <= threshold) {
        result.setY(screen.top() + gap);
    } else if (bottomDist <= threshold) {
        result.setY(screen.bottom() + 1 - ball.height() - gap);
    }
    return result;
}

int calcPanelWidth(int contentWidth, int minWidth, int maxWidth)
{
    constexpr int kHorizontalPadding = 12; // 容器左右各 12px
    return std::clamp(contentWidth + kHorizontalPadding * 2, minWidth, maxWidth);
}

int rowTextAvailableWidth(int panelWidth, int iconWidth)
{
    constexpr int kHorizontalPadding = 12; // 容器左右各 12px
    constexpr int kRowEdge = 2;           // 行左右各 2px 边距
    constexpr int kIconTextGap = 8;
    return panelWidth - kHorizontalPadding * 2 - kRowEdge * 2 - iconWidth - kIconTextGap;
}

bool needMarquee(int textWidth, int availableWidth)
{
    return textWidth > availableWidth;
}

int screenIndexAt(const QList<QRect>& screens, const QPoint& globalPos)
{
    int result = -1;
    long bestArea = 0;
    for (int i = 0; i < screens.size(); ++i) {
        if (screens.at(i).contains(globalPos)) {
            const long area = static_cast<long>(screens.at(i).width()) * screens.at(i).height();
            // 屏幕边缘可能重叠，取包含该点的最小屏幕
            if (result == -1 || area < bestArea) {
                result = i;
                bestArea = area;
            }
        }
    }
    return result;
}

} // namespace window_layout
