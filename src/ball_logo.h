#pragma once

class QIcon;
class QPainter;
class QRectF;

// 品牌图形（F7）：叶形图标「浮青」。
// 实心绿色渐变叶片 + 白绿渐变叶梗，叶根深绿、叶尖浅绿，梗根部渐变为深绿。
// 悬浮球与窗口图标共用同一绘制函数，保证品牌图形唯一。
namespace ball_logo {

// 在指定矩形内绘制完整品牌图形（矩形应为正方形）
void paintBallLogo(QPainter* painter, const QRectF& rect);

// 由品牌图形生成的多尺寸窗口/应用图标（16/24/32/48）
QIcon brandIcon();

} // namespace ball_logo
