#pragma once

#include <QList>
#include <QString>
#include <QStringList>

// 拖放快速添加的分流判定（F16）：纯逻辑，便于自动化测试。
// 仅接受资源管理器拖入的本地文件/文件夹（含 .lnk 与白名单文档），
// 网址、文本、白名单外文件一律拒绝。
namespace drop_service {

// 单个拖入项的判定结果
struct DropVerdict {
    QString path;    // 原始本地路径
    bool accepted;   // 是否可加入
    QString reason;  // 拒绝原因（accepted 为 false 时用于逐项提示）
};

// 对一批拖入路径逐一判定，合法与非法保持原始顺序
QList<DropVerdict> classifyDroppedPaths(const QStringList& paths);

// 便捷提取
QStringList acceptedPaths(const QList<DropVerdict>& verdicts);
QStringList rejectedReasons(const QList<DropVerdict>& verdicts);

} // namespace drop_service
