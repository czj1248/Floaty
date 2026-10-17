#pragma once

#include <QColor>
#include <QString>

// 「浮青」设计语言令牌（V2_spec 第 3 章）。
// 全部界面只能引用本文件令牌，不允许在各窗口散落自定义色值。
namespace theme {

// 色彩令牌
inline const QColor kPrimary{0x2E, 0x8B, 0x6B};        // 竹绿，主色
inline const QColor kPrimaryHover{0x3A, 0xA4, 0x7E};   // 主色悬停
inline const QColor kPrimaryPressed{0x26, 0x73, 0x5A}; // 主色按下
inline const QColor kPrimaryLight{0xE5, 0xF2, 0xEB};   // 浅绿底
inline const QColor kPanelBg{0xF7, 0xFB, 0xF8};        // 青瓷白
inline const QColor kTextMain{0x24, 0x34, 0x2D};       // 主文字
inline const QColor kTextSub{0x6B, 0x7B, 0x73};        // 次文字
inline const QColor kBorder{0xD6, 0xE2, 0xDB};         // 描边
inline const QColor kDanger{0xC0, 0x50, 0x4D};         // 危险色
inline const QColor kWhite{0xFF, 0xFF, 0xFF};

// 球体渐变两端
inline const QColor kBallHighlight{0x7C, 0xC9, 0xA9};
inline const QColor kBallBody{0x2E, 0x8B, 0x6B};

// 字号
constexpr int kFontSizeBody = 9;     // 面板、菜单、列表正文（pt）
constexpr int kFontSizeTitle = 12;   // 品牌头主行（pt）
constexpr int kFontSizeCaption = 8;  // 说明文字（pt）

inline QString fontFamily() { return QStringLiteral("Microsoft YaHei UI"); }

// 尺寸令牌
constexpr int kRadiusWindow = 8; // 菜单、面板圆角
constexpr int kRadiusControl = 6; // 按钮、列表行圆角
constexpr int kPanelRowHeight = 34;
constexpr int kPanelMaxWidthMax = 480;     // 面板最大宽度设置上限
constexpr int kPanelMaxWidthStep = 20;     // 面板最大宽度设置步进
// 一个正文字符宽度（9pt，96 DPI 下 9*96/72≈12px，中文字符宽约等于字号像素）
constexpr int kPanelCharWidth = kFontSizeBody * 96 / 72;
// 容纳一个字符的面板宽度 = 容器左右边距(12*2) + 行左右边距(2*2) + 图标(20) + 图标文字间距(8) + 字符宽
constexpr int kPanelOneCharWidth = 12 * 2 + 2 * 2 + 20 + 8 + kPanelCharWidth;
// 面板最小宽度：取一个字符面板宽，并向上对齐到步进倍数，保证 (上限-下限) 可被步进整除
constexpr int kPanelMinWidth =
        kPanelOneCharWidth + (kPanelMaxWidthStep - (kPanelOneCharWidth % kPanelMaxWidthStep))
                              % kPanelMaxWidthStep;
constexpr int kPanelMaxWidthMin = kPanelMinWidth; // 面板最大宽度设置下限（等于最小宽度）
constexpr int kDefaultPanelMaxWidth = 320; // 面板最大宽度默认值

// 贴边吸附参数
constexpr int kSnapDistance = 24; // 进入吸附的边缘距离
constexpr int kSnapGap = 8;       // 停靠后与屏幕边缘的间隙

// 悬浮球三档尺寸
constexpr int kBallSizeSmall = 40;
constexpr int kBallSizeStandard = 48;
constexpr int kBallSizeLarge = 56;

// 外观默认值
constexpr int kDefaultBallSize = kBallSizeStandard;
constexpr int kDefaultOpacity = 88;          // 面板不透明度默认值（百分比）
constexpr int kOpacityMin = 0;               // 面板不透明度调节下限
constexpr int kOpacityMax = 100;             // 面板不透明度调节上限
constexpr int kOpacityStep = 5;              // 面板不透明度调节步进

// 应用到 qApp 的全局样式表：菜单、对话框、按钮、列表、输入控件统一风格
QString buildGlobalStyleSheet();

} // namespace theme
