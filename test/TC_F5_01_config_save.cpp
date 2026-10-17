// TC-F5-01 变更写入配置文件（自动化部分）
// 覆盖：保存后 JSON 记录全部快捷项（名称、目标、类型键）与悬浮球坐标，
// 且目录中无临时文件或备份文件残留。
// "退出时状态为最新"的人工核对见 spec 用例（程序在每次变更后立即落盘）。
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "config_store.h"
#include "test_helpers.h"

namespace {
void run()
{
    const QString dir = makeTempDir("TC_F5_01");
    const QString path = dir + "/config.json";
    ConfigStore store(path);

    QuickItem program;
    program.name = QStringLiteral("画图");
    program.target = QStringLiteral("C:/Windows/System32/mspaint.exe");
    program.type = TargetType::Program;
    QuickItem script;
    script.name = QStringLiteral("测试脚本");
    script.target = dir + "/hello.bat";
    script.type = TargetType::Script;
    store.addItem(program);
    store.addItem(script);
    store.setBallPosition(QPoint(123, 456));
    store.save();

    QFile file(path);
    TC_CHECK(file.exists(), "配置文件应被创建");
    TC_CHECK(file.open(QIODevice::ReadOnly), "配置文件应可读");
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    TC_CHECK(doc.isObject(), "配置文件应为 JSON 对象");
    const QJsonObject root = doc.object();

    const QJsonObject ball = root.value(QLatin1String("ball")).toObject();
    TC_CHECK(ball.value(QLatin1String("x")).toInt() == 123
                 && ball.value(QLatin1String("y")).toInt() == 456,
             "悬浮球坐标应正确落盘");

    const QJsonArray items = root.value(QLatin1String("items")).toArray();
    TC_CHECK(items.size() == 2, "快捷项数量应为 2");
    TC_CHECK(items.at(0).toObject().value(QLatin1String("name")).toString()
                 == QStringLiteral("画图"),
             "第 1 项名称应正确");
    TC_CHECK(items.at(0).toObject().value(QLatin1String("target")).toString()
                 == QStringLiteral("C:/Windows/System32/mspaint.exe"),
             "第 1 项目标应正确");
    TC_CHECK(items.at(0).toObject().value(QLatin1String("type")).toString()
                 == QStringLiteral("program"),
             "第 1 项类型键应为 program");
    TC_CHECK(items.at(1).toObject().value(QLatin1String("type")).toString()
                 == QStringLiteral("script"),
             "第 2 项类型键应为 script");

    // 无临时文件或备份文件残留
    const QStringList files = QDir(dir).entryList(QDir::Files);
    TC_CHECK(files.size() == 1 && files.first() == QStringLiteral("config.json"),
             "目录中不应残留临时或备份文件");
}
} // namespace
REGISTER_TEST("TC-F5-01", run);
