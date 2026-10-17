#pragma once

#include <QString>
#include <optional>

// 默认分组标识：内置分组，不可删除、不可重命名，始终排在最前
inline constexpr const char* kDefaultGroupId = "default";
inline const QString kDefaultGroupName = QStringLiteral("默认分组");

// 快捷项目标类型，对应 spec F4 的类型白名单
enum class TargetType {
    Program,    // 程序：.exe .lnk
    Script,     // 脚本：.bat .cmd
    Document,   // 文档：白名单内的其余扩展名
    Folder,     // 本地文件夹
    Url         // http:// 或 https:// 网址
};

// 一个快捷项：展开面板与管理窗口中的同一条数据
struct QuickItem {
    QString id;        // 唯一标识，V2 用于排序与归属变更
    QString name;      // 显示名称
    QString target;    // 本地文件/文件夹路径，或 http(s) 网址
    TargetType type = TargetType::Document;
    QString groupId = QLatin1String(kDefaultGroupId); // 所属分组
    int order = 0;     // 组内顺序，从 0 起
};

// 分组：默认分组受保护
struct QuickGroup {
    QString id;
    QString name;
    bool protectedGroup = false;
};

// 类型中文名，用于界面提示
QString targetTypeName(TargetType type);

// 类型 <-> 配置文件中的字符串键
QString targetTypeKey(TargetType type);
std::optional<TargetType> targetTypeFromKey(const QString& key);

// 是否属于白名单内的文件扩展名（小写、不含点）
bool isWhitelistedExtension(const QString& ext);

// 按目标自动识别类型：http(s) 前缀为网址；真实存在的文件夹为文件夹；
// 其余按扩展名归类；白名单外返回 std::nullopt
std::optional<TargetType> detectTargetType(const QString& target);

// 目标是否属于 F4 支持的类型全集（添加/编辑时的准入判断）
bool isSupportedTarget(const QString& target);

// 文件选择对话框的过滤器，由白名单扩展名生成
QString whitelistedFileFilter();
