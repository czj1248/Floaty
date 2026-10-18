#include "expand_panel.h"

#include <QApplication>
#include <QCursor>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QElapsedTimer>
#include <QEvent>
#include <QFileInfo>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QScrollArea>
#include <QScrollBar>
#include <QShowEvent>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <functional>

#include "config_store.h"
#include "icon_provider.h"
#include "quick_add.h"
#include "theme.h"
#include "window_layout.h"

namespace floaty_ui {

// 空状态提示文案（宽度测量与控件展示共用）
constexpr const char* kEmptyHintText = "暂无快捷项，可拖入文件添加";

// 跑马灯文字：平时原位截断绘制，仅在所属行被悬停且文字超长时由面板定时器驱动滚动
class MarqueeLabel : public QWidget {
public:
    explicit MarqueeLabel(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TranslucentBackground);
    }

    void setText(const QString& text)
    {
        m_text = text;
        recalc();
        update();
    }

    void setAvailableWidth(int width)
    {
        m_avail = width;
        update();
    }

    bool textFits() const { return m_textWidth <= m_avail; }

    // 悬停进入：开始一轮"停顿 - 左移 - 回位"循环
    void startScroll()
    {
        m_active = true;
        m_clock.restart();
        m_shift = 0;
        update();
    }

    // 悬停离开：立即复位，不做滚动
    void stopScroll()
    {
        m_active = false;
        m_shift = 0;
        update();
    }

    void tick()
    {
        if (!m_active || textFits()) {
            return;
        }
        const int elapsed = m_clock.elapsed();
        const int holdMs = 500;
        const int speedPxPerSec = 45;
        const int cycle = m_textWidth + kTrail;
        if (elapsed < holdMs) {
            m_shift = 0;
        } else {
            m_shift = ((elapsed - holdMs) * speedPxPerSec / 1000) % (cycle + holdMs);
            if (m_shift > cycle) {
                // 一轮结束后短暂停顿再重新开始
                m_shift = 0;
                m_clock.restart();
            }
        }
        update();
    }

protected:
    void paintEvent(QPaintEvent* /*event*/) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::TextAntialiasing);
        p.setFont(font());
        p.setPen(theme::kTextMain);
        p.setClipRect(rect());
        const int y = (height() - QFontMetrics(font()).height()) / 2
                      + QFontMetrics(font()).ascent();
        // 两份文字首尾相接实现连续跑马灯；不滚动时第二份在裁剪区外
        p.drawText(QPoint(-m_shift, y), m_text);
        if (m_active && !textFits()) {
            p.drawText(QPoint(m_textWidth + kTrail - m_shift, y), m_text);
        }
    }

private:
    void recalc() { m_textWidth = QFontMetrics(font()).horizontalAdvance(m_text); }

    static constexpr int kTrail = 28; // 循环首尾间隔
    QString m_text;
    int m_textWidth = 0;
    int m_avail = 0;
    int m_shift = 0;
    bool m_active = false;
    QElapsedTimer m_clock;
};

