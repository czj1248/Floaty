#include "manager_dialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include <QUrl>
#include <QVBoxLayout>

#include "config_store.h"
#include "icon_provider.h"
#include "theme.h"

namespace floaty_ui {

// 根据目标生成默认名称：文件取主名，文件夹取目录名，网址取主机名
QString defaultNameForTarget(const QString& target, TargetType type)
{
    if (type == TargetType::Url) {
        const QString host = QUrl(target).host();
        return host.isEmpty() ? target : host;
    }
    if (type == TargetType::Folder) {
        return QFileInfo(target).fileName();
    }
    return QFileInfo(target).completeBaseName();
}

// 右栏单行卡片：系统图标 + 名称 + 目标摘要
class ItemCard : public QWidget {
public:
    ItemCard(const QIcon& icon, const QString& name, const QString& sub, QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* iconLabel = new QLabel(this);
        iconLabel->setFixedSize(20, 20);
        iconLabel->setPixmap(icon.pixmap(20, 20));

        m_nameLabel = new QLabel(name, this);
        QFont nameFont = m_nameLabel->font();
        nameFont.setPointSize(theme::kFontSizeBody);
        m_nameLabel->setFont(nameFont);
        // 文字色与选中加粗由全局 QSS 按 objectName 与 selected 属性控制（3.2 状态色）
        m_nameLabel->setObjectName(QStringLiteral("itemNameLabel"));

        auto* subLabel = new QLabel(this);
        subLabel->setMinimumWidth(0); // 允许布局压缩，超长路径省略号截断
        QFont subFont = subLabel->font();
        subFont.setPointSize(theme::kFontSizeCaption);
        subLabel->setFont(subFont);
        subLabel->setStyleSheet(QStringLiteral("color: %1;").arg(theme::kTextSub.name()));
        // 目标摘要可能很长，构造时按右栏可用宽度做省略号；窗口缩放由列表滚动兜底
        const QFontMetrics subFm(subFont);
        subLabel->setText(subFm.elidedText(sub, Qt::ElideMiddle, 560));

        auto* texts = new QVBoxLayout;
        texts->setContentsMargins(0, 0, 0, 0);
        texts->setSpacing(1);
        texts->addWidget(m_nameLabel);
        texts->addWidget(subLabel);

        auto* row = new QHBoxLayout(this);
        row->setContentsMargins(8, 5, 8, 5);
        row->setSpacing(9);
        row->addWidget(iconLabel, 0, Qt::AlignVCenter);
        row->addLayout(texts, 1);
    }

