// 图标生成工具：用 ball_logo 同一绘制函数渲染多尺寸 PNG，打包为 PNG 压缩的
// 多分辨率 ICO（resources/app.ico）。图标与软件 Logo 同源，Logo 调整后重新
// 构建并运行本工具即可再生成。
// 用法：make_icon <app.ico 输出路径>

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <QGuiApplication>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QRectF>

#include <cstdio>

#include "ball_logo.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::fprintf(stderr, "usage: make_icon <output.ico>\n");
        return 1;
    }

    QGuiApplication app(argc, argv);

    // Windows 资源管理器常用档位；256 档必须使用 PNG 压缩
    const int sizes[] = { 16, 24, 32, 48, 64, 128, 256 };
    const int count = static_cast<int>(sizeof(sizes) / sizeof(sizes[0]));

    QList<QByteArray> pngs;
    pngs.reserve(count);
    for (int side : sizes) {
        QImage img(side, side, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        QPainter p(&img);
        ball_logo::paintBallLogo(&p, QRectF(0, 0, side, side));
        p.end();

        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        if (!img.save(&buf, "PNG")) {
            std::fprintf(stderr, "PNG encode failed at size %d\n", side);
            return 2;
        }
        pngs.append(buf.data());
    }

    QFile out(QString::fromLocal8Bit(argv[1]));
    if (!out.open(QIODevice::WriteOnly)) {
        std::fprintf(stderr, "cannot open output: %s\n", argv[1]);
        return 3;
    }

    // ICO 为小端序：文件头 + 目录项数组 + 各尺寸 PNG 数据
    QDataStream s(&out);
    s.setByteOrder(QDataStream::LittleEndian);
    s << quint16(0) << quint16(1) << quint16(count);

    quint32 offset = 6 + 16u * count;
    for (int i = 0; i < count; ++i) {
        const quint8 dim = sizes[i] >= 256 ? 0 : static_cast<quint8>(sizes[i]); // 0 表示 256
        s << dim << dim;
        s << quint8(0) << quint8(0);      // 调色板色数 / 保留
        s << quint16(1) << quint16(32);   // 平面数 / 位深
        s << quint32(pngs[i].size()) << offset;
        offset += pngs[i].size();
    }
    for (const QByteArray& png : pngs) {
        out.write(png);
    }

    std::printf("wrote %s (%d sizes)\n", argv[1], count);
    return 0;
}
