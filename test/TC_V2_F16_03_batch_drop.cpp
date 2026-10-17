// TC-V2-F16-03 多文件批量拖放分流
// 多个白名单文件与文件夹全部接受；结果保持原始顺序
#include <QDir>

#include "drop_service.h"
#include "test_helpers.h"

namespace {
void run()
{
    const QString dir = makeTempDir("TC_V2_F16_03");
    const QString f1 = dir + "/drop1.txt";
    const QString f2 = dir + "/drop2.txt";
    const QString f3 = dir + "/drop3.txt";
    const QString folder = dir + "/subfolder";
    TC_CHECK(writeTextFile(f1, "1"), "前置：创建 drop1");
    TC_CHECK(writeTextFile(f2, "2"), "前置：创建 drop2");
    TC_CHECK(writeTextFile(f3, "3"), "前置：创建 drop3");
    TC_CHECK(QDir().mkpath(folder), "前置：创建文件夹");

    const QStringList input = {f1, folder, f2, f3};
    const QList<drop_service::DropVerdict> verdicts =
        drop_service::classifyDroppedPaths(input);
    const QStringList accepted = drop_service::acceptedPaths(verdicts);
    TC_CHECK(accepted.size() == 4, "三个文本文件与一个文件夹应全部接受");
    TC_CHECK(accepted == input, "批量结果应保持拖入顺序");
    TC_CHECK(drop_service::rejectedReasons(verdicts).isEmpty(), "不应有拒绝项");
    TC_CHECK(verdicts.at(1).accepted && verdicts.at(1).path == folder, "文件夹应接受");
}
} // namespace
REGISTER_TEST("TC-V2-F16-03", run);