    // 选中态切换：名称变主色加粗（与左栏分组行选中态一致，3.2 状态色）
    void setNameSelected(bool selected)
    {
        // 动态属性变更后需重新抛光，全局 QSS 的 [selected="true"] 规则才会重新命中
        m_nameLabel->setProperty("selected", selected);
        m_nameLabel->style()->unpolish(m_nameLabel);
        m_nameLabel->style()->polish(m_nameLabel);
    }

private:
    QLabel* m_nameLabel = nullptr;
};

// 支持组内拖拽排序的列表：内部移动结束后通知主窗口提交顺序
class ReorderList : public QListWidget {
    Q_OBJECT
public:
    using QListWidget::QListWidget;

signals:
    void orderDropped();

protected:
    void dropEvent(QDropEvent* event) override
    {
        QListWidget::dropEvent(event);
        emit orderDropped();
    }
};

// 编辑快捷项对话框：名称 + 目标（可浏览）+ 类型提示 + 分组归属
class EditItemDialog : public QDialog {
public:
    EditItemDialog(QWidget* parent, const QuickItem& initial, const QList<QuickGroup>& groups)
        : QDialog(parent)
    {
        setWindowTitle(QStringLiteral("浮启 · 编辑快捷项"));
        resize(480, 168);

        m_nameEdit = new QLineEdit(initial.name, this);
        m_targetEdit = new QLineEdit(initial.target, this);
        m_typeLabel = new QLabel(this);
        m_browseButton = new QPushButton(QStringLiteral("浏览..."), this);
        m_groupCombo = new QComboBox(this);
        for (const QuickGroup& g : groups) {
            m_groupCombo->addItem(g.name, g.id);
        }
        const int groupIndex = m_groupCombo->findData(initial.groupId);
        m_groupCombo->setCurrentIndex(groupIndex >= 0 ? groupIndex : 0);

        connect(m_browseButton, &QPushButton::clicked, this, [this]() {
            const QString path = QFileDialog::getOpenFileName(
                this, QStringLiteral("选择目标"), QString(), whitelistedFileFilter());
            if (!path.isEmpty()) {
                m_targetEdit->setText(path);
            }
        });

        auto updateTypeLabel = [this](const QString& text) {
            const std::optional<TargetType> type = detectTargetType(text);
            m_typeLabel->setText(type.has_value()
                ? QStringLiteral("类型：%1").arg(targetTypeName(*type))
                : QStringLiteral("类型：不支持的类型"));
        };
        connect(m_targetEdit, &QLineEdit::textChanged, this, updateTypeLabel);
        updateTypeLabel(initial.target);

        auto* form = new QFormLayout;
        form->addRow(QStringLiteral("名称："), m_nameEdit);
        auto* targetRow = new QHBoxLayout;
        targetRow->addWidget(m_targetEdit, 1);
        targetRow->addWidget(m_browseButton);
        form->addRow(QStringLiteral("目标："), targetRow);
        form->addRow(QStringLiteral("分组："), m_groupCombo);
        form->addRow(QString(), m_typeLabel);

        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        buttons->button(QDialogButtonBox::Ok)->setProperty("variant", "primary");
        buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
        buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        auto* layout = new QVBoxLayout(this);
        layout->addLayout(form);
        layout->addWidget(buttons);
    }

    QString name() const { return m_nameEdit->text(); }
    QString target() const { return m_targetEdit->text(); }
    QString groupId() const { return m_groupCombo->currentData().toString(); }

private:
    QLineEdit* m_nameEdit;
    QLineEdit* m_targetEdit;
    QLabel* m_typeLabel;
    QPushButton* m_browseButton;
    QComboBox* m_groupCombo;
};

} // namespace floaty_ui

