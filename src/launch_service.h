#pragma once

#include "types.h"

// 启动结果
enum class LaunchResult {
    Success,       // 已交给系统打开
    TargetMissing, // 目标不存在（文件或文件夹已被移动/删除）
    OpenFailed     // 系统调用打开失败
};

// 启动服务（F3）：文件类目标（程序/脚本/文档）与文件夹统一走系统默认打开方式，
// 等效于在资源管理器中双击；网址交给默认浏览器
class LaunchService {
public:
    LaunchResult launch(const QuickItem& item) const;

    // 失败时给用户的明确提示文案
    static QString failureMessage(LaunchResult result, const QuickItem& item);
};
