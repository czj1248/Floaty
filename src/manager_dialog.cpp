#include "manager_dialog.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

#include "config_store.h"
#include "icon_provider.h"

namespace {

// 根据目标生成默认名称：文件取主名，文件夹取目录名，网址取主机名
QString defaultNameForTarget(const QString& target, TargetType type)
{
    if (type == TargetType::Url) {
        const QString host = QUrl(target).host();
        return host.isEmpty() ? target : host;
    }
    if (type == TargetType::Folder) {
        return QFileInfo(target).dir().dirName();
    }
    return QFileInfo(target).completeBaseName();
}

// 编辑快捷项的小对话框：名称 + 目标（可浏览）+ 类型实时提示
class EditItemDialog : public QDialog {
public:
    EditItemDialog(QWidget* parent, const QuickItem& initial)
        : QDialog(parent)
    {
        setWindowTitle(QStringLiteral("编辑快捷项"));
        resize(460, 130);

        auto* nameEdit = new QLineEdit(initial.name, this);
        auto* targetEdit = new QLineEdit(initial.target, this);
        auto* typeLabel = new QLabel(this);
        auto* browseButton = new QPushButton(QStringLiteral("浏览..."), this);

        connect(browseButton, &QPushButton::clicked, this, [this, targetEdit]() {
            const QString path = QFileDialog::getOpenFileName(
                this, QStringLiteral("选择目标"), QString(), whitelistedFileFilter());
            if (!path.isEmpty()) {
                targetEdit->setText(path);
            }
        });

        auto updateTypeLabel = [typeLabel](const QString& text) {
            const std::optional<TargetType> type = detectTargetType(text);
            typeLabel->setText(type.has_value()
                ? QStringLiteral("类型：%1").arg(targetTypeName(*type))
                : QStringLiteral("类型：不支持的类型"));
        };
        connect(targetEdit, &QLineEdit::textChanged, this, updateTypeLabel);
        updateTypeLabel(initial.target);

        auto* form = new QFormLayout;
        form->addRow(QStringLiteral("名称："), nameEdit);
        auto* targetRow = new QHBoxLayout;
        targetRow->addWidget(targetEdit, 1);
        targetRow->addWidget(browseButton);
        form->addRow(QStringLiteral("目标："), targetRow);
        form->addRow(QString(), typeLabel);

        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        auto* layout = new QVBoxLayout(this);
        layout->addLayout(form);
        layout->addWidget(buttons);

        m_nameEdit = nameEdit;
        m_targetEdit = targetEdit;
    }

    QString name() const { return m_nameEdit->text(); }
    QString target() const { return m_targetEdit->text(); }

private:
    QLineEdit* m_nameEdit;
    QLineEdit* m_targetEdit;
};

} // namespace

ManagerDialog::ManagerDialog(ConfigStore& store, QWidget* parent)
    : QDialog(parent)
    , m_store(store)
{
    setWindowTitle(QStringLiteral("管理快捷项"));
    resize(560, 420);

    m_list = new QListWidget(this);
    m_list->setAlternatingRowColors(true);
    m_list->setIconSize(QSize(20, 20));

    auto* addButton = new QPushButton(QStringLiteral("添加文件"), this);
    auto* addFolderButton = new QPushButton(QStringLiteral("添加文件夹"), this);
    auto* addUrlButton = new QPushButton(QStringLiteral("添加网址"), this);
    auto* editButton = new QPushButton(QStringLiteral("编辑"), this);
    auto* removeButton = new QPushButton(QStringLiteral("删除"), this);
    auto* closeButton = new QPushButton(QStringLiteral("关闭"), this);

    connect(addButton, &QPushButton::clicked, this, &ManagerDialog::addFile);
    connect(addFolderButton, &QPushButton::clicked, this, &ManagerDialog::addFolder);
    connect(addUrlButton, &QPushButton::clicked, this, &ManagerDialog::addUrl);
    connect(editButton, &QPushButton::clicked, this, &ManagerDialog::editCurrent);
    connect(removeButton, &QPushButton::clicked, this, &ManagerDialog::removeCurrent);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    auto* buttonColumn = new QVBoxLayout;
    buttonColumn->addWidget(addButton);
    buttonColumn->addWidget(addFolderButton);
    buttonColumn->addWidget(addUrlButton);
    buttonColumn->addWidget(editButton);
    buttonColumn->addWidget(removeButton);
    buttonColumn->addStretch(1);
    buttonColumn->addWidget(closeButton);

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_list, 1);
    mainLayout->addLayout(buttonColumn);

    reload();
}

