#include "floating_ball.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QGuiApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPolygon>
#include <QScreen>

#include "config_store.h"
#include "expand_panel.h"
#include "manager_dialog.h"
#include "window_layout.h"

FloatingBall::FloatingBall(ConfigStore& store, QWidget* parent)
    : QWidget(parent)
    , m_store(store)
    , m_panel(new ExpandPanel(store, this))
{
    setObjectName(QStringLiteral("floatingBall"));
    setFixedSize(ballSize());
    // 无边框、置顶、不进任务栏、点击不夺走键盘焦点（F1）
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                   | Qt::Tool | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setCursor(Qt::PointingHandCursor);

    // 恢复上次位置，并夹取在当前屏幕内
    const QRect scr = QGuiApplication::primaryScreen()->availableGeometry();
    move(window_layout::clampBallTopLeft(m_store.ballPosition(), ballSize(), scr));
}

void FloatingBall::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF r = rect().adjusted(2.0, 2.0, -2.0, -2.0);

    // 球体：左上高光的径向渐变圆
    QRadialGradient grad(r.center() + QPointF(-r.width() * 0.15, -r.height() * 0.15),
                         r.width() * 0.9);
    grad.setColorAt(0.0, QColor(0x7f, 0xbb, 0xff));
    grad.setColorAt(1.0, QColor(0x15, 0x63, 0xc0));
    p.setPen(Qt::NoPen);
    p.setBrush(grad);
    p.drawEllipse(r);

    // 中央向上箭头，示意向上展开
    const QPointF c = r.center();
    const qreal s = r.width() / 48.0;
    QPolygonF arrow;
    arrow << QPointF(c.x(), c.y() - 9 * s)
          << QPointF(c.x() - 8 * s, c.y() + 1 * s)
          << QPointF(c.x() - 3 * s, c.y() + 1 * s)
          << QPointF(c.x() - 3 * s, c.y() + 9 * s)
          << QPointF(c.x() + 3 * s, c.y() + 9 * s)
          << QPointF(c.x() + 3 * s, c.y() + 1 * s)
          << QPointF(c.x() + 8 * s, c.y() + 1 * s);
    p.setBrush(Qt::white);
    p.drawPolygon(arrow);
}

void FloatingBall::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton) {
        return; // 右键走 contextMenuEvent，不参与拖动与单击逻辑
    }
    m_pressed = true;
    m_dragging = false;
    m_pressGlobal = event->globalPosition().toPoint();
    m_pressWindowPos = pos();
}

void FloatingBall::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_pressed || !(event->buttons() & Qt::LeftButton)) {
        return;
    }
    const QPoint delta = event->globalPosition().toPoint() - m_pressGlobal;
    if (!m_dragging && delta.manhattanLength() <= 6) {
        return; // 位移过小视为按住未拖
    }
    // 拖动跟随（F1），并夹取在屏幕可视范围内（不吸附，仅防止拖出屏幕找不回）
    m_dragging = true;
    const QRect scr = QGuiApplication::primaryScreen()->availableGeometry();
    move(window_layout::clampBallTopLeft(m_pressWindowPos + delta, ballSize(), scr));
}

void FloatingBall::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || !m_pressed) {
        // 忽略非左键释放，以及没有按下与之配对的释放（面板弹出层收起时的穿透释放）
        return;
    }
    const bool wasDragging = m_dragging;
    m_pressed = false;
    m_dragging = false;
    if (wasDragging) {
        savePosition();
    } else {
        togglePanel(); // 单击：展开/收起（F2）
    }
}

void FloatingBall::contextMenuEvent(QContextMenuEvent* event)
{
    // 右键菜单仅含两项（F6），右键不触发展开面板
    QMenu menu(this);
    QAction* manageAction = menu.addAction(QStringLiteral("管理快捷项"));
    QAction* quitAction = menu.addAction(QStringLiteral("退出"));
    QAction* chosen = menu.exec(event->globalPos());
    if (chosen == manageAction) {
        openManager();
    } else if (chosen == quitAction) {
        QApplication::quit();
    }
}

void FloatingBall::togglePanel()
{
    if (m_panel->isVisible()) {
        m_panel->close();
        return;
    }
    m_panel->popupForBall(geometry());
}

void FloatingBall::openManager()
{
    if (!m_manager) {
        m_manager = new ManagerDialog(m_store, this);
        // 快捷项变更后，若面板处于展开状态则同步刷新
        connect(m_manager, &ManagerDialog::itemsChanged, this, [this]() {
            if (m_panel->isVisible()) {
                m_panel->reload();
            }
        });
    }
    m_manager->show();
    m_manager->raise();
    m_manager->activateWindow();
}

void FloatingBall::savePosition()
{
    m_store.setBallPosition(pos());
    m_store.save();
}
