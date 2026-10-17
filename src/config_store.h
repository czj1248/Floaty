#pragma once

#include <QList>
#include <QPoint>
#include <QString>

#include "types.h"

// 配置持久化（F5）：JSON 文件，默认路径 %APPDATA%\Floaty\config.json
// 保存快捷项列表与悬浮球位置；不写注册表
class ConfigStore {
public:
    // 默认配置文件路径：%APPDATA%/Floaty/config.json
    static QString defaultConfigFilePath();

    ConfigStore();
    // 指定配置文件路径（测试隔离用）
    explicit ConfigStore(const QString& filePath);

    // 读取配置；文件不存在或内容无效时保持默认空状态
    void load();
    // 立即写盘。使用 QSaveFile 原子替换，不残留临时文件
    void save();

    const QList<QuickItem>& items() const { return m_items; }
    void addItem(const QuickItem& item);
    void updateItem(int row, const QuickItem& item);
    void removeItem(int row);

    QPoint ballPosition() const { return m_ballPos; }
    bool hasBallPosition() const { return m_hasBallPos; }
    void setBallPosition(const QPoint& pos);

private:
    QString m_filePath;
    QList<QuickItem> m_items;
    QPoint m_ballPos;
    bool m_hasBallPos = false;
};
