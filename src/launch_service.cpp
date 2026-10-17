#include "launch_service.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>

LaunchResult LaunchService::launch(const QuickItem& item) const
{
    // 网址：交给默认浏览器，不做存在性检查
    if (item.type == TargetType::Url) {
        if (QDesktopServices::openUrl(QUrl(item.target))) {
            return LaunchResult::Success;
        }
        return LaunchResult::OpenFailed;
    }

    // 文件类与文件夹：先确认目标存在，再统一走系统默认打开方式
    const QFileInfo info(item.target);
    if (!info.exists()) {
        return LaunchResult::TargetMissing;
    }
    if (QDesktopServices::openUrl(QUrl::fromLocalFile(item.target))) {
        return LaunchResult::Success;
    }
    return LaunchResult::OpenFailed;
}

QString LaunchService::failureMessage(LaunchResult result, const QuickItem& item)
{
    switch (result) {
    case LaunchResult::TargetMissing:
        return QStringLiteral("目标不存在，可能已被移动或删除：\n%1").arg(item.target);
    case LaunchResult::OpenFailed:
        return QStringLiteral("打开失败：\n%1").arg(item.target);
    case LaunchResult::Success:
        break;
    }
    return QString();
}
