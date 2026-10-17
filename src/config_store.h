#pragma once

#include <QList>
#include <QPoint>
#include <QString>

#include "types.h"

// 分组操作结果（F10）
enum class GroupError {
    Ok,
    EmptyName,      // 分组名为空
    DuplicateName,  // 与已有分组重名
    ProtectedGroup, // 默认分组禁止删除或重命名
    NotFound        // 分组或快捷项不存在
};

// 外观设置（F13）
struct Appearance {
    int ballSize = 48;      // 悬浮球直径：40 / 48 / 56
    int opacity = 88;       // 展开面板背景不透明度，百分比 0-100
    int panelMaxWidth = 320; // 展开面板最大宽度，80-480（步进 20）
};

// 配置持久化（F5/F10/F11/F13/F15）：JSON 文件，
// 默认路径 %APPDATA%\Floaty\config.json。V2 结构，不做 V1 迁移。
class ConfigStore {
public:
    // 默认配置文件路径：%APPDATA%/Floaty/config.json
    static QString defaultConfigFilePath();

    ConfigStore();
    // 指定配置文件路径（测试隔离用）
    explicit ConfigStore(const QString& filePath);

    // 读取配置；文件不存在时为 V2 默认空状态（仅含默认分组）
    void load();
    // 立即写盘。使用 QSaveFile 原子替换，不残留临时文件
    void save();

    // ---- 分组（F10）----
    // 分组顺序：默认分组恒在最前，自定义分组按创建时间
    const QList<QuickGroup>& groups() const { return m_groups; }
    GroupError createGroup(const QString& name, QString* newId = nullptr);
    GroupError renameGroup(const QString& id, const QString& name);
    // 删除自定义分组：组内快捷项全部移入默认分组
    GroupError deleteGroup(const QString& id);
    // 面板中需要显示分区标题的分组：默认与自定义顺序，仅含非空分组
    QList<QuickGroup> visibleGroups() const;
    bool groupExists(const QString& id) const;

    // ---- 快捷项（F4/F10/F11）----
    const QList<QuickItem>& items() const { return m_items; }
    QList<QuickItem> itemsInGroup(const QString& groupId) const;
    // 新增快捷项到指定分组末尾，返回新项 id；非法分组返回空串
    QString addItem(const QString& name, const QString& target, TargetType type,
                    const QString& groupId = QLatin1String(kDefaultGroupId));
    // 按 id 编辑：名称、目标、类型与归属均可改变；改归属时排到目标组末尾
    GroupError updateItem(const QString& id, const QString& name, const QString& target,
                          TargetType type, const QString& groupId);
    GroupError removeItem(const QString& id);
    // 组内移动：delta 为 -1 上移、+1 下移；首行上移/末行下移不回绕，返回 false
    bool moveItem(const QString& id, int delta);
    // 拖拽排序：按给定的 id 顺序重排某分组，顺序外的 id 不允许出现在列表中
    GroupError reorderGroup(const QString& groupId, const QStringList& orderedIds);

    // ---- 悬浮球位置（F1/F15）----
    QPoint ballPosition() const { return m_ballPos; }
    bool hasBallPosition() const { return m_hasBallPos; }
    void setBallPosition(const QPoint& pos);
    // 球所在显示器名称（QScreen::name()），用于多屏恢复
    QString ballScreen() const { return m_ballScreen; }
    void setBallScreen(const QString& screen) { m_ballScreen = screen; }

    // ---- 外观（F13）----
    Appearance appearance() const { return m_appearance; }
    void setAppearance(const Appearance& a) { m_appearance = a; }

private:
    QuickItem* findItem(const QString& id);
    QuickGroup* findGroup(const QString& id);
    void renumberGroup(const QString& groupId);

    QString m_filePath;
    QList<QuickGroup> m_groups;
    QList<QuickItem> m_items;
    QPoint m_ballPos;
    QString m_ballScreen;
    bool m_hasBallPos = false;
    Appearance m_appearance;
};
