#include "theme.h"

namespace theme {

QString buildGlobalStyleSheet()
{
    // 注意：不能写通配 QWidget{} 规则，否则 QStyleSheetStyle 会给滚动区等容器
    // 强制开启样式化背景，遮盖各窗口自绘的半透明底；字体与文字色由 QApplication 统一设置。
    return QStringLiteral(
        // 对话框与普通容器
        "QDialog { background: #F7FBF8; }"
        "QLabel { background: transparent; }"

        // 展开面板：整棵滚动链保持透明，背景由 ExpandPanel::paintEvent 自绘半透明圆角
        "#expandPanel, #expandPanel QScrollArea,"
        "#expandPanel QScrollArea > QWidget, #expandPanel QScrollArea > QWidget > QWidget,"
        "#expandPanel QScrollBar:vertical { background: transparent; }"

        // 右键菜单（F8）
        "QMenu { background: #F7FBF8; border: 1px solid #D6E2DB; border-radius: 8px;"
        "       padding: 6px; }"
        "QMenu::item { height: 30px; padding: 0 18px 0 14px; border-radius: 6px;"
        "             background: transparent; }"
        "QMenu::item:selected { background: #E5F2EB; color: #24342D; }"
        "QMenu::separator { height: 1px; background: #D6E2DB; margin: 4px 8px; }"
        "QMenu::icon { left: 10px; }"

        // 按钮：默认描边次按钮
        "QPushButton { background: #FFFFFF; border: 1px solid #D6E2DB; border-radius: 6px;"
        "             padding: 5px 14px; color: #24342D; }"
        "QPushButton:hover { border-color: #2E8B6B; color: #2E8B6B; }"
        "QPushButton:pressed { background: #EEF5F0; }"
        "QPushButton:disabled { color: #9DAAA2; border-color: #E4EBE7; background: #F4F7F5; }"
        // 主操作：竹绿实心
        "QPushButton[variant=\"primary\"] { background: #2E8B6B; border: 1px solid #2E8B6B;"
        "                                 color: #FFFFFF; font-weight: bold; }"
        "QPushButton[variant=\"primary\"]:hover { background: #3AA47E; border-color: #3AA47E;"
        "                                        color: #FFFFFF; }"
        "QPushButton[variant=\"primary\"]:pressed { background: #26735A; border-color: #26735A; }"
        // 危险操作：仅危险色文字
        "QPushButton[variant=\"danger\"] { background: transparent; border: none;"
        "                                color: #C0504D; padding: 5px 14px; }"
        "QPushButton[variant=\"danger\"]:hover { color: #A93E3B; text-decoration: underline; }"

        // 列表：普通/悬停/选中三态全窗口统一（3.2 列表行状态色）
        "QListWidget { background: #FFFFFF; border: 1px solid #D6E2DB; border-radius: 8px;"
        "             outline: none; color: #24342D; }"
        "QListWidget::item { border-radius: 6px; color: #24342D; }"
        "QListWidget::item:hover { background: #E5F2EB; color: #24342D; }"
        "QListWidget::item:selected { background: #E5F2EB; color: #2E8B6B; font-weight: bold; }"
        // 管理窗口右栏行卡片名称标签：普通主文字色，选中随动态属性变主色加粗
        "QLabel#itemNameLabel { background: transparent; color: #24342D; }"
        "QLabel#itemNameLabel[selected=\"true\"] { color: #2E8B6B; font-weight: bold; }"

        // 展开面板悬浮滚动条：纤细半透明、贴右缘内侧，不占布局（F12）
        "QScrollBar[overlay=\"true\"]:vertical { background: transparent; width: 8px;"
        "                                     margin: 0; border: none; }"
        "QScrollBar[overlay=\"true\"]::handle:vertical { background: rgba(46,139,107,90);"
        "        border: none; border-radius: 3px; min-height: 24px; }"
        "QScrollBar[overlay=\"true\"]::handle:vertical:hover { background: rgba(38,115,90,150); }"
        "QScrollBar[overlay=\"true\"]::add-line:vertical,"
        " QScrollBar[overlay=\"true\"]::sub-line:vertical { height: 0; border: none; }"
        "QScrollBar[overlay=\"true\"]::add-page:vertical,"
        " QScrollBar[overlay=\"true\"]::sub-page:vertical { background: transparent; }"

        // 输入框
        "QLineEdit { background: #FFFFFF; border: 1px solid #D6E2DB; border-radius: 6px;"
        "           padding: 5px 8px; selection-background-color: #2E8B6B; }"
        "QLineEdit:focus { border: 1px solid #2E8B6B; }"

        // 滑块（F13 透明度）
        "QSlider::groove:horizontal { height: 4px; background: #D6E2DB; border-radius: 2px; }"
        "QSlider::sub-page:horizontal { background: #2E8B6B; border-radius: 2px; }"
        "QSlider::handle:horizontal { width: 14px; height: 14px; margin: -6px 0;"
        "                           background: #FFFFFF; border: 2px solid #2E8B6B;"
        "                           border-radius: 8px; }"
        "QSlider::handle:horizontal:hover { background: #E5F2EB; }"

        // 单选按钮（F13 球尺寸三档）
        "QRadioButton { spacing: 8px; background: transparent; }"
        "QRadioButton::indicator { width: 16px; height: 16px; border-radius: 9px;"
        "                         border: 1px solid #B9C8C0; background: #FFFFFF; }"
        "QRadioButton::indicator:hover { border-color: #2E8B6B; }"
        "QRadioButton::indicator:checked { border: 5px solid #2E8B6B; }"

        // 滚动条：纤细风格
        "QScrollBar:vertical { background: transparent; width: 8px; margin: 2px; }"
        "QScrollBar::handle:vertical { background: #C4D4CB; border-radius: 3px; min-height: 24px; }"
        "QScrollBar::handle:vertical:hover { background: #9FB8AB; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        "QScrollBar:horizontal { background: transparent; height: 8px; margin: 2px; }"
        "QScrollBar::handle:horizontal { background: #C4D4CB; border-radius: 3px; min-width: 24px; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: transparent; }"

        // 提示气泡
        "QToolTip { background: #24342D; color: #FFFFFF; border: none; border-radius: 4px;"
        "          padding: 3px 6px; }"
    );
}

} // namespace theme
