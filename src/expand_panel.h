#pragma once

#include <QWidget>

#include "launch_service.h"
#include "types.h"

class QListWidget;
class QListWidgetItem;
class ConfigStore;

// 展开面板（F2）：弹出式窗口，点击面板外任意区域自动收起；
// 双击快捷项启动目标（F3），单击不启动；快捷项超量时列表内部滚动
class ExpandPanel : public QWidget {
    Q_OBJECT
public:
    explicit ExpandPanel(ConfigStore& store, QWidget* parent = nullptr);

    // 依据悬浮球的全局几何重建列表并展开（面板尺寸不变，悬浮球尺寸不变）
    void popupForBall(const QRect& ballGlobal);

    // 从配置重建列表（管理窗口增删改后同步）
    void reload();

private:
    void launchCurrent(QListWidgetItem* rowItem);
    QSize preferredSize() const;

    ConfigStore& m_store;
    LaunchService m_launcher;
    QListWidget* m_list;
};