void ManagerDialog::showEvent(QShowEvent* /*event*/)
{
    reload();
}

void ManagerDialog::reload()
{
    m_list->clear();
    const QList<QuickItem> items = m_store.items();
    for (int i = 0; i < items.size(); ++i) {
        const QuickItem& item = items.at(i);
        auto* row = new QListWidgetItem(m_list);
        row->setIcon(iconForTarget(item));
        row->setText(item.name);
        row->setToolTip(QStringLiteral("%1\n%2").arg(targetTypeName(item.type), item.target));
        row->setData(Qt::UserRole, i);
    }
}

void ManagerDialog::addFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("添加程序/脚本/文档"), QString(), whitelistedFileFilter());
    if (!path.isEmpty()) {
        commitTarget(path);
    }
}

void ManagerDialog::addFolder()
{
    const QString path = QFileDialog::getExistingDirectory(
        this, QStringLiteral("添加文件夹"));
    if (!path.isEmpty()) {
        commitTarget(path);
    }
}

void ManagerDialog::addUrl()
{
    bool ok = false;
    const QString text = QInputDialog::getText(
        this, QStringLiteral("添加网址"),
        QStringLiteral("网址（以 http:// 或 https:// 开头）："),
        QLineEdit::Normal, QString(), &ok);
    if (!ok) {
        return;
    }
    const QString url = text.trimmed();
    if (!url.startsWith(QLatin1String("http://"), Qt::CaseInsensitive)
        && !url.startsWith(QLatin1String("https://"), Qt::CaseInsensitive)) {
        QMessageBox::warning(this, QStringLiteral("Floaty"),
                             QStringLiteral("网址必须以 http:// 或 https:// 开头"));
        return;
    }
    commitTarget(url);
}

void ManagerDialog::editCurrent()
{
    QListWidgetItem* current = m_list->currentItem();
    if (!current) {
        return;
    }
    const int row = m_list->row(current);
    const QuickItem item = m_store.items().at(row);

    EditItemDialog dlg(this, item);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }
    const QString newTarget = dlg.target().trimmed();
    const std::optional<TargetType> type = detectTargetType(newTarget);
    if (!type.has_value()) {
        QMessageBox::warning(this, QStringLiteral("Floaty"),
                             QStringLiteral("不支持的文件类型：\n%1").arg(newTarget));
        return;
    }
    QString newName = dlg.name().trimmed();
    if (newName.isEmpty()) {
        newName = defaultNameForTarget(newTarget, *type);
    }
    QuickItem updated;
    updated.name = newName;
    updated.target = newTarget;
    updated.type = *type;
    m_store.updateItem(row, updated);
    m_store.save();
    reload();
    emit itemsChanged();
}

void ManagerDialog::removeCurrent()
{
    QListWidgetItem* current = m_list->currentItem();
    if (!current) {
        return;
    }
    const int row = m_list->row(current);
    const QuickItem item = m_store.items().at(row);
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, QStringLiteral("Floaty"),
        QStringLiteral("确定删除快捷项“%1”吗？").arg(item.name),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }
    m_store.removeItem(row);
    m_store.save();
    reload();
    emit itemsChanged();
}

void ManagerDialog::commitTarget(const QString& target)
{
    const QString trimmed = target.trimmed();
    const std::optional<TargetType> type = detectTargetType(trimmed);
    if (!type.has_value()) {
        // 白名单外类型（如 .reg、.ps1）：拒绝并明确提示（F4 验收标准第 3 条）
        QMessageBox::warning(this, QStringLiteral("Floaty"),
                             QStringLiteral("不支持的文件类型：\n%1").arg(trimmed));
        return;
    }
    const std::optional<QString> name = askName(defaultNameForTarget(trimmed, *type));
    if (!name.has_value()) {
        return;
    }
    QuickItem item;
    item.name = *name;
    item.target = trimmed;
    item.type = *type;
    m_store.addItem(item);
    m_store.save();
    reload();
    emit itemsChanged();
}

std::optional<QString> ManagerDialog::askName(const QString& defaultName)
{
    bool ok = false;
    const QString text = QInputDialog::getText(
        this, QStringLiteral("Floaty"), QStringLiteral("快捷项名称："),
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
