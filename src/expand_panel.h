#pragma once

#include <QWidget>

#include "launch_service.h"
#include "types.h"

class QScrollArea;
class QScrollBar;
class QTimer;
class QVBoxLayout;
class ConfigStore;

namespace floaty_ui {
class ItemRow;
class MarqueeLabel;
} // namespace floaty_ui

// 展开面板（F2/F10/F12/F16）：弹出式半透明圆角窗口，按分组分区显示，
// 宽度随内容自适应，超长名称悬停跑马灯，可按分组接收资源管理器拖放
class ExpandPanel : public QWidget {
    Q_OBJECT
public:
    explicit ExpandPanel(ConfigStore& store, QWidget* parent = nullptr);

    // 依据悬浮球的全局几何重建内容并展开；屏幕按球当前所在显示器计算（F15）
    void popupForBall(const QRect& ballGlobal);

    // 从配置重建（管理窗口改动、拖放新增、外观变动后同步）
    void reload();

    // F13 设置对话框实时预览：-1 表示使用配置中的不透明度
    void setPreviewOpacity(int opacityPercent) { m_previewOpacity = opacityPercent; update(); }
    // F13 面板最大宽度实时预览：-1 表示使用配置值；面板可见时立即按新上限重算宽度
    void setPreviewMaxWidth(int maxWidthPx);

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    using ItemRow = floaty_ui::ItemRow;
    using MarqueeLabel = floaty_ui::MarqueeLabel;

    void launchItem(const QString& itemId);
    void setHoveredRow(ItemRow* row);
    void clearDropTarget();
    // 面板坐标下落点所属分组：标题或组内行返回该组，空白区域返回默认分组
    QString groupIdAt(const QPoint& panelPos) const;
    // 当前生效的面板最大宽度（预览值优先，否则取配置）
    int effectiveMaxWidth() const;
    // 依据 m_ballGeometry 与当前内容重设面板大小与位置
    void applyGeometry();
    // 悬浮滚动条位置：贴内容右缘内侧，覆盖在内边距与行缘区域之上，不占布局
    void positionOverlayBar();

    ConfigStore& m_store;
    LaunchService m_launcher;
    QScrollArea* m_scroll;
    QScrollBar* m_overlayBar = nullptr; // 悬浮在视口右缘的垂直滚动条
    QWidget* m_container;
    QVBoxLayout* m_containerLayout;
    QTimer* m_marqueeTimer;

    ItemRow* m_hoveredRow = nullptr;
    bool m_dropActive = false;          // 拖放悬停在面板上：主色描边
    QWidget* m_dropTarget = nullptr;   // 当前拖放落点的标题行或快捷项行
    int m_panelWidth = 260;
    int m_contentHeight = 0;           // 全部内容自然高度，用于面板高度夹取
    int m_previewOpacity = -1;         // 设置实时预览的不透明度，-1 为跟随配置
    int m_previewMaxWidth = -1;        // 设置实时预览的最大宽度，-1 为跟随配置
    QRect m_ballGeometry;              // 本次展开时悬浮球全局几何（预览重算宽度用）
};