ManagerDialog::ManagerDialog(ConfigStore& store, QWidget* parent)
    : QDialog(parent)
    , m_store(store)
{
    setWindowTitle(QStringLiteral("浮启 · 管理快捷项"));
    setMinimumSize(760, 520);
    resize(760, 520);

    // ---- 左栏：分组 ----
    m_groupList = new QListWidget(this);
    m_groupList->setFrameShape(QFrame::NoFrame);
    connect(m_groupList, &QListWidget::currentItemChanged, this,
            [this]() { reloadItems(); updateActions(); });

    m_renameGroupButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/edit.svg")), QStringLiteral("重命名"), this);
    m_deleteGroupButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/delete.svg")), QStringLiteral("删除分组"), this);
    auto* groupOpsRow = new QHBoxLayout;
    groupOpsRow->setSpacing(8);
    groupOpsRow->addWidget(m_renameGroupButton);
    groupOpsRow->addWidget(m_deleteGroupButton);
    connect(m_renameGroupButton, &QPushButton::clicked, this,
            &ManagerDialog::renameCurrentGroup);
    connect(m_deleteGroupButton, &QPushButton::clicked, this,
            &ManagerDialog::deleteCurrentGroup);

    auto* newGroupButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/new_group.svg")), QStringLiteral("新建分组"), this);
    newGroupButton->setProperty("variant", "primary");
    connect(newGroupButton, &QPushButton::clicked, this, &ManagerDialog::createGroup);

    auto* leftPane = new QVBoxLayout;
    leftPane->setSpacing(8);
    leftPane->addWidget(new QLabel(QStringLiteral("分组"), this));
    leftPane->addWidget(m_groupList, 1);
    leftPane->addLayout(groupOpsRow);
    leftPane->addWidget(newGroupButton);

    // ---- 右栏：快捷项 ----
    m_itemList = new floaty_ui::ReorderList(this);
    m_itemList->setViewMode(QListView::ListMode);
    m_itemList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_itemList->setDragDropMode(QAbstractItemView::NoDragDrop);
    m_itemList->setDefaultDropAction(Qt::MoveAction);
    m_itemList->setUniformItemSizes(false);
    connect(m_itemList, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem* current, QListWidgetItem* previous) {
                // 右栏卡片名称跟随选中态变主色加粗（3.2 列表行状态色）
                auto applyCard = [this](QListWidgetItem* it, bool selected) {
                    if (!it) {
                        return;
                    }
                    if (auto* card =
                            dynamic_cast<floaty_ui::ItemCard*>(m_itemList->itemWidget(it))) {
                        card->setNameSelected(selected);
                    }
                };
                applyCard(previous, false);
                applyCard(current, true);
                updateActions();
            });
    connect(m_itemList, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem*) { editCurrent(); });
    connect(qobject_cast<floaty_ui::ReorderList*>(m_itemList),
            &floaty_ui::ReorderList::orderDropped, this,
            &ManagerDialog::commitGroupReorder);

    auto* addFileButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/add_file.svg")), QStringLiteral("添加文件"), this);
    addFileButton->setProperty("variant", "primary");
    auto* addFolderButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/add_folder.svg")), QStringLiteral("添加文件夹"), this);
    auto* addUrlButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/add_url.svg")), QStringLiteral("添加网址"), this);
    m_editButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/edit.svg")), QStringLiteral("编辑"), this);
    m_removeButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/delete.svg")), QStringLiteral("删除"), this);
    m_removeButton->setProperty("variant", "danger");
    m_upButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/move_up.svg")), QStringLiteral("上移"), this);
    m_downButton = new QPushButton(
        QIcon(QStringLiteral(":/icons/move_down.svg")), QStringLiteral("下移"), this);
    auto* closeButton = new QPushButton(QStringLiteral("关闭"), this);
    for (auto* b : {addFileButton, addFolderButton, addUrlButton, m_editButton, m_removeButton,
                    m_upButton, m_downButton}) {
        b->setIconSize(QSize(16, 16));
    }

    connect(addFileButton, &QPushButton::clicked, this, &ManagerDialog::addFile);
    connect(addFolderButton, &QPushButton::clicked, this, &ManagerDialog::addFolder);
    connect(addUrlButton, &QPushButton::clicked, this, &ManagerDialog::addUrl);
    connect(m_editButton, &QPushButton::clicked, this, &ManagerDialog::editCurrent);
    connect(m_removeButton, &QPushButton::clicked, this, &ManagerDialog::removeCurrent);
    connect(m_upButton, &QPushButton::clicked, this, [this]() { moveCurrent(-1); });
    connect(m_downButton, &QPushButton::clicked, this, [this]() { moveCurrent(1); });
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    // 按钮分两行，避免单行最小宽度把窗口撑出小屏幕；第一行以添加与排序为主
    auto* addBar = new QHBoxLayout;
    addBar->setSpacing(8);
    addBar->addWidget(addFileButton);
    addBar->addWidget(addFolderButton);
    addBar->addWidget(addUrlButton);
    addBar->addStretch(1);
    addBar->addWidget(m_upButton);
    addBar->addWidget(m_downButton);

    auto* editBar = new QHBoxLayout;
    editBar->setSpacing(8);
    editBar->addWidget(m_editButton);
    editBar->addWidget(m_removeButton);
    editBar->addStretch(1);
    editBar->addWidget(closeButton);

    auto* rightPane = new QVBoxLayout;
    rightPane->setSpacing(10);
    rightPane->addWidget(m_itemList, 1);
    rightPane->addLayout(addBar);
    rightPane->addLayout(editBar);

    auto* columns = new QHBoxLayout;
    columns->setContentsMargins(16, 12, 16, 12);
    columns->setSpacing(14);
    columns->addLayout(leftPane, 0);
    columns->addLayout(rightPane, 1);
    m_groupList->setFixedWidth(168);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addLayout(columns, 1);

    reloadGroups();
}