// 分组标题行：主色加粗文字，双击无反应，可作为拖放落点
class GroupHeaderRow : public QWidget {
public:
    GroupHeaderRow(const QString& groupId, const QString& name, QWidget* parent = nullptr)
        : QWidget(parent)
        , m_name(name)
    {
        setFixedHeight(22);
        setProperty("groupId", groupId);
        setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    void paintEvent(QPaintEvent* /*event*/) override
    {
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        if (property("dropTarget").toBool()) {
            // 拖放悬停的分组：浅绿底强调
            p.setPen(Qt::NoPen);
            p.setBrush(theme::kPrimaryLight);
            p.drawRoundedRect(rect().adjusted(0, 1, 0, -1), theme::kRadiusControl,
                              theme::kRadiusControl);
        }
        QFont f = font();
        f.setBold(true);
        p.setFont(f);
        p.setPen(theme::kPrimary);
        const QFontMetrics fm(f);
        // 超过行可用宽度的标题以尾部省略号截断（F12：标题不做跑马灯）
        const QString shown = fm.elidedText(m_name, Qt::ElideRight, width() - 4);
        const int y = (height() - fm.height()) / 2 + fm.ascent();
        p.drawText(QPoint(2, y), shown);
    }

private:
    QString m_name;
};

// 快捷项行：系统图标 + 名称，双击启动；悬停时浅绿底，名称超长交给跑马灯
class ItemRow : public QWidget {
public:
    ItemRow(const QuickItem& item, const QIcon& icon, int textAvailWidth, QWidget* parent = nullptr)
        : QWidget(parent)
        , m_itemId(item.id)
    {
        setFixedHeight(theme::kPanelRowHeight);
        setProperty("groupId", item.groupId);
        setAttribute(Qt::WA_TranslucentBackground);

        auto* iconLabel = new QLabel(this);
        iconLabel->setFixedSize(20, 20);
        iconLabel->setPixmap(icon.pixmap(20, 20));
        iconLabel->setAttribute(Qt::WA_TranslucentBackground);

        m_text = new MarqueeLabel(this);
        m_text->setText(item.name);
        m_text->setAvailableWidth(textAvailWidth);

        auto* row = new QHBoxLayout(this);
        row->setContentsMargins(2, 0, 2, 0);
        row->setSpacing(8);
        row->addWidget(iconLabel);
        row->addWidget(m_text, 1);
    }

    QString itemId() const { return m_itemId; }
    MarqueeLabel* marquee() const { return m_text; }

    void setHoverCallback(const std::function<void(ItemRow*, bool)>& cb) { m_hoverCb = cb; }
    void setActivatedCallback(const std::function<void(const QString&)>& cb) { m_activatedCb = cb; }

protected:
    void paintEvent(QPaintEvent* /*event*/) override
    {
        if (!m_hovered) {
            return;
        }
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(theme::kPrimaryLight);
        p.drawRoundedRect(rect().adjusted(0, 1, 0, -1), theme::kRadiusControl,
                          theme::kRadiusControl);
    }

    void enterEvent(QEnterEvent* /*event*/) override
    {
        m_hovered = true;
        update();
        if (m_hoverCb) {
            m_hoverCb(this, true);
        }
    }

    void leaveEvent(QEvent* /*event*/) override
    {
        m_hovered = false;
        update();
        if (m_hoverCb) {
            m_hoverCb(this, false);
        }
    }

    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton && m_activatedCb) {
            m_activatedCb(m_itemId);
        }
    }

private:
    QString m_itemId;
    MarqueeLabel* m_text;
    bool m_hovered = false;
    std::function<void(ItemRow*, bool)> m_hoverCb;
    std::function<void(const QString&)> m_activatedCb;
};

// 空列表提示
class EmptyStateWidget : public QWidget {
public:
    explicit EmptyStateWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setFixedHeight(118);
        auto* icon = new QLabel(this);
        icon->setPixmap(QIcon(QStringLiteral(":/icons/empty_state.svg")).pixmap(40, 40));
        icon->setAlignment(Qt::AlignCenter);
        icon->setAttribute(Qt::WA_TranslucentBackground);

        auto* hint = new QLabel(QString::fromUtf8(kEmptyHintText), this);
        hint->setAlignment(Qt::AlignCenter);
        hint->setWordWrap(true);
        hint->setStyleSheet(QStringLiteral("color: %1;").arg(theme::kTextSub.name()));

        auto* col = new QVBoxLayout(this);
        col->setContentsMargins(0, 14, 0, 14);
        col->setSpacing(8);
        col->addWidget(icon);
        col->addWidget(hint);
    }
};

} // namespace floaty_ui

