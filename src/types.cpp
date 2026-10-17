#include "types.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

namespace {

// F4 类型白名单：程序 / 脚本 / 文档
const char* const kProgramExts[] = { "exe", "lnk" };
const char* const kScriptExts[] = { "bat", "cmd" };
const char* const kDocumentExts[] = {
    "txt", "md", "pdf", "doc", "docx", "xls", "xlsx", "ppt", "pptx",
    "jpg", "jpeg", "png", "gif", "bmp", "webp",
    "mp3", "wav", "flac", "mp4", "mkv", "avi", "mov",
    "zip", "rar", "7z"
};

template <size_t N>
bool contains(const char* const (&arr)[N], const QString& ext)
{
    for (const char* e : arr) {
        if (ext == QLatin1String(e)) {
            return true;
        }
    }
    return false;
}

} // namespace

QString targetTypeName(TargetType type)
{
    switch (type) {
    case TargetType::Program:  return QStringLiteral("程序");
    case TargetType::Script:   return QStringLiteral("脚本");
    case TargetType::Document: return QStringLiteral("文档");
    case TargetType::Folder:   return QStringLiteral("文件夹");
    case TargetType::Url:      return QStringLiteral("网址");
    }
    return QString();
}

QString targetTypeKey(TargetType type)
{
    switch (type) {
    case TargetType::Program:  return QStringLiteral("program");
    case TargetType::Script:   return QStringLiteral("script");
    case TargetType::Document: return QStringLiteral("document");
    case TargetType::Folder:   return QStringLiteral("folder");
    case TargetType::Url:      return QStringLiteral("url");
    }
    return QString();
}

std::optional<TargetType> targetTypeFromKey(const QString& key)
{
    if (key == QLatin1String("program"))  return TargetType::Program;
    if (key == QLatin1String("script"))   return TargetType::Script;
    if (key == QLatin1String("document")) return TargetType::Document;
    if (key == QLatin1String("folder"))   return TargetType::Folder;
    if (key == QLatin1String("url"))      return TargetType::Url;
    return std::nullopt;
}

bool isWhitelistedExtension(const QString& ext)
{
    const QString e = ext.toLower();
    return contains(kProgramExts, e) || contains(kScriptExts, e) || contains(kDocumentExts, e);
}

std::optional<TargetType> detectTargetType(const QString& target)
{
    const QString t = target.trimmed();
    if (t.isEmpty()) {
        return std::nullopt;
    }
    // 网址：http:// 或 https:// 开头
    if (t.startsWith(QLatin1String("http://"), Qt::CaseInsensitive)
        || t.startsWith(QLatin1String("https://"), Qt::CaseInsensitive)) {
        return TargetType::Url;
    }
    // 带协议前缀但非 http(s) 的输入（如 file://、ftp://）一律拒绝，不当作本地路径
    if (t.contains(QLatin1String("://"))) {
        return std::nullopt;
    }
    // 文件夹：必须真实存在（管理窗口通过目录对话框选择，均真实存在）
    if (QFileInfo(t).isDir()) {
        return TargetType::Folder;
    }
    // 文件：按扩展名归类，白名单外拒绝
    const QString ext = QFileInfo(t).suffix().toLower();
    if (contains(kProgramExts, ext)) {
        return TargetType::Program;
    }
    if (contains(kScriptExts, ext)) {
        return TargetType::Script;
    }
    if (contains(kDocumentExts, ext)) {
        return TargetType::Document;
    }
    return std::nullopt;
}

bool isSupportedTarget(const QString& target)
{
    return detectTargetType(target).has_value();
}

QString whitelistedFileFilter()
{
    QStringList patterns;
    for (const char* e : kProgramExts) {
        patterns << QStringLiteral("*.") + QLatin1String(e);
    }
    for (const char* e : kScriptExts) {
        patterns << QStringLiteral("*.") + QLatin1String(e);
    }
    for (const char* e : kDocumentExts) {
        patterns << QStringLiteral("*.") + QLatin1String(e);
    }
    return QStringLiteral("支持的文件 (%1);;所有文件 (*)").arg(patterns.join(QLatin1Char(' ')));
}
