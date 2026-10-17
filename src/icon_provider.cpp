#include "icon_provider.h"

#include <QApplication>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QStyle>

QIcon iconForTarget(const QuickItem& item)
{
    switch (item.type) {
    case TargetType::Folder:
        return QFileIconProvider().icon(QFileIconProvider::Folder);
    case TargetType::Url:
        return QApplication::style()->standardIcon(QStyle::SP_FileLinkIcon);
    default:
        // 程序/脚本/文档：系统按文件读取关联图标，.lnk 会解析到目标程序图标
        return QFileIconProvider().icon(QFileInfo(item.target));
    }
}
