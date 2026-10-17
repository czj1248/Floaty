#include "drop_service.h"

#include <QFileInfo>

#include "types.h"

namespace drop_service {

QList<DropVerdict> classifyDroppedPaths(const QStringList& paths)
{
    QList<DropVerdict> verdicts;
    for (const QString& raw : paths) {
        DropVerdict v;
        v.path = raw;
        const QString path = raw.trimmed();

        const QFileInfo info(path);
        if (path.isEmpty() || !info.exists()) {
            v.accepted = false;
            v.reason = QStringLiteral("%1：文件不存在").arg(path);
            verdicts.append(v);
            continue;
        }
        if (info.isDir()) {
            // 文件夹始终接受
            v.accepted = true;
            verdicts.append(v);
            continue;
        }
        // 文件：按扩展名白名单判定（含 exe/lnk/bat/cmd 与 25 种文档）
        const QString ext = info.suffix().toLower();
        if (isWhitelistedExtension(ext)) {
            v.accepted = true;
        } else {
            v.accepted = false;
            v.reason = QStringLiteral("%1：不支持的文件类型").arg(info.fileName());
        }
        verdicts.append(v);
    }
    return verdicts;
}

QStringList acceptedPaths(const QList<DropVerdict>& verdicts)
{
    QStringList result;
    for (const DropVerdict& v : verdicts) {
        if (v.accepted) {
            result.append(v.path);
        }
    }
    return result;
}

QStringList rejectedReasons(const QList<DropVerdict>& verdicts)
{
    QStringList result;
    for (const DropVerdict& v : verdicts) {
        if (!v.accepted) {
            result.append(v.reason);
        }
    }
    return result;
}

} // namespace drop_service
