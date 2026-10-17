#pragma once

#include <QDialog>
#include <optional>

#include "types.h"

class QListWidget;
class ConfigStore;

// 管理窗口（F4）：列出全部快捷项，支持添加（文件/文件夹/网址）、编辑、删除；
// 类型按扩展名自动识别，白名单外拒绝
class ManagerDialog : public QDialog {
    Q_OBJECT
public:
    explicit ManagerDialog(ConfigStore& store, QWidget* parent = nullptr);

signals:
    void itemsChanged(); // 快捷项发生增删改后通知悬浮球刷新展开面板

protected:
    void showEvent(QShowEvent* event) override;

private:
    void reload();
    void addFile();
    void addFolder();
    void addUrl();
    void editCurrent();
    void removeCurrent();

    // 校验目标并询问名称后写入配置；白名单外给出"不支持的文件类型"提示
    void commitTarget(const QString& target);
    // 询问快捷项名称（预填默认名，输入为空时使用默认名），取消时返回 std::nullopt
    std::optional<QString> askName(const QString& defaultName);

    ConfigStore& m_store;
    QListWidget* m_list;
};
