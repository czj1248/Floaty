#include "expand_panel.h"

#include <QGuiApplication>
#include <QListWidget>
#include <QMessageBox>
#include <QScreen>
#include <QVBoxLayout>

#include "config_store.h"
#include "icon_provider.h"
#include "launch_service.h"
#include "window_layout.h"

namespace {
constexpr int kPanelWidth = 260;
constexpr int kRowHeight = 34;
constexpr int kPanelGap = 8; // 面板与悬浮球的间距
}

ExpandPanel::ExpandPanel(ConfigStore& store, QWidget* parent)
    : QWidget(parent)
    , m_store(store)
{
    setObjectName(QStringLiteral("expandPanel"));
    // 弹出式窗口：点击面板外自动收起（F2 验收标准第 2 条）
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    // 弹出层因点击外部关闭时，不向点击处回放鼠标按下事件，
    // 避免"再次单击悬浮球收起"（F2 验收标准第 3 条）被回放成重新展开
    setAttribute(Qt::WA_NoMouseReplay);

    m_list = new QListWidget(this);
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setIconSize(QSize(20, 20));
    // 仅双击启动（F3）：不连接 itemClicked
    connect(m_list, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem* it) { launchCurrent(it); });

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(0);
    layout->addWidget(m_list);

    setStyleSheet(QStringLiteral(
        "#expandPanel { background: #fafafa; border: 1px solid #c0c0c0; }"
        "QListWidget { background: #fafafa; }"
        "QListWidget::item { padding-left: 6px; border-radius: 3px; }"
        "QListWidget::item:selected { background: #dce9f9; color: #202020; }"));
}

void ExpandPanel::popupForBall(const QRect& ballGlobal)
{
    reload();
    resize(preferredSize());
    const QRect scr = QGuiApplication::primaryScreen()->availableGeometry();
    // 靠悬浮球上方展开，屏幕上缘空间不足时向下展开（F2 边界说明）
    move(window_layout::calcPanelTopLeft(ballGlobal, size(), scr, kPanelGap));
    show();
}

void ExpandPanel::reload()
{
    m_list->clear();
    const QList<QuickItem> items = m_store.items();
    if (items.isEmpty()) {
        QListWidgetItem* hint = new QListWidgetItem(m_list);
        hint->setText(QStringLiteral("暂无快捷项，右键悬浮球可添加"));
        hint->setFlags(Qt::NoItemFlags);
        hint->setSizeHint(QSize(0, kRowHeight));
        return;
    }
    for (int i = 0; i < items.size(); ++i) {
        const QuickItem& item = items.at(i);
        auto* row = new QListWidgetItem(m_list);
        row->setIcon(iconForTarget(item));
        row->setText(item.name);
        row->setToolTip(QStringLiteral("%1\n%2").arg(targetTypeName(item.type), item.target));
        row->setData(Qt::UserRole, i);
        row->setSizeHint(QSize(0, kRowHeight));
    }
}

void ExpandPanel::launchCurrent(QListWidgetItem* rowItem)
{
    if (!rowItem) {
        return;
    }
    const int index = rowItem->data(Qt::UserRole).toInt();
    const QList<QuickItem> items = m_store.items();
    if (index < 0 || index >= items.size()) {
        return;
    }
    const QuickItem item = items.at(index);
    const LaunchResult result = m_launcher.launch(item);
    // 启动后面板收起（F3 验收标准第 7 条）；失败时给出明确提示且不崩溃（第 8 条）
    close();
    if (result != LaunchResult::Success) {
        QMessageBox::warning(this, QStringLiteral("Floaty"),
                             LaunchService::failureMessage(result, item));
    }
}

QSize ExpandPanel::preferredSize() const
{
    const QRect scr = QGuiApplication::primaryScreen()->availableGeometry();
    const int rows = qMax(1, m_list->count());
    // 面板高度封顶为屏幕可用高度的三分之二，超出部分列表内部滚动（F2 验收标准第 4 条）
    const int height = qMin(scr.height() * 2 / 3, rows * kRowHeight + 2);
    return QSize(kPanelWidth, height);
}
