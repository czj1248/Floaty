#pragma once

#include <QDialog>

class ConfigStore;
class QButtonGroup;
class QLabel;
class QRadioButton;
class QSlider;

// 外观设置对话框（F13）：球大小三档、面板不透明度滑块、面板最大宽度滑块。
// 控件变动时按作用域分别发出预览信号（此时不落盘）；
// 确定写入配置，取消由调用方按打开前状态还原预览。
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(ConfigStore& store, QWidget* parent = nullptr);

    // 从已保存配置同步全部控件（不发预览信号）；每次打开对话框前调用，
    // 保证取消后再次打开时显示的是已保存值而非上次预览残留值
    void syncFromConfig();

signals:
    // 球直径预览（40/48/56）：仅作用于悬浮球几何
    void ballSizePreview(int ballDiameter);
    // 面板背景不透明度预览（0-100）：仅作用于已展开面板背景
    void opacityPreview(int opacityPercent);
    // 面板最大宽度预览（80-480，步进 20）：仅作用于已展开面板宽度
    void panelMaxWidthPreview(int maxWidthPx);

private:
    int selectedDiameter() const;

    ConfigStore& m_store;
    QButtonGroup* m_sizeGroup = nullptr;
    QRadioButton* m_small = nullptr;
    QRadioButton* m_standard = nullptr;
    QRadioButton* m_large = nullptr;
    QSlider* m_opacitySlider = nullptr;
    QLabel* m_opacityValue = nullptr;
    QSlider* m_maxWidthSlider = nullptr;
    QLabel* m_maxWidthValue = nullptr;
};
