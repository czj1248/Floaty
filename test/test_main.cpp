// 自动化测试入口：依次运行所有注册用例，全部通过返回 0，任一失败返回 1
#include <QGuiApplication>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

#include "test_helpers.h"

int main(int argc, char* argv[])
{
#ifdef _WIN32
    ::SetConsoleOutputCP(CP_UTF8); // 控制台按 UTF-8 输出中文
#endif

    QGuiApplication app(argc, argv);

    int passed = 0;
    int failed = 0;
    for (const TestCaseEntry& entry : testRegistry()) {
        try {
            entry.fn();
            ++passed;
            std::printf("[通过] %s\n", entry.id);
        } catch (const std::exception& e) {
            ++failed;
            std::printf("[失败] %s: %s\n", entry.id, e.what());
        }
        std::fflush(stdout);
    }
    std::printf("合计: %d 通过, %d 失败\n", passed, failed);
    std::fflush(stdout);
    return failed == 0 ? 0 : 1;
}
