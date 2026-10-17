cmake_minimum_required(VERSION 3.16)
project(Floaty VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

# 核心逻辑源码：主程序与自动化测试共用
set(FLOATY_CORE_SOURCES
    src/types.cpp
    src/config_store.cpp
    src/launch_service.cpp
    src/window_layout.cpp
    src/single_instance.cpp
)

# 界面源码：仅主程序使用
set(FLOATY_UI_SOURCES
    src/icon_provider.cpp
    src/floating_ball.cpp
    src/expand_panel.cpp
    src/manager_dialog.cpp
)

add_executable(Floaty WIN32
    src/main.cpp
    ${FLOATY_CORE_SOURCES}
    ${FLOATY_UI_SOURCES}
)
target_include_directories(Floaty PRIVATE src)
# MSVC 源码与执行字符集统一为 UTF-8，保证中文字符串正确
target_compile_options(Floaty PRIVATE /utf-8)
target_link_libraries(Floaty PRIVATE Qt6::Widgets)

# 自动化测试工程：每个 TC_*.cpp 对应 spec/main_test_cases.md 中的一条用例
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
