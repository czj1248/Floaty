#pragma once

#include <QList>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>

// 窗口几何计算：纯函数，便于自动化测试（TC-F1-03 / TC-F2-05 / F14 / F15）
namespace window_layout {

// 展开面板左上角：优先在悬浮球上方展开，空间不足时改为下方；
// 水平方向以悬浮球为中心，整体夹取在屏幕可视范围内
QPoint calcPanelTopLeft(const QRect& ball, const QSize& panel, const QRect& screen, int gap);

// 将悬浮球位置夹取在屏幕可视范围内，保证球不会被拖到完全看不见
QPoint clampBallTopLeft(const QPoint& pos, const QSize& ball, const QRect& screen);

// 首次启动的默认位置：屏幕右侧、竖直居中
QPoint defaultBallTopLeft(const QSize& ball, const QRect& screen);

// ---- F14 贴边吸附 ----
// 松开鼠标时调用：距屏幕任意边小于 threshold 则该方向吸附（留 gap 间隙）。
// 水平与竖直独立判定，因此四角会双向吸附；中部位置不修正。
QPoint snapToEdge(const QPoint& pos, const QSize& ball, const QRect& screen,
                  int threshold, int gap);

// ---- F12 面板宽度自适应 ----
// contentWidth 为最宽内容行宽（含行左右各 2px 边距），
// 加容器左右各 12px 内边距后夹取到 [minWidth, maxWidth]；
// 宽度与内容高度无关（垂直滚动条悬浮不占位）
int calcPanelWidth(int contentWidth, int minWidth, int maxWidth);
// 行文字可用宽度：面板宽减去容器内边距（12*2）、行左右边距（2*2）、图标与间距
int rowTextAvailableWidth(int panelWidth, int iconWidth);
// 文字是否超出可用宽度（超出才需要悬停跑马灯）
bool needMarquee(int textWidth, int availableWidth);

// ---- F15 多显示器 ----
// 返回全局坐标点所在屏幕在屏幕列表中的下标；不在任何屏幕内返回 -1。
// 点同时落在多个屏幕（边缘重叠）时选包含它的最小屏幕。
int screenIndexAt(const QList<QRect>& screens, const QPoint& globalPos);

} // namespace window_layout
