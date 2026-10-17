#pragma once

#include <QIcon>

#include "types.h"

// 快捷项图标（F4）：文件与文件夹读取系统图标（等效资源管理器显示），
// 网址使用通用图标
QIcon iconForTarget(const QuickItem& item);
