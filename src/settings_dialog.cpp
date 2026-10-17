#include "settings_dialog.h"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>

#include "config_store.h"
#include "theme.h"

SettingsDialog::SettingsDialog(ConfigStore& store, QWidget* parent)
    : QDialog(parent)
    , m_store(store)
{
    setWindowTitle(QStringLiteral("浮启 · 设置"));
    setFixedWidth(380);

    // 球大小三档单选
    m_small = new QRadioButton(QStringLiteral("小（40 像素）"), this);
    m_standard = new QRadioButton(QStringLiteral("标准（48 像素）"), this);
    m_large = new QRadioButton(QStringLiteral("大（56 像素）"), this);
    m_sizeGroup = new QButtonGroup(this);
    m_sizeGroup->addButton(m_small);
    m_sizeGroup->addButton(m_standard);
    m_sizeGroup->addButton(m_large);
    auto* sizeBox = new QVBoxLayout;
    sizeBox->setSpacing(8);
    sizeBox->addWidget(m_small);
    sizeBox->addWidget(m_standard);
    sizeBox->addWidget(m_large);

    QFont titleFont;
    titleFont.setBold(true);
    auto* sizeTitle = new QLabel(QStringLiteral("悬浮球大小"), this);
    sizeTitle->setFont(titleFont);

    // 不透明度滑块：0-100，步进 5
    auto* opacityTitle = new QLabel(QStringLiteral("展开面板不透明度"), this);
    opacityTitle->setFont(titleFont);
    m_opacitySlider = new QSlider(Qt::Horizontal, this);
    m_opacitySlider->setRange(theme::kOpacityMin, theme::kOpacityMax);
    m_opacitySlider->setSingleStep(theme::kOpacityStep);
    m_opacitySlider->setPageStep(theme::kOpacityStep);
    m_opacitySlider->setTickInterval(theme::kOpacityStep);
    m_opacityValue = new QLabel(this);
    m_opacityValue->setFixedWidth(48);
    auto* opacityRow = new QHBoxLayout;
    opacityRow->addWidget(m_opacitySlider, 1);
    opacityRow->addWidget(m_opacityValue);

    // 面板最大宽度滑块：80-480，步进 20
    auto* maxWidthTitle = new QLabel(QStringLiteral("展开面板最大宽度"), this);
    maxWidthTitle->setFont(titleFont);
    m_maxWidthSlider = new QSlider(Qt::Horizontal, this);
    m_maxWidthSlider->setRange(theme::kPanelMaxWidthMin, theme::kPanelMaxWidthMax);
    m_maxWidthSlider->setSingleStep(theme::kPanelMaxWidthStep);
    m_maxWidthSlider->setPageStep(theme::kPanelMaxWidthStep);
    m_maxWidthSlider->setTickInterval(theme::kPanelMaxWidthStep);
    m_maxWidthValue = new QLabel(this);
    m_maxWidthValue->setFixedWidth(64);
    auto* maxWidthRow = new QHBoxLayout;
    maxWidthRow->addWidget(m_maxWidthSlider, 1);
    maxWidthRow->addWidget(m_maxWidthValue);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setProperty("variant", "primary");
    buttons->button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        Appearance next = m_store.appearance();
        next.ballSize = selectedDiameter();
        next.opacity = m_opacitySlider->value();
        next.panelMaxWidth = m_maxWidthSlider->value();
        m_store.setAppearance(next);
        m_store.save();
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // 实时预览：三类控件各自只发对应作用域的信号，互不牵连
    connect(m_sizeGroup, &QButtonGroup::buttonToggled, this, [this](QAbstractButton* b) {
        if (b->isChecked()) {
            emit ballSizePreview(selectedDiameter());
        }
    });
    connect(m_opacitySlider, &QSlider::valueChanged, this, [this](int value) {
        m_opacityValue->setText(QStringLiteral("%1%").arg(value));
        emit opacityPreview(value);
    });
    connect(m_maxWidthSlider, &QSlider::valueChanged, this, [this](int value) {
        m_maxWidthValue->setText(QStringLiteral("%1 像素").arg(value));
        emit panelMaxWidthPreview(value);
    });

    auto* form = new QVBoxLayout;
    form->setContentsMargins(20, 16, 20, 16);
    form->setSpacing(10);
    form->addWidget(sizeTitle);
    form->addLayout(sizeBox);
    form->addSpacing(8);
    form->addWidget(opacityTitle);
    form->addLayout(opacityRow);
    form->addSpacing(8);
    form->addWidget(maxWidthTitle);
    form->addLayout(maxWidthRow);
    form->addStretch(1);

    auto* buttonsRow = new QHBoxLayout;
    buttonsRow->setContentsMargins(20, 0, 20, 16);
    buttonsRow->addWidget(buttons);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addLayout(form, 1);
    root->addLayout(buttonsRow);

    // 首次打开按已保存配置初始化控件
    syncFromConfig();
}

void SettingsDialog::syncFromConfig()
{
    const Appearance current = m_store.appearance();
    // 仅同步显示状态：阻塞信号，避免把还原动作误发为实时预览
    const QSignalBlocker blockSize(m_sizeGroup);
    const QSignalBlocker blockOpacity(m_opacitySlider);
    const QSignalBlocker blockMaxWidth(m_maxWidthSlider);
    if (current.ballSize <= theme::kBallSizeSmall) {
        m_small->setChecked(true);
    } else if (current.ballSize >= theme::kBallSizeLarge) {
        m_large->setChecked(true);
    } else {
        m_standard->setChecked(true);
    }
    m_opacitySlider->setValue(current.opacity);
    m_opacityValue->setText(QStringLiteral("%1%").arg(current.opacity));
    m_maxWidthSlider->setValue(current.panelMaxWidth);
    m_maxWidthValue->setText(QStringLiteral("%1 像素").arg(current.panelMaxWidth));
}

int SettingsDialog::selectedDiameter() const
{
    if (m_small->isChecked()) {
        return theme::kBallSizeSmall;
    }
    if (m_large->isChecked()) {
        return theme::kBallSizeLarge;
    }
    return theme::kBallSizeStandard;
}
