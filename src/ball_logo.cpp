#include "ball_logo.h"

#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QPixMap>
#include <QRadialGradient>

#include "theme.h"

namespace ball_logo {

void paintBallLogo(QPainter* p, const QRectF& rect)
{
    p->save();
    p->setRenderHint(QPainter::Antialiasing);

    const qreal side = qMin(rect.width(), rect.height());
    const QRectF r(rect.center().x() - side / 2, rect.center().y() - side / 2, side, side);
    const qfloat16 scale = static_cast<qfloat16>(side / 48.0);

    // 球体：左上方高光的竹绿径向渐变
    QRadialGradient grad(r.center() + QPointF(-side * 0.15, -side * 0.18), side * 0.95);
    grad.setColorAt(0.0, theme::kBallHighlight);
    grad.setColorAt(1.0, theme::kBallBody);
    p->setPen(Qt::NoPen);
    p->setBrush(grad);
    p->drawEllipse(r);

    // 内部白色图形：一道自下而上的弧线（浮起的轨迹）
    QPainterPath arc;
    arc.moveTo(r.left() + side * 0.30, r.top() + side * 0.72);
    arc.cubicTo(r.left() + side * 0.26, r.top() + side * 0.46,
                r.left() + side * 0.48, r.top() + side * 0.40,
                r.left() + side * 0.56, r.top() + side * 0.28);
    QPen arcPen(QColor(0xFF, 0xFF, 0xFF, 235));
    arcPen.setWidthF(2.4 * scale);
    arcPen.setCapStyle(Qt::RoundCap);
    p->setPen(arcPen);
    p->setBrush(Qt::NoBrush);
    p->drawPath(arc);

    // 弧线顶端的小叶尖（启动、初启之意）
    p->save();
    p->translate(r.left() + side * 0.60, r.top() + side * 0.25);
    p->rotate(-35);
    p->setPen(Qt::NoPen);
    p->setBrush(QColor(0xFF, 0xFF, 0xFF, 240));
    p->drawEllipse(QRectF(-2.6 * scale, -5.5 * scale, 5.2 * scale, 11.0 * scale));
    p->restore();

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