ExpandPanel::ExpandPanel(ConfigStore& store, QWidget* parent)
    : QWidget(parent)
    , m_store(store)
    , m_marqueeTimer(new QTimer(this))
{
    setObjectName(QStringLiteral("expandPanel"));
    // 工具窗口而非 Qt::Popup：避免 Popup 在鼠标按下时 show 会立即自关、
    // 以及 WA_NoMouseReplay 导致点击悬浮球无法收回的问题；外部点击关闭由 eventFilter 自行管理
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);

    m_container = new QWidget;
    m_container->setAttribute(Qt::WA_TranslucentBackground);
    m_containerLayout = new QVBoxLayout(m_container);
    m_containerLayout->setContentsMargins(12, 6, 12, 6);
    m_containerLayout->setSpacing(0);
    m_containerLayout->addStretch(1);

    m_scroll = new QScrollArea(this);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 垂直滚动条始终隐藏、不参与布局：宽度与名称可用宽度不随内容高度变化（F12）
    m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setWidgetResizable(true);
    m_scroll->setWidget(m_container);
    m_scroll->setAttribute(Qt::WA_TranslucentBackground);
    m_scroll->viewport()->setAttribute(Qt::WA_TranslucentBackground);
    m_scroll->viewport()->setAutoFillBackground(false);

    // 悬浮滚动条：视口子控件，几何上贴右缘内侧，覆盖在容器内边距与行缘区域之上
    QScrollBar* internalBar = m_scroll->verticalScrollBar();
    m_overlayBar = new QScrollBar(Qt::Vertical, m_scroll->viewport());
    m_overlayBar->setProperty("overlay", true);
    m_overlayBar->hide();
    connect(internalBar, &QScrollBar::rangeChanged, this, [this](int min, int max) {
        const QScrollBar* src = m_scroll->verticalScrollBar();
        m_overlayBar->setRange(min, max);
        m_overlayBar->setPageStep(src->pageStep());
        m_overlayBar->setSingleStep(src->singleStep());
        m_overlayBar->setVisible(max > min);
        positionOverlayBar();
        m_overlayBar->raise();
    });
    connect(internalBar, &QScrollBar::valueChanged, m_overlayBar, &QScrollBar::setValue);
    connect(m_overlayBar, &QScrollBar::valueChanged, internalBar, &QScrollBar::setValue);
    m_scroll->viewport()->installEventFilter(this);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(m_scroll);

    // 仅悬停行的跑马灯占用定时器，行离开即停（静置零 CPU）
    m_marqueeTimer->setInterval(30);
    connect(m_marqueeTimer, &QTimer::timeout, this, [this]() {
        if (m_hoveredRow) {
            m_hoveredRow->marquee()->tick();
        }
    });
}