void ManagerDialog::showEvent(QShowEvent* /*event*/)
{
    reloadGroups(currentGroupId());
}

QuickGroup ManagerDialog::groupById(const QString& id) const
{
    for (const QuickGroup& g : m_store.groups()) {
        if (g.id == id) {
            return g;
        }
    }
    return QuickGroup{};
}

QString ManagerDialog::currentGroupId() const
{
    QListWidgetItem* current = m_groupList->currentItem();
    return current ? current->data(Qt::UserRole).toString() : QString();
}

QString ManagerDialog::currentItemId() const
{
    QListWidgetItem* current = m_itemList->currentItem();
    return current ? current->data(Qt::UserRole).toString() : QString();
}

void ManagerDialog::reloadGroups(const QString& selectGroupId)
{
    {
        QSignalBlocker blocker(m_groupList);
        m_groupList->clear();

        auto* allItem = new QListWidgetItem(QStringLiteral("全部快捷项"), m_groupList);
        allItem->setData(Qt::UserRole, QString());

        for (const QuickGroup& g : m_store.groups()) {
            auto* item = new QListWidgetItem(g.name, m_groupList);
            item->setData(Qt::UserRole, g.id);
        }

        const QString wanted = selectGroupId;
        for (int i = 0; i < m_groupList->count(); ++i) {
            if (m_groupList->item(i)->data(Qt::UserRole).toString() == wanted) {
                m_groupList->setCurrentRow(i);
                break;
            }
        }
        if (!m_groupList->currentItem()) {
            m_groupList->setCurrentRow(0); // 默认停在"全部快捷项"
        }
    }
    reloadItems();
    updateActions();
}

void ManagerDialog::reloadItems(const QString& selectItemId)
{
    m_itemList->clear();

    // 汇总视图按分组顺序展示全部；分组视图仅展示该组
    struct Row {
        QuickItem item;
        QString sub;
    };
    QList<Row> rows;
    const QString groupId = currentGroupId();
    if (groupId.isEmpty()) {
        for (const QuickGroup& g : m_store.groups()) {
            for (const QuickItem& item : m_store.itemsInGroup(g.id)) {
                rows.append({item, QStringLiteral("%1 · %2").arg(g.name, item.target)});
            }
        }
    } else {
        for (const QuickItem& item : m_store.itemsInGroup(groupId)) {
            rows.append({item, item.target});
        }
    }

    if (rows.isEmpty()) {
        auto* placeholder = new QListWidgetItem(m_itemList);
        auto* card = new floaty_ui::ItemCard(
            QIcon(QStringLiteral(":/icons/empty_state.svg")),
            groupId.isEmpty() ? QStringLiteral("暂无快捷项")
                              : QStringLiteral("当前分组暂无快捷项"),
            QStringLiteral("可通过下方按钮或直接拖入悬浮球添加"));
        placeholder->setFlags(Qt::NoItemFlags);
        placeholder->setSizeHint(card->sizeHint());
        m_itemList->setItemWidget(placeholder, card);
        m_itemList->setDragDropMode(QAbstractItemView::NoDragDrop);
        return;
    }

    int selectRow = -1;
    for (int i = 0; i < rows.size(); ++i) {
        const Row& r = rows.at(i);
        auto* item = new QListWidgetItem(m_itemList);
        auto* card = new floaty_ui::ItemCard(iconForTarget(r.item), r.item.name, r.sub);
        item->setData(Qt::UserRole, r.item.id);
        item->setSizeHint(QSize(0, 44));
        m_itemList->setItemWidget(item, card);
        if (r.item.id == selectItemId) {
            selectRow = i;
        }
    }
    if (selectRow >= 0) {
        m_itemList->setCurrentRow(selectRow);
    }

    // F11：仅具体分组视图允许组内拖拽；汇总视图禁用排序
    m_itemList->setDragDropMode(groupId.isEmpty() ? QAbstractItemView::NoDragDrop
                                                  : QAbstractItemView::InternalMove);
    m_itemList->setDragEnabled(!groupId.isEmpty());
}

