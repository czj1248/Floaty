#include <QApplication>
#include <QFont>
#include <QGuiApplication>
#include <QPalette>
#include <QScreen>

#include "ball_logo.h"
#include "config_store.h"
#include "floating_ball.h"
#include "single_instance.h"
#include "theme.h"
#include "window_layout.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Floaty"));
    QApplication::setOrganizationName(QStringLiteral("Floaty"));

    // 显式使用「浮青」浅色调色板与中文字体：
    // 不依赖系统深浅色（Win11 深色模式下 Qt 默认调色板会把未设置背景的滚动区等染成深色）
    QFont appFont(QStringLiteral("Microsoft YaHei UI"), 9);
    app.setFont(appFont);
    QPalette pal = app.palette();
    pal.setColor(QPalette::Window, theme::kPanelBg);
    pal.setColor(QPalette::Base, theme::kWhite);
    pal.setColor(QPalette::AlternateBase, theme::kPrimaryLight);
    pal.setColor(QPalette::Button, theme::kWhite);
    pal.setColor(QPalette::Text, theme::kTextMain);
    pal.setColor(QPalette::WindowText, theme::kTextMain);
    pal.setColor(QPalette::ButtonText, theme::kTextMain);
    pal.setColor(QPalette::Highlight, theme::kPrimary);
    pal.setColor(QPalette::HighlightedText, theme::kWhite);
    pal.setColor(QPalette::PlaceholderText, theme::kTextSub);
    app.setPalette(pal);

    // 「浮青」全局样式：菜单、对话框、按钮、列表统一令牌
    app.setStyleSheet(theme::buildGlobalStyleSheet());

    // 品牌窗口图标：所有对话框标题栏左上角与任务栏共用同一品牌图形
    app.setWindowIcon(ball_logo::brandIcon());

    // 单实例：已有实例在运行时直接退出，不创建第二个悬浮球
    if (!acquireSingleInstance()) {
        return 0;
    }

    ConfigStore store;
    store.load();

    // 首次启动（无配置文件）：以空快捷项与屏幕默认位置开始，并创建配置文件（F5/F15）
    if (!store.hasBallPosition()) {
        const QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
        const QSize ball = FloatingBall::sizeForDiameter(store.appearance().ballSize);
        store.setBallPosition(window_layout::defaultBallTopLeft(ball, screen));
        store.save();
    }

    FloatingBall floatingBall(store);
    floatingBall.show();
    return app.exec();
}
