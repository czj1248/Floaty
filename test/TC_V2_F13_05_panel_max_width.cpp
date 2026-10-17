// TC-V2-F13-05 面板最大宽度设置项（V2.1 新增）
// 范围 80-480、步进 20、默认 320；配置往返持久化；
// 面板宽度计算按该设置值作为上限夹取，短内容仍不小于面板最小宽度
#include "config_store.h"
#include "test_helpers.h"
#include "theme.h"
#include "window_layout.h"

namespace {
void run()
{
    // 规格常量自检：下限等于面板最小宽度，上限与步进为约定规格
    TC_CHECK(theme::kPanelMaxWidthMin == theme::kPanelMinWidth,
             "最大宽度可调下限应等于面板最小宽度");
    TC_CHECK(theme::kPanelMaxWidthMax == 480 && theme::kPanelMaxWidthStep == 20
                  && theme::kDefaultPanelMaxWidth == 320,
             "最大宽度应为 80-480、步进 20、默认 320");
    TC_CHECK((theme::kPanelMaxWidthMax - theme::kPanelMaxWidthMin) % theme::kPanelMaxWidthStep
                  == 0,
             "调节区间应被步进整除");

    const QString path = makeTempDir("TC_V2_F13_05") + "/config.json";
    {
        ConfigStore store(path);
        Appearance next;
        next.panelMaxWidth = theme::kPanelMaxWidthMax;
        store.setAppearance(next);
        store.save();
    }
    {
        ConfigStore reader(path);
        reader.load();
        TC_CHECK(reader.appearance().panelMaxWidth == 480, "最大宽度 480 应持久化");
    }

    // 宽度上限夹取：超长内容分别按下限 / 240 / 480 封顶
    TC_CHECK(window_layout::calcPanelWidth(1000, theme::kPanelMinWidth,
                                           theme::kPanelMaxWidthMin)
                 == theme::kPanelMaxWidthMin,
             "上限为下限时超长内容封顶到下限");
    TC_CHECK(window_layout::calcPanelWidth(1000, theme::kPanelMinWidth, 240) == 240,
             "上限 240 时超长内容封顶 240");
    TC_CHECK(window_layout::calcPanelWidth(1000, theme::kPanelMinWidth, 480) == 480,
             "上限 480 时超长内容封顶 480");

    // 短内容在上限为下限时：内容+24 超过上限则封顶到上限
    TC_CHECK(window_layout::calcPanelWidth(100, theme::kPanelMinWidth,
                                           theme::kPanelMaxWidthMin)
                 == theme::kPanelMaxWidthMin,
             "短内容在上限为下限时，超过上限则封顶到上限");
}
} // namespace
REGISTER_TEST("TC-V2-F13-05", run);