void ManagerDialog::updateActions()
{
    const QString groupId = currentGroupId();
    const bool hasItem = !currentItemId().isEmpty();
    const bool groupView = !groupId.isEmpty();
    const bool customGroup = groupView && groupId != QLatin1String(kDefaultGroupId);

    m_editButton->setEnabled(hasItem);
    m_removeButton->setEnabled(hasItem);
    m_upButton->setEnabled(hasItem && groupView);
    m_downButton->setEnabled(hasItem && groupView);
    m_renameGroupButton->setEnabled(customGroup);
    m_deleteGroupButton->setEnabled(customGroup);
}

void ManagerDialog::createGroup()
{
    bool ok = false;
    const QString text = QInputDialog::getText(
        this, QStringLiteral("浮启 · 新建分组"), QStringLiteral("分组名称："),
        QLineEdit::Normal, QString(), &ok);
    if (!ok) {
        return;
    }
    QString newId;
    const GroupError err = m_store.createGroup(text, &newId);
    if (err != GroupError::Ok) {
        const QString msg = err == GroupError::EmptyName
            ? QStringLiteral("分组名称不能为空")
            : QStringLiteral("已存在同名分组");
        QMessageBox::warning(this, QStringLiteral("浮启"), msg);
        return;
    }
    m_store.save();
    reloadGroups(newId);
    emit itemsChanged();
}

void ManagerDialog::renameCurrentGroup()
{
    const QString id = currentGroupId();
    if (id.isEmpty() || id == QLatin1String(kDefaultGroupId)) {
        return;
    }
    const QuickGroup g = groupById(id);
    bool ok = false;
    const QString text = QInputDialog::getText(
        this, QStringLiteral("浮启 · 重命名分组"), QStringLiteral("分组名称："),
        QLineEdit::Normal, g.name, &ok);
    if (!ok) {
        return;
    }
    const GroupError err = m_store.renameGroup(id, text);
    if (err != GroupError::Ok) {
        const QString msg = err == GroupError::EmptyName
            ? QStringLiteral("分组名称不能为空")
            : QStringLiteral("已存在同名分组");
        QMessageBox::warning(this, QStringLiteral("浮启"), msg);
        return;
    }
    m_store.save();
    reloadGroups(id);
    emit itemsChanged();
}

void ManagerDialog::deleteCurrentGroup()
{
    const QString id = currentGroupId();
    if (id.isEmpty() || id == QLatin1String(kDefaultGroupId)) {
        return;
    }
    const QuickGroup g = groupById(id);
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, QStringLiteral("浮启"),
        QStringLiteral("删除分组“%1”后，组内快捷项将移入默认分组。确定删除吗？").arg(g.name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }
    m_store.deleteGroup(id);
    m_store.save();
    reloadGroups(QString());
    emit itemsChanged();
}

void ManagerDialog::addFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("浮启 · 添加程序/脚本/文档"), QString(), whitelistedFileFilter());
    if (!path.isEmpty()) {
        commitTarget(path, currentGroupId());
    }
}

void ManagerDialog::addFolder()
{
    const QString path = QFileDialog::getExistingDirectory(
        this, QStringLiteral("浮启 · 添加文件夹"));
    if (!path.isEmpty()) {
        commitTarget(path, currentGroupId());
    }
}

void ManagerDialog::addUrl()
{
    bool ok = false;
    const QString text = QInputDialog::getText(
        this, QStringLiteral("浮启 · 添加网址"),
        QStringLiteral("网址（以 http:// 或 https:// 开头）："),
        QLineEdit::Normal, QString(), &ok);
    if (!ok) {
        return;
    }
    const QString url = text.trimmed();
    if (!url.startsWith(QLatin1String("http://"), Qt::CaseInsensitive)
        && !url.startsWith(QLatin1String("https://"), Qt::CaseInsensitive)) {
        QMessageBox::warning(this, QStringLiteral("浮启"),
                             QStringLiteral("网址必须以 http:// 或 https:// 开头"));
        return;
    }
    commitTarget(url, currentGroupId());
}

