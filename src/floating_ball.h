#pragma once

#include <QPoint>
#include <QSize>
#include <QWidget>

class ConfigStore;
class ExpandPanel;
class ManagerDialog;

// 悬浮球（F1/F6）：无边框置顶小窗口，不抢键盘焦点；
// 左键单击切换展开面板，左键按住拖动改变位置，右键弹出菜单
class FloatingBall : public QWidget {
    Q_OBJECT
public:
    explicit FloatingBall(ConfigStore& store, QWidget* parent = nullptr);

    // 悬浮球固定尺寸（首次默认位置计算也需要）
    static QSize ballSize() { return QSize(48, 48); }

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void togglePanel();
    void openManager();
    void savePosition();

    ConfigStore& m_store;
    ExpandPanel* m_panel;
    ManagerDialog* m_manager = nullptr;

    // 左键按下状态：用于区分拖动与单击
    bool m_pressed = false;
    bool m_dragging = false;
    QPoint m_pressGlobal;     // 按下时鼠标全局坐标
    QPoint m_pressWindowPos;  // 按下时窗口左上角全局坐标
};
