#pragma once

#include <QDialog>
#include <optional>

#include "types.h"

class QListWidget;
class QListWidgetItem;
class QPushButton;
class ConfigStore;

// 管理窗口（F4/F9/F10/F11）：品牌头 + 左栏分组（含"全部快捷项"汇总视图）
// + 右栏快捷项列表与操作按钮；组内支持拖拽与上移/下移排序
class ManagerDialog : public QDialog {
    Q_OBJECT
public:
    explicit ManagerDialog(ConfigStore& store, QWidget* parent = nullptr);

signals:
    // 快捷项或分组发生增删改后通知悬浮球刷新展开面板
    void itemsChanged();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void reloadGroups(const QString& selectGroupId = QString());
    void reloadItems(const QString& selectItemId = QString());
    void updateActions();
    QString currentGroupId() const;
    QString currentItemId() const;
    QuickGroup groupById(const QString& id) const;

    void createGroup();
    void renameCurrentGroup();
    void deleteCurrentGroup();
    void addFile();
    void addFolder();
    void addUrl();
    void editCurrent();
    void removeCurrent();
    void moveCurrent(int delta);
    void commitGroupReorder();

    // 校验目标并询问名称后写入指定分组；白名单外给出"不支持的文件类型"提示
    void commitTarget(const QString& target, const QString& groupId);
    // 询问快捷项名称（预填默认名，输入为空时使用默认名），取消时返回 std::nullopt
    std::optional<QString> askName(const QString& defaultName);

    ConfigStore& m_store;
    QListWidget* m_groupList;
    QListWidget* m_itemList;
    QPushButton* m_editButton;
    QPushButton* m_removeButton;
    QPushButton* m_upButton;
    QPushButton* m_downButton;
    QPushButton* m_renameGroupButton;
    QPushButton* m_deleteGroupButton;
};
