#include "floating_ball.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QCursor>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QEnterEvent>
#include <QGuiApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QMimeData>
#include <QPainter>
#include <QScreen>
#include <QTimer>
#include <QUrl>
#include <QVariantAnimation>

#include "ball_logo.h"
#include "config_store.h"
#include "expand_panel.h"
#include "manager_dialog.h"
#include "quick_add.h"
#include "settings_dialog.h"
#include "theme.h"
#include "window_layout.h"

QSize FloatingBall::sizeForDiameter(int diameter)
{
    if (diameter <= theme::kBallSizeSmall) {
        diameter = theme::kBallSizeSmall;
    } else if (diameter >= theme::kBallSizeLarge) {
        diameter = theme::kBallSizeLarge;
    } else {
        diameter = theme::kBallSizeStandard;
    }
    return QSize(diameter, diameter);
}

FloatingBall::FloatingBall(ConfigStore& store, QWidget* parent)
    : QWidget(parent)
    , m_store(store)
    , m_panel(new ExpandPanel(store, this))
{
    setObjectName(QStringLiteral("floatingBall"));
    setFixedSize(sizeForDiameter(m_store.appearance().ballSize));
    // 无边框、置顶、不进任务栏、点击不夺走键盘焦点（F1）
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                   | Qt::Tool | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setCursor(Qt::PointingHandCursor);
    setAcceptDrops(true);

    // F15：恢复到配置记录的显示器；该显示器不存在时回主屏默认位置
    QScreen* screen = restoreScreen();
    const QRect scr = screen->availableGeometry();
    if (m_store.hasBallPosition()) {
        move(window_layout::clampBallTopLeft(m_store.ballPosition(), size(), scr));
    } else {
        move(window_layout::defaultBallTopLeft(size(), scr));
    }
}

void FloatingBall::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // F7：品牌图形与管理窗口品牌头同源；m_hoverProgress 驱动悬停浅绿高亮
    const QRectF r = rect().adjusted(2.0, 2.0, -2.0, -2.0);
    ball_logo::paintBallLogo(&p, r, m_hoverProgress);

    // F16：拖放悬停时主色描边高亮
    if (m_dropHover) {
        p.setBrush(Qt::NoBrush);
        QPen pen(theme::kPrimary, 2);
        p.setPen(pen);
        p.drawEllipse(rect().adjusted(1, 1, -1, -1));
    }
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
    // F14：拖动过程严格跟随，不做吸附跳变
    m_dragging = true;
    QScreen* screen = screenAtCursor();
    const QRect scr = screen->availableGeometry();
    move(window_layout::clampBallTopLeft(m_pressWindowPos + delta, size(), scr));
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
        // F14：松开时贴边吸附；F15：以球心所在屏幕为准
        QScreen* screen = QGuiApplication::screenAt(geometry().center());
        if (!screen) {
            screen = QGuiApplication::primaryScreen();
        }
        const QRect scr = screen->availableGeometry();
        move(window_layout::snapToEdge(pos(), size(), scr,
                                       theme::kSnapDistance, theme::kSnapGap));
        savePosition();
    } else {
        togglePanel(); // 单击：展开/收起（F2）
    }
}

void FloatingBall::contextMenuEvent(QContextMenuEvent* event)
{
    // F8：右键菜单三项，图标自绘，菜单与面板内不出现品牌文字
    QMenu menu(this);
    QAction* manageAction =
        menu.addAction(QIcon(QStringLiteral(":/icons/manage.svg")),
                       QStringLiteral("管理快捷项"));
    QAction* settingsAction =
        menu.addAction(QIcon(QStringLiteral(":/icons/settings.svg")),
                       QStringLiteral("设置"));
    menu.addSeparator();
    QAction* quitAction =
        menu.addAction(QIcon(QStringLiteral(":/icons/quit.svg")),
                       QStringLiteral("退出"));
    QAction* chosen = menu.exec(event->globalPos());
    if (chosen == manageAction) {
        openManager();
    } else if (chosen == settingsAction) {
        openSettings();
    } else if (chosen == quitAction) {
        QApplication::quit();
    }
}

void FloatingBall::dragEnterEvent(QDragEnterEvent* event)
{
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }
    event->acceptProposedAction();
    m_dropHover = true;
    update();
}

void FloatingBall::dragLeaveEvent(QDragLeaveEvent* /*event*/)
{
    m_dropHover = false;
    update();
}

void FloatingBall::dropEvent(QDropEvent* event)
{
    QStringList paths;
    for (const QUrl& url : event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            paths.append(url.toLocalFile());
        }
    }
    m_dropHover = false;
    update();
    if (paths.isEmpty()) {
        event->ignore();
        return;
    }
    event->acceptProposedAction();
    // 拖到悬浮球：一律进入默认分组（F16）
    if (quickAddPaths(this, m_store, paths, QString::fromLatin1(kDefaultGroupId))
        && m_panel->isVisible()) {
        m_panel->reload();
    }
}

