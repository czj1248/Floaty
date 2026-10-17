#pragma once

#include <QPoint>
#include <QSize>
#include <QWidget>

class ConfigStore;
class ExpandPanel;
class ManagerDialog;
class SettingsDialog;
class QScreen;

// 悬浮球（F1/F6/F7/F13/F14/F15/F16）：无边框置顶小窗口，不抢键盘焦点；
// 左键单击切换展开面板，左键按住拖动（松开贴边吸附），右键三项菜单；
// 接受资源管理器文件拖放快速添加到默认分组
class FloatingBall : public QWidget {
    Q_OBJECT
public:
    explicit FloatingBall(ConfigStore& store, QWidget* parent = nullptr);

    // 外观三档对应的边长：仅允许 40/48/56
    static QSize sizeForDiameter(int diameter);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void togglePanel();
    void openManager();
    void openSettings();
    void savePosition();
    void applyAppearance(int diameter);
    void centerOnBallScreen(QWidget* dialog);
    QScreen* screenAtCursor() const;
    QScreen* restoreScreen() const;

    ConfigStore& m_store;
    ExpandPanel* m_panel;
    ManagerDialog* m_manager = nullptr;
    SettingsDialog* m_settings = nullptr;

    // 左键按下状态：用于区分拖动与单击
    bool m_pressed = false;
    bool m_dragging = false;
    QPoint m_pressGlobal;     // 按下时鼠标全局坐标
    QPoint m_pressWindowPos;  // 按下时窗口左上角全局坐标

    bool m_dropHover = false; // 拖放悬停：主色描边高亮

    QPoint m_settingsOpenPos; // 设置对话框打开时的球位置（取消还原用）
};
