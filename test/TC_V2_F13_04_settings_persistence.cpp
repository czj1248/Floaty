// TC-V2-F13-04 外观设置持久化（V2.1：三项外观）
// 默认外观 48/88/320；改为 56/0/240 写盘后重启一致
#include "config_store.h"
#include "test_helpers.h"
#include "theme.h"

namespace {
void run()
{
    const QString path = makeTempDir("TC_V2_F13_04") + "/config.json";
    {
        ConfigStore store(path);
        TC_CHECK(store.appearance().ballSize == theme::kDefaultBallSize, "默认球尺寸应为 48");
        TC_CHECK(store.appearance().opacity == theme::kDefaultOpacity, "默认不透明度应为 88%");
        TC_CHECK(store.appearance().panelMaxWidth == theme::kDefaultPanelMaxWidth,
                 "默认面板最大宽度应为 320");

        Appearance next;
        next.ballSize = theme::kBallSizeLarge;
        next.opacity = 0; // V2.1：不透明度下限为 0
        next.panelMaxWidth = 240;
        store.setAppearance(next);
        store.save();
    }
    {
        ConfigStore reader(path);
        reader.load();
        TC_CHECK(reader.appearance().ballSize == 56, "球尺寸 56 应持久化");
        TC_CHECK(reader.appearance().opacity == 0, "不透明度 0% 应持久化（V2.1 下限）");
        TC_CHECK(reader.appearance().panelMaxWidth == 240, "面板最大宽度 240 应持久化");
    }
}
} // namespace
REGISTER_TEST("TC-V2-F13-04", run);