void FloatingBall::enterEvent(QEnterEvent* event)
{
    QWidget::enterEvent(event);
    // 悬停进入：叶片渐变为浅绿高亮
    if (!m_hoverAnim) {
        m_hoverAnim = new QVariantAnimation(this);
        m_hoverAnim->setDuration(160);
        m_hoverAnim->setEasingCurve(QEasingCurve::OutCubic);
        connect(m_hoverAnim, &QVariantAnimation::valueChanged, this,
                [this](const QVariant& v) {
                    m_hoverProgress = v.toReal();
                    update();
                });
    }
    m_hoverAnim->stop();
    m_hoverAnim->setStartValue(m_hoverProgress);
    m_hoverAnim->setEndValue(1.0);
    m_hoverAnim->start();
}

void FloatingBall::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
    // 悬停离开：渐变回常态
    if (m_hoverAnim) {
        m_hoverAnim->stop();
        m_hoverAnim->setStartValue(m_hoverProgress);
        m_hoverAnim->setEndValue(0.0);
        m_hoverAnim->start();
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
    centerOnBallScreen(m_manager);
    m_manager->show();
    m_manager->raise();
    m_manager->activateWindow();
}

void FloatingBall::openSettings()
{
    if (!m_settings) {
        m_settings = new SettingsDialog(m_store, this);
        // 实时预览（未落盘）：三类信号作用域严格分离，互不牵连（F13）
        connect(m_settings, &SettingsDialog::ballSizePreview, this,
                [this](int diameter) { applyAppearance(diameter); });
        connect(m_settings, &SettingsDialog::opacityPreview, m_panel,
                &ExpandPanel::setPreviewOpacity);
        connect(m_settings, &SettingsDialog::panelMaxWidthPreview, m_panel,
                &ExpandPanel::setPreviewMaxWidth);
        connect(m_settings, &QDialog::finished, this, [this](int result) {
            // 结束预览：面板透明度与最大宽度立即回到配置值（可见时同步重建几何）
            m_panel->setPreviewOpacity(-1);
            m_panel->setPreviewMaxWidth(-1);
            if (result == QDialog::Rejected) {
                // 取消：球尺寸与位置完整还原到打开对话框前（含贴边停靠间隙）
                setFixedSize(sizeForDiameter(m_store.appearance().ballSize));
                move(m_settingsOpenPos);
                update();
            }
        });
    }
    // 记录打开前位置：无论预览期间球心锚定到何处，取消都回到该点
    m_settingsOpenPos = pos();
    // 每次打开按已保存配置重建控件状态（上次取消的预览残留不保留）
    m_settings->syncFromConfig();
    centerOnBallScreen(m_settings);
    m_settings->show();
    m_settings->raise();
    m_settings->activateWindow();
}

// F15：对话框始终在球当前所在显示器的工作区居中。
// 几何统一取自 QGuiApplication 屏幕列表（与展开面板同源），
// 避免与 screenAt 的几何口径不一致导致居中偏移。
void FloatingBall::centerOnBallScreen(QWidget* dialog)
{
    QList<QRect> screenGeoms;
    const QList<QScreen*> screens = QGuiApplication::screens();
    for (QScreen* s : screens) {
        screenGeoms.append(s->availableGeometry());
    }
    const int idx = window_layout::screenIndexAt(screenGeoms, geometry().center());
    const QRect area = idx >= 0 ? screenGeoms.at(idx)
                                : QGuiApplication::primaryScreen()->availableGeometry();
    // show 前对话框尚未布局，尺寸为默认值；先强制抛光并调整到布局所需尺寸再居中
    dialog->ensurePolished();
    dialog->adjustSize();
    dialog->move(area.center() - QPoint(dialog->width() / 2, dialog->height() / 2));
}

void FloatingBall::applyAppearance(int diameter)
{
    const QPoint center = geometry().center();
    setFixedSize(sizeForDiameter(diameter));
    // 尺寸切换时保持球心位置，越出可视区才夹取；
    // 屏幕口径与展开面板/对话框居中统一取自 QGuiApplication 屏幕列表（F15）
    QList<QRect> screenGeoms;
    const QList<QScreen*> screens = QGuiApplication::screens();
    for (QScreen* s : screens) {
        screenGeoms.append(s->availableGeometry());
    }
    const int idx = window_layout::screenIndexAt(screenGeoms, center);
    const QRect scr = idx >= 0 ? screenGeoms.at(idx)
                               : QGuiApplication::primaryScreen()->availableGeometry();
    const QPoint topLeft = center - QPoint(width() / 2, height() / 2);
    move(window_layout::clampBallTopLeft(topLeft, size(), scr));
    update();
}

QScreen* FloatingBall::screenAtCursor() const
{
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    return screen ? screen : QGuiApplication::primaryScreen();
}

QScreen* FloatingBall::restoreScreen() const
{
    const QString name = m_store.ballScreen();
    if (!name.isEmpty()) {
        for (QScreen* s : QGuiApplication::screens()) {
            if (s->name() == name) {
                return s;
            }
        }
    }
    return QGuiApplication::primaryScreen();
}

void FloatingBall::savePosition()
{
    m_store.setBallPosition(pos());
    QScreen* screen = QGuiApplication::screenAt(geometry().center());
    m_store.setBallScreen(screen ? screen->name() : QString());
    m_store.save();
}
