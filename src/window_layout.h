#pragma once

#include <QPoint>
#include <QRect>
#include <QSize>

// 窗口几何计算：纯函数，便于自动化测试（TC-F1-03 / TC-F2-05）
namespace window_layout {

// 展开面板左上角：优先在悬浮球上方展开，空间不足时改为下方；
// 水平方向以悬浮球为中心，整体夹取在屏幕可视范围内
QPoint calcPanelTopLeft(const QRect& ball, const QSize& panel, const QRect& screen, int gap);

// 将悬浮球位置夹取在屏幕可视范围内，保证球不会被拖到完全看不见（不做贴边吸附）
QPoint clampBallTopLeft(const QPoint& pos, const QSize& ball, const QRect& screen);

// 首次启动的默认位置：屏幕右侧、竖直居中
QPoint defaultBallTopLeft(const QSize& ball, const QRect& screen);

} // namespace window_layout
