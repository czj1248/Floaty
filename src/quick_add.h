#pragma once

#include <QStringList>

class ConfigStore;
class QWidget;

// F16 拖拽快速添加的统一入口：对一批资源管理器拖入的本地路径分流、
// 命名确认并写入配置。单个合法项弹名称确认框；多个合法项按默认名批量加入；
// 非法项逐一列出。有新增返回 true。
bool quickAddPaths(QWidget* parent, ConfigStore& store, const QStringList& localPaths,
                   const QString& groupId);
