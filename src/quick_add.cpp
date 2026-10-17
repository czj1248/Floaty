#include "quick_add.h"

#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>

#include "config_store.h"
#include "drop_service.h"

namespace {

// 拖入项的默认显示名：文件取主名，文件夹取目录名
QString defaultDropName(const QString& path)
{
    const QFileInfo info(path);
    if (info.isDir()) {
        return info.fileName();
    }
    return info.completeBaseName();
}

} // namespace

bool quickAddPaths(QWidget* parent, ConfigStore& store, const QStringList& localPaths,
                   const QString& groupId)
{
    const QList<drop_service::DropVerdict> verdicts =
        drop_service::classifyDroppedPaths(localPaths);
    QStringList accepted = drop_service::acceptedPaths(verdicts);
    const QStringList rejected = drop_service::rejectedReasons(verdicts);

    if (accepted.isEmpty()) {
        QMessageBox::warning(parent, QStringLiteral("浮启"),
                             QStringLiteral("拖入的内容均不支持添加：\n%1")
                                 .arg(rejected.join(QLatin1Char('\n'))));
        return false;
    }

    // 混合拖放：先逐一列出被拒绝的项
    if (!rejected.isEmpty()) {
        QMessageBox::information(parent, QStringLiteral("浮启"),
                                 QStringLiteral("以下内容未添加：\n%1")
                                     .arg(rejected.join(QLatin1Char('\n'))));
    }

    if (accepted.size() == 1) {
        // 单文件：弹名称确认框，预填默认名
        const QString path = accepted.first();
        const QString fallback = defaultDropName(path);
        bool ok = false;
        const QString text = QInputDialog::getText(
            parent, QStringLiteral("浮启 · 添加快捷项"), QStringLiteral("快捷项名称："),
            QLineEdit::Normal, fallback, &ok);
        if (!ok) {
            return false; // Esc 取消，整次拖放不加入
        }
        accepted = {path};
        QString name = text.trimmed();
        if (name.isEmpty()) {
            name = fallback;
        }
        const std::optional<TargetType> type = detectTargetType(path);
        if (!type.has_value()) {
            return false;
        }
        store.addItem(name, path, *type, groupId);
        store.save();
        return true;
    }

    // 多文件：按默认名批量直接加入
    for (const QString& path : accepted) {
        const std::optional<TargetType> type = detectTargetType(path);
        if (type.has_value()) {
            store.addItem(defaultDropName(path), path, *type, groupId);
        }
    }
    store.save();
    return true;
}
