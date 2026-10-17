// TC-F5-03 首次启动（无配置文件）（自动化部分）
// 覆盖：配置文件不存在时以空快捷项、无记忆位置的默认状态开始；
// 随后写入默认位置并保存时，配置文件从无到有被创建。
// 实际启动主程序后的默认位置与空面板为人工观察项。
#include <QFileInfo>

#include "config_store.h"
#include "test_helpers.h"

namespace {
void run()
{
    const QString dir = makeTempDir("TC_F5_03");
    const QString path = dir + "/config.json";
    TC_CHECK(!QFileInfo::exists(path), "前置：配置文件不应存在");

    ConfigStore store(path);
    store.load();
    TC_CHECK(store.items().isEmpty(), "首次启动快捷项列表应为空");
    TC_CHECK(!store.hasBallPosition(), "首次启动不应有记忆位置");

    // 主程序随后以屏幕默认位置写入并保存，配置文件从无到有
    store.setBallPosition(QPoint(900, 540));
    store.save();
    TC_CHECK(QFileInfo::exists(path), "保存后配置文件应被创建");
}
} // namespace
REGISTER_TEST("TC-F5-03", run);