void ExpandPanel::reload()
{
    // 清空旧内容（stretch 保留在最后）
    while (m_containerLayout->count() > 1) {
        QLayoutItem* item = m_containerLayout->takeAt(0);
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    m_hoveredRow = nullptr;
    m_marqueeTimer->stop();
    clearDropTarget();

    const QList<QuickItem> allItems = m_store.items();
    const int maxWidth = effectiveMaxWidth();

    // 第一轮：测量最宽内容，确定面板宽度（F12）
    const QFontMetrics fm(font());
    QFont boldFont = font();
    boldFont.setBold(true);
    const QFontMetrics boldFm(boldFont);

    if (allItems.isEmpty()) {
        m_containerLayout->insertWidget(0, new floaty_ui::EmptyStateWidget(m_container));
        // 空状态同样按内容计算：图标 40px 与提示文字取宽，含行左右各 2px 边距
        const int emptyContent =
            2 + qMax(40, fm.horizontalAdvance(QString::fromUtf8(floaty_ui::kEmptyHintText))) + 2;
        m_panelWidth = window_layout::calcPanelWidth(emptyContent, theme::kPanelMinWidth, maxWidth);
        m_contentHeight = 118 + 12;
        return;
    }

    constexpr int kRowEdge = 2;  // 行左右缘各 2px
    constexpr int kIconW = 20;   // 图标宽
    constexpr int kIconGap = 8;  // 图标与文字间距
    int maxContent = 0;
    const QList<QuickGroup> groups = m_store.visibleGroups();
    for (const QuickGroup& g : groups) {
        maxContent = qMax(maxContent, kRowEdge + boldFm.horizontalAdvance(g.name) + kRowEdge);
        const QList<QuickItem> groupItems = m_store.itemsInGroup(g.id);
        for (const QuickItem& item : groupItems) {
            maxContent = qMax(maxContent,
                              kRowEdge + kIconW + kIconGap + fm.horizontalAdvance(item.name)
                                  + kRowEdge);
        }
    }
    m_panelWidth = window_layout::calcPanelWidth(maxContent, theme::kPanelMinWidth, maxWidth);
    const int textAvail = window_layout::rowTextAvailableWidth(m_panelWidth, 20);

    // 第二轮：按分组分区构建
    int contentHeight = 12; // 上下各 6
    int insertIndex = 0;
    for (int gi = 0; gi < groups.size(); ++gi) {
        const QuickGroup& group = groups.at(gi);
        const QList<QuickItem> groupItems = m_store.itemsInGroup(group.id);

        auto* header = new floaty_ui::GroupHeaderRow(group.id, group.name, m_container);
        m_containerLayout->insertWidget(insertIndex++, header);
        contentHeight += 22;

        for (const QuickItem& item : groupItems) {
            auto* row = new floaty_ui::ItemRow(item, iconForTarget(item), textAvail, m_container);
            row->setHoverCallback([this](ItemRow* r, bool entered) { setHoveredRow(entered ? r : nullptr); });
            row->setActivatedCallback([this](const QString& id) { launchItem(id); });
            m_containerLayout->insertWidget(insertIndex++, row);
            contentHeight += theme::kPanelRowHeight;
        }

        if (gi + 1 < groups.size()) {
            m_containerLayout->insertSpacing(insertIndex++, 8);
            contentHeight += 8; // 组间 8px 间隔
        }
    }
    m_contentHeight = contentHeight;
}

void ExpandPanel::popupForBall(const QRect& ballGlobal)
{
    m_ballGeometry = ballGlobal;
    reload();
    applyGeometry();
    show();
}

void ExpandPanel::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    // 展开后主动获取焦点，使 focusOutEvent 能感知外部点击
    setFocus();
    activateWindow();
    raise();
}

void ExpandPanel::focusOutEvent(QFocusEvent* event)
{
    QWidget::focusOutEvent(event);
    // 焦点转移到面板内部子控件（如悬浮滚动条）时不关闭
    QWidget* focus = QApplication::focusWidget();
    if (focus && isAncestorOf(focus)) {
        return;
    }
    // 点击悬浮球导致的失焦：交 togglePanel 处理，避免先关后开
    if (m_ballGeometry.contains(QCursor::pos())) {
        return;
    }
    close();
}

int ExpandPanel::effectiveMaxWidth() const
{
    return m_previewMaxWidth >= 0 ? m_previewMaxWidth : m_store.appearance().panelMaxWidth;
}

void ExpandPanel::setPreviewMaxWidth(int maxWidthPx)
{
    m_previewMaxWidth = maxWidthPx;
    if (isVisible()) {
        reload();
        applyGeometry();
    }
}

void ExpandPanel::applyGeometry()
{
    // F15：以球当前所在显示器为准计算展开几何
    QList<QRect> screenGeoms;
    const QList<QScreen*> screens = QGuiApplication::screens();
    for (QScreen* s : screens) {
        screenGeoms.append(s->availableGeometry());
    }
    const int idx = window_layout::screenIndexAt(screenGeoms, m_ballGeometry.center());
    const QRect scr = idx >= 0 ? screenGeoms.at(idx)
                               : QGuiApplication::primaryScreen()->availableGeometry();

    const int height = qMin(scr.height() * 2 / 3, m_contentHeight);
    resize(m_panelWidth, height);
    move(window_layout::calcPanelTopLeft(m_ballGeometry, size(), scr, theme::kSnapGap));
}

bool ExpandPanel::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_scroll->viewport() && event->type() == QEvent::Resize) {
        positionOverlayBar();
    }
    return QWidget::eventFilter(watched, event);
}

void ExpandPanel::positionOverlayBar()
{
    constexpr int kBarWidth = 8;
    QWidget* vp = m_scroll->viewport();
    m_overlayBar->setGeometry(vp->width() - kBarWidth - 2, 2, kBarWidth, vp->height() - 4);
}

