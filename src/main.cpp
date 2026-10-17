#include <QApplication>
#include <QGuiApplication>
#include <QScreen>

#include "config_store.h"
#include "floating_ball.h"
#include "single_instance.h"
#include "window_layout.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Floaty"));
    QApplication::setOrganizationName(QStringLiteral("Floaty"));

    // 单实例：已有实例在运行时直接退出，不创建第二个悬浮球
    if (!acquireSingleInstance()) {
        return 0;
    }

    ConfigStore store;
    store.load();

    // 首次启动（无配置文件）：以空快捷项与屏幕默认位置开始，并创建配置文件（F5）
    const QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
    if (!store.hasBallPosition()) {
        store.setBallPosition(window_layout::defaultBallTopLeft(FloatingBall::ballSize(), screen));
        store.save();
    }

    FloatingBall ball(store);
    ball.show();
    return app.exec();
}