void ManagerDialog::editCurrent()
{
    const QString id = currentItemId();
    if (id.isEmpty()) {
        return;
    }
    QuickItem original;
    bool found = false;
    for (const QuickItem& item : m_store.items()) {
        if (item.id == id) {
            original = item;
            found = true;
            break;
        }
    }
    if (!found) {
        return;
    }

    floaty_ui::EditItemDialog dlg(this, original, m_store.groups());
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }
    const QString newTarget = dlg.target().trimmed();
    const std::optional<TargetType> type = detectTargetType(newTarget);
    if (!type.has_value()) {
        QMessageBox::warning(this, QStringLiteral("浮启"),
                             QStringLiteral("不支持的文件类型：\n%1").arg(newTarget));
        return;
    }
    QString newName = dlg.name().trimmed();
    if (newName.isEmpty()) {
        newName = floaty_ui::defaultNameForTarget(newTarget, *type);
    }
    m_store.updateItem(id, newName, newTarget, *type, dlg.groupId());
    m_store.save();
    reloadGroups(dlg.groupId());
    reloadItems(id);
    emit itemsChanged();
}

void ManagerDialog::removeCurrent()
{
    const QString id = currentItemId();
    if (id.isEmpty()) {
        return;
    }
    QuickItem target;
    for (const QuickItem& item : m_store.items()) {
        if (item.id == id) {
            target = item;
            break;
        }
    }
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, QStringLiteral("浮启"),
        QStringLiteral("确定删除快捷项“%1”吗？").arg(target.name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }
    m_store.removeItem(id);
    m_store.save();
    reloadItems();
    updateActions();
    emit itemsChanged();
}

void ManagerDialog::moveCurrent(int delta)
{
    const QString id = currentItemId();
    if (id.isEmpty() || currentGroupId().isEmpty()) {
        return; // 汇总视图禁用排序
    }
    if (!m_store.moveItem(id, delta)) {
        return; // 首行上移、末行下移不回绕
    }
    m_store.save();
    reloadItems(id);
    emit itemsChanged();
}

void ManagerDialog::commitGroupReorder()
{
    const QString groupId = currentGroupId();
    if (groupId.isEmpty()) {
        return;
    }
    QStringList ids;
    for (int i = 0; i < m_itemList->count(); ++i) {
        const QString id = m_itemList->item(i)->data(Qt::UserRole).toString();
        if (!id.isEmpty()) {
            ids.append(id);
        }
    }
    if (m_store.reorderGroup(groupId, ids) != GroupError::Ok) {
        reloadItems();
        return;
    }
    m_store.save();
    emit itemsChanged();
}

void ManagerDialog::commitTarget(const QString& target, const QString& groupId)
{
    const QString trimmed = target.trimmed();
    const std::optional<TargetType> type = detectTargetType(trimmed);
    if (!type.has_value()) {
        // 白名单外类型拒绝并明确提示（F4）
        QMessageBox::warning(this, QStringLiteral("浮启"),
                             QStringLiteral("不支持的文件类型：\n%1").arg(trimmed));
        return;
    }
    const std::optional<QString> name =
        askName(floaty_ui::defaultNameForTarget(trimmed, *type));
    if (!name.has_value()) {
        return;
    }
    // 汇总视图下新增一律进入默认分组
    const QString realGroup =
        groupId.isEmpty() ? QString::fromLatin1(kDefaultGroupId) : groupId;
    m_store.addItem(*name, trimmed, *type, realGroup);
    m_store.save();
    reloadGroups(realGroup);
    emit itemsChanged();
}

std::optional<QString> ManagerDialog::askName(const QString& defaultName)
{
    bool ok = false;
    const QString text = QInputDialog::getText(
        this, QStringLiteral("浮启 · 添加快捷项"), QStringLiteral("快捷项名称："),
        QLineEdit::Normal, defaultName, &ok);
    if (!ok) {
        return std::nullopt;
    }
    QString name = text.trimmed();
    if (name.isEmpty()) {
        name = defaultName; // 名称不允许为空
    }
    return name;
}

#include "manager_dialog.moc"
