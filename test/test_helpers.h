#pragma once

// 自动化测试公共辅助（可复用）：极简用例注册与断言，无第三方依赖。
// 每个 TC_*.cpp 通过 REGISTER_TEST 自注册用例，由 test_main.cpp 统一运行，
// 新增用例只需在 test/ 下新建文件并在 CMakeLists.txt 加入源码列表。

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QThread>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

struct TestCaseEntry {
    const char* id;
    void (*fn)();
};

inline std::vector<TestCaseEntry>& testRegistry()
{
    static std::vector<TestCaseEntry> registry;
    return registry;
}

#define REGISTER_TEST(id, fn)                     \
    static bool s_registered_##fn = []() {        \
        testRegistry().push_back({id, fn});       \
        return true;                              \
    }()

// 断言失败时抛出，中止当前用例
struct TcFailure : std::runtime_error {
    explicit TcFailure(const std::string& msg) : std::runtime_error(msg) {}
};

#define TC_CHECK(cond, msg)                                       \
    do {                                                          \
        if (!(cond)) {                                            \
            throw TcFailure(std::string("断言失败: ") + (msg));    \
        }                                                         \
    } while (false)

// 准备一个专属临时工作目录（每次先清空再创建，保证用例可重复执行）
inline QString makeTempDir(const QString& tag)
{
    const QString path = QDir::tempPath() + QStringLiteral("/floaty_auto/") + tag;
    QDir dir(path);
    if (dir.exists()) {
        dir.removeRecursively();
    }
    dir.mkpath(".");
    return path;
}

// 轮询等待条件成立，期间保持事件循环运转
inline bool waitFor(const std::function<bool()>& cond, int timeoutMs)
{
    QElapsedTimer timer;
    timer.start();
    while (!cond()) {
        if (timer.elapsed() > timeoutMs) {
            return false;
        }
        QCoreApplication::processEvents();
        QThread::msleep(20);
    }
    return true;
}

// 写一个文本文件（UTF-8）
inline bool writeTextFile(const QString& path, const QString& content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return file.write(content.toUtf8()) >= 0;
}
