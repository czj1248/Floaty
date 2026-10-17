cmake_minimum_required(VERSION 3.16)
project(Floaty VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 本项目为纯 Widgets 桌面应用，不使用 Vulkan 渲染；加 QUIET 可消除
# Qt6Gui 触发的 "Could NOT find WrapVulkanHeaders" 状态提示（未安装 Vulkan SDK 时）
find_package(Qt6 REQUIRED QUIET COMPONENTS Widgets Svg)
qt_standard_project_setup()

# 核心逻辑源码：主程序与自动化测试共用
set(FLOATY_CORE_SOURCES
    src/types.cpp
    src/config_store.cpp
    src/launch_service.cpp
    src/window_layout.cpp
    src/single_instance.cpp
    src/drop_service.cpp
)

# 界面源码：仅主程序使用
set(FLOATY_UI_SOURCES
    src/theme.cpp
    src/ball_logo.cpp
    src/icon_provider.cpp
    src/quick_add.cpp
    src/settings_dialog.cpp
    src/floating_ball.cpp
    src/expand_panel.cpp
    src/manager_dialog.cpp
)

qt_add_resources(FLOATY_RESOURCES src/resources.qrc)

add_executable(Floaty WIN32
    src/main.cpp
    ${FLOATY_CORE_SOURCES}
    ${FLOATY_UI_SOURCES}
    ${FLOATY_RESOURCES}
)
target_include_directories(Floaty PRIVATE src)
# MSVC 源码与执行字符集统一为 UTF-8，保证中文字符串正确
target_compile_options(Floaty PRIVATE /utf-8)
target_link_libraries(Floaty PRIVATE Qt6::Widgets Qt6::Svg)

# 自动化测试工程：每个 TC_*.cpp 对应测试用例规格中的一条用例
enable_testing()
add_executable(FloatyTests
    test/test_main.cpp
    test/TC_F1_03_ball_clamp.cpp
    test/TC_F2_05_panel_direction.cpp
    test/TC_F3_03_launch_script.cpp
    test/TC_F3_09_target_missing.cpp
    test/TC_F4_02_detect_file_type.cpp
    test/TC_F4_03_url_target.cpp
    test/TC_F4_04_folder_target.cpp
    test/TC_F4_05_whitelist.cpp
    test/TC_F5_01_config_save.cpp
    test/TC_F5_02_config_load.cpp
    test/TC_F5_03_first_launch.cpp
    test/TC_NF_03_single_instance.cpp
    test/TC_V2_F10_01_group_assign.cpp
    test/TC_V2_F10_02_empty_group_hidden.cpp
    test/TC_V2_F10_04_change_group.cpp
    test/TC_V2_F10_05_delete_group.cpp
    test/TC_V2_F10_06_default_protected.cpp
    test/TC_V2_F10_07_rename_group.cpp
    test/TC_V2_F11_01_reorder_move.cpp
    test/TC_V2_F11_02_move_buttons.cpp
    test/TC_V2_F11_03_order_persistence.cpp
    test/TC_V2_F12_02_adaptive_width.cpp
    test/TC_V2_F12_03_max_width.cpp
    test/TC_V2_F12_05_marquee_gate.cpp
    test/TC_V2_F13_04_settings_persistence.cpp
    test/TC_V2_F13_05_panel_max_width.cpp
    test/TC_V2_F14_01_edge_snap.cpp
    test/TC_V2_F15_01_screen_at.cpp
    test/TC_V2_F16_03_batch_drop.cpp
    test/TC_V2_F16_04_drop_reject.cpp
    test/TC_V2_F16_05_mixed_drop.cpp
    test/TC_V2_CFG_02_v2_roundtrip.cpp
    ${FLOATY_CORE_SOURCES}
)
target_include_directories(FloatyTests PRIVATE src test)
target_compile_options(FloatyTests PRIVATE /utf-8)
target_link_libraries(FloatyTests PRIVATE Qt6::Widgets)
add_test(NAME FloatyTests COMMAND FloatyTests)

# 构建后用 windeployqt 部署 Qt 运行库，形成绿色免安装目录
get_filename_component(QT_BIN_DIR "${CMAKE_PREFIX_PATH}" PATH)
get_filename_component(QT_BIN_DIR "${QT_BIN_DIR}" PATH)
set(QT_BIN_DIR "${QT_BIN_DIR}/bin")
find_program(WINDEPLOYQT_EXE windeployqt HINTS "${QT_BIN_DIR}")
if(WINDEPLOYQT_EXE)
    add_custom_command(TARGET Floaty POST_BUILD
        COMMAND "${WINDEPLOYQT_EXE}" --no-translations "$<TARGET_FILE:Floaty>"
        COMMENT "正在部署 Qt 运行库到 Floaty 输出目录"
    )
    add_custom_command(TARGET FloatyTests POST_BUILD
        COMMAND "${WINDEPLOYQT_EXE}" --no-translations "$<TARGET_FILE:FloatyTests>"
        COMMENT "正在部署 Qt 运行库到 FloatyTests 输出目录"
    )
endif()
