#include "ball_logo.h"

#include <QIcon>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPixMap>

namespace ball_logo {

// 设计坐标系为 48x48，以下路径与渐变坐标均基于该坐标系
namespace {

// 叶片轮廓：叶根左下、叶尖右上，外轮廓为平滑曲线
QPainterPath buildLeafPath()
{
    QPainterPath leaf;
    leaf.moveTo(11.5, 38.5);
    leaf.cubicTo(17.6, 42.5, 25.0, 40.0, 28.5, 32.8);
    leaf.cubicTo(33.0, 26.2, 35.8, 19.2, 37.8, 10.0);
    leaf.cubicTo(32.3, 10.6, 24.5, 12.3, 19.5, 16.8);
    leaf.cubicTo(13.5, 22.0, 10.2, 28.3, 11.5, 38.5);
    leaf.closeSubpath();
    return leaf;
}

// 梗：外段短柄与内段中脉合并为单一路径（厚度剖面 柄端0.8 -> 根部1.6 -> 尖部0.4）
QPainterPath buildStemPath()
{
    QPainterPath stem;
    stem.moveTo(9.10, 41.87);
    stem.quadTo(10.32, 40.72, 10.93, 40.09);
    stem.quadTo(11.54, 39.45, 11.93, 38.91);
    stem.quadTo(12.31, 38.36, 12.85, 37.84);
    stem.quadTo(13.17, 37.53, 13.69, 36.49);
    stem.quadTo(14.20, 35.45, 15.53, 33.56);
    stem.quadTo(16.86, 31.67, 18.39, 29.77);
    stem.quadTo(19.91, 27.87, 21.74, 25.92);
    stem.quadTo(23.56, 23.97, 25.64, 21.87);
    stem.quadTo(27.72, 19.77, 29.95, 17.55);
    stem.quadTo(32.17, 15.33, 34.41, 12.99);
    stem.lineTo(36.64, 10.64);
    stem.lineTo(36.36, 10.36);
    stem.quadTo(31.75, 14.91, 29.46, 17.06);
    stem.quadTo(27.16, 19.21, 25.03, 21.26);
    stem.quadTo(22.90, 23.31, 21.00, 25.22);
    stem.quadTo(19.09, 27.13, 17.50, 29.04);
    stem.quadTo(15.90, 30.95, 14.52, 32.83);
    stem.quadTo(13.14, 34.71, 12.57, 35.71);
    stem.quadTo(11.99, 36.71, 11.64, 36.99);
    stem.quadTo(11.05, 37.46, 10.66, 37.98);
    stem.quadTo(10.26, 38.49, 9.81, 39.22);
    stem.quadTo(9.36, 39.94, 8.93, 40.64);
    stem.lineTo(8.50, 41.33);
    stem.closeSubpath();
    return stem;
}

} // namespace

void paintBallLogo(QPainter* p, const QRectF& rect)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);

    // 等比缩放：48x48 设计坐标系映射到目标正方形（居中）
    const qreal side = qMin(rect.width(), rect.height());
    const QRectF r(rect.center().x() - side / 2, rect.center().y() - side / 2, side, side);
    p->translate(r.topLeft());
    p->scale(side / 48.0, side / 48.0);

    p->setPen(Qt::NoPen);

    // 叶片：叶根深绿 -> 中部竹绿 -> 叶尖浅绿
    QLinearGradient leafGrad(QPointF(11.5, 38.5), QPointF(37.8, 10.0));
    leafGrad.setColorAt(0.0, QColor(0x1E, 0x5F, 0x44));
    leafGrad.setColorAt(0.45, QColor(0x2E, 0x8B, 0x6B));
    leafGrad.setColorAt(1.0, QColor(0x8A, 0xD3, 0xA8));
    p->setBrush(leafGrad);
    p->drawPath(buildLeafPath());

    // 梗：叶尖->柄端方向渐变，0%~66% 恒定纯白，66%~92% 根部区域渐变为深绿，
    // 92% 以后（叶外短柄）恒定深绿，保证浅色背景下短柄可见
    QLinearGradient stemGrad(QPointF(36.5, 10.5), QPointF(8.8, 41.6));
    stemGrad.setColorAt(0.0, QColor(0xFF, 0xFF, 0xFF));
    stemGrad.setColorAt(0.66, QColor(0xFF, 0xFF, 0xFF));
    stemGrad.setColorAt(0.92, QColor(0x1E, 0x5F, 0x44));
    stemGrad.setColorAt(1.0, QColor(0x1E, 0x5F, 0x44));
    p->setBrush(stemGrad);
    p->drawPath(buildStemPath());

    p->restore();
}

QIcon brandIcon()
{
    QIcon icon;
    // 多尺寸绘制，标题栏/任务栏/Alt-Tab 各取最清晰的一档
    const int sizes[] = { 16, 24, 32, 48 };
    for (int side : sizes) {
        QPixmap pm(side, side);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        paintBallLogo(&p, QRectF(0, 0, side, side));
        p.end();
        icon.addPixmap(pm);
    }
    return icon;
}

} // namespace ball_logo