void ExpandPanel::launchItem(const QString& itemId)
{
    for (const QuickItem& item : m_store.items()) {
        if (item.id != itemId) {
            continue;
        }
        const LaunchResult result = m_launcher.launch(item);
        // 启动后面板收起；失败给出明确提示且不崩溃
        close();
        if (result != LaunchResult::Success) {
            QMessageBox::warning(this, QStringLiteral("浮启"),
                                 LaunchService::failureMessage(result, item));
        }
        return;
    }
}

void ExpandPanel::setHoveredRow(ItemRow* row)
{
    if (m_hoveredRow == row) {
        if (row == nullptr) {
            return;
        }
        // 同一行重复 enter：重启跑马灯计时
    }
    if (m_hoveredRow && m_hoveredRow != row) {
        m_hoveredRow->marquee()->stopScroll();
    }
    m_hoveredRow = row;
    if (row && !row->marquee()->textFits()) {
        row->marquee()->startScroll();
        if (!m_marqueeTimer->isActive()) {
            m_marqueeTimer->start();
        }
    } else {
        if (row) {
            row->marquee()->stopScroll();
        }
        m_marqueeTimer->stop();
    }
}

void ExpandPanel::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int opacityPercent = m_previewOpacity >= 0 ? m_previewOpacity
                                                     : m_store.appearance().opacity;
    QColor bg = theme::kPanelBg;
    bg.setAlpha(opacityPercent * 255 / 100);

    QPainterPath path;
    path.addRoundedRect(rect().adjusted(1, 1, -1, -1), theme::kRadiusWindow,
                        theme::kRadiusWindow);
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawPath(path);

    // 描边：常态浅灰，拖放悬停时主色强调（F16）
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(m_dropActive ? theme::kPrimary : theme::kBorder, m_dropActive ? 2 : 1));
    p.drawPath(path);
}

void ExpandPanel::dragEnterEvent(QDragEnterEvent* event)
{
    if (!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }
    event->acceptProposedAction();
    m_dropActive = true;
    update();
}

void ExpandPanel::dragMoveEvent(QDragMoveEvent* event)
{
    QWidget* targetWidget = nullptr;
    QWidget* w = childAt(event->position().toPoint());
    while (w && w != this) {
        if (w->property("groupId").isValid()) {
            targetWidget = w;
            break;
        }
        w = w->parentWidget();
    }
    if (m_dropTarget != targetWidget) {
        clearDropTarget();
        m_dropTarget = targetWidget;
        if (m_dropTarget) {
            m_dropTarget->setProperty("dropTarget", true);
            m_dropTarget->update();
        }
    }
    event->acceptProposedAction();
}

void ExpandPanel::dragLeaveEvent(QDragLeaveEvent* /*event*/)
{
    m_dropActive = false;
    clearDropTarget();
    update();
}

void ExpandPanel::dropEvent(QDropEvent* event)
{
    QStringList paths;
    for (const QUrl& url : event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            paths.append(url.toLocalFile());
        }
    }
    const QString groupId = groupIdAt(event->position().toPoint());
    m_dropActive = false;
    clearDropTarget();
    update();

    if (paths.isEmpty()) {
        event->ignore();
        return;
    }
    event->acceptProposedAction();
    if (quickAddPaths(this, m_store, paths, groupId)) {
        reload(); // 拖入后面板保持展开并立即显示新项
    }
}

void ExpandPanel::clearDropTarget()
{
    if (m_dropTarget) {
        m_dropTarget->setProperty("dropTarget", false);
        m_dropTarget->update();
        m_dropTarget = nullptr;
    }
}

QString ExpandPanel::groupIdAt(const QPoint& panelPos) const
{
    QWidget* w = childAt(panelPos);
    while (w && w != this) {
        const QVariant group = w->property("groupId");
        if (group.isValid()) {
            return group.toString();
        }
        w = w->parentWidget();
    }
    // 面板空白区域：默认分组
    return QString::fromLatin1(kDefaultGroupId);
}
