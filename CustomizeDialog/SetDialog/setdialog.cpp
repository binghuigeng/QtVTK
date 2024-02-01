#include "setdialog.h"
#include "ui_setdialog.h"

SetDialog::SetDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SetDialog)
{
    ui->setupUi(this);

    radioButtonExclusive(); // 单选框互斥
    initial(); // 初始化
    initSignalAndSlot(); // 初始化信号与槽连接
    setChecked(); // 设置选中状态
}

SetDialog::~SetDialog()
{
    delete ui;
}

void SetDialog::setWidgetContent(bool windowInquiry)
{
    ui->chbInquiry->setChecked(windowInquiry);
}

void SetDialog::setDefaultButton()
{
//    ui->btnClose->setFocus();  // 设置焦点
//    ui->btnClose->setDefault(true); // 设置默认按钮，设置了这个属性，当用户按下回车的时候，就会按下该按钮
}

void SetDialog::setControlShow(bool windowTop, bool windowInquiry, int index)
{
    ui->chbTop->setChecked(windowTop);
    ui->chbInquiry->setChecked(windowInquiry);
    switch (index) {
    case SysConfig::Default:
        ui->rdoDefault->setChecked(true);
        break;
    case SysConfig::Gray:
        ui->rdoGray->setChecked(true);
        break;
    case SysConfig::NavyBlue:
        ui->rdoNavyBlue->setChecked(true);
        break;
    case SysConfig::DarkBlue:
        ui->rdoDarkBlue->setChecked(true);
        break;
    default:
        ui->rdoDefault->setChecked(true);
        break;
    }
}

void SetDialog::slt_chbTop_stateChanged(int state)
{
    if (state == Qt::Checked) {
        SysConfig::setWindowTop(true);
    } else if (state == Qt::Unchecked) {
        SysConfig::setWindowTop(false);
    }

    emit sigWindowOnTop();
}

void SetDialog::slt_chbInquiry_stateChanged(int state)
{
    if (state == Qt::Checked) {
        SysConfig::setWindowClose(true);
    } else if (state == Qt::Unchecked) {
        SysConfig::setWindowClose(false);
    }
}

void SetDialog::slt_chbColorBasis_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) {
        SysConfig::setPointCloudColorBasis(true);
        ui->chbColorBasis->setText("坐标值");
    } else if (arg1 == Qt::Unchecked) {
        SysConfig::setPointCloudColorBasis(false);
        ui->chbColorBasis->setText("点云轮廓");
    }

    // 重启应用程序
    emit sigRestart();
}

void SetDialog::slt_rdoDefault_clicked()
{
    SysConfig::setRendererBackground(SysConfig::Default);
    emit sigRendererBackground(SysConfig::Default);
}

void SetDialog::slt_rdoGray_clicked()
{
    SysConfig::setRendererBackground(SysConfig::Gray);
    emit sigRendererBackground(SysConfig::Gray);
}

void SetDialog::slt_rdoNavyBlue_clicked()
{
    SysConfig::setRendererBackground(SysConfig::NavyBlue);
    emit sigRendererBackground(SysConfig::NavyBlue);
}

void SetDialog::slt_rdoDarkBlue_clicked()
{
    SysConfig::setRendererBackground(SysConfig::DarkBlue);
    emit sigRendererBackground(SysConfig::DarkBlue);
}

void SetDialog::slt_spbUdpPort_valueChanged(int arg1)
{
    SysConfig::setUdpPort(arg1);
}

void SetDialog::slt_btnUdpBase_toggled(bool checked)
{
    if (checked) {
        ui->spbUdpPort->setPrefix("0x");
        ui->spbUdpPort->setDisplayIntegerBase(16); // 设置显示为 16 进制
//        ui->spbUdpPort->setSpecialValueText("0x0");
        ui->btnUdpBase->setText("十六进制");
        SysConfig::setUdpBase(16);
    } else {
        ui->spbUdpPort->setPrefix("");
        ui->spbUdpPort->setDisplayIntegerBase(10); // 设置显示为 10 进制
        ui->btnUdpBase->setText("十进制");
        SysConfig::setUdpBase(10);
    }
}

void SetDialog::radioButtonExclusive()
{
    // 渲染背景单选框互斥容器
    btnGroupRendererBackground.addButton(ui->rdoDefault); // 默认
    btnGroupRendererBackground.addButton(ui->rdoGray); // 灰色
    btnGroupRendererBackground.addButton(ui->rdoNavyBlue); // 藏青色
    btnGroupRendererBackground.addButton(ui->rdoDarkBlue); // 深蓝色
}

void SetDialog::initial()
{
    // 设置窗口标志位 隐藏窗口的最小化和最大化按钮，以及窗口置顶
    Qt::WindowFlags flags = this->windowFlags(); // 获取窗口标志位
    flags &= ~Qt::WindowMinMaxButtonsHint; // 隐藏窗口的最小化和最大化按钮
    this->setWindowFlags(flags | Qt::WindowStaysOnTopHint); // 设置窗口标志位 窗口置顶

    // 设置窗口标题
    this->setWindowTitle("设置"); // 设置窗口标志位 窗口置顶

    // UDP
    ui->spbUdpPort->setRange(0, 65535); // 设置范围为0到255
    ui->spbUdpPort->setValue(SysConfig::getUdpPort());
    ui->btnUdpBase->setCheckable(true); // 设置进制转换按钮为可选中状态，这意味着用户可以单击按钮来切换其选中状态

    // 关闭按钮
    //    ui->btnClose->setText("关闭");
}

void SetDialog::initSignalAndSlot()
{
    // 窗口置顶
    connect(ui->chbTop, &QCheckBox::stateChanged, this, &SetDialog::slt_chbTop_stateChanged);
    // 窗口询问
    connect(ui->chbInquiry, &QCheckBox::stateChanged, this, &SetDialog::slt_chbInquiry_stateChanged);
    // 点云颜色
    connect(ui->chbColorBasis, &QCheckBox::stateChanged, this, &SetDialog::slt_chbColorBasis_stateChanged);
    // 渲染背景
    connect(ui->rdoDefault, &QRadioButton::clicked, this, &SetDialog::slt_rdoDefault_clicked);
    connect(ui->rdoGray, &QRadioButton::clicked, this, &SetDialog::slt_rdoGray_clicked);
    connect(ui->rdoNavyBlue, &QRadioButton::clicked, this, &SetDialog::slt_rdoNavyBlue_clicked);
    connect(ui->rdoDarkBlue, &QRadioButton::clicked, this, &SetDialog::slt_rdoDarkBlue_clicked);
    // UDP
    connect(ui->spbUdpPort, QOverload<int>::of(&QSpinBox::valueChanged), this, &SetDialog::slt_spbUdpPort_valueChanged);
    connect(ui->btnUdpBase, &QPushButton::toggled, this, &SetDialog::slt_btnUdpBase_toggled);
}

void SetDialog::setChecked()
{
    ui->chbColorBasis->setChecked(SysConfig::getPointCloudColorBasis());
    ui->btnUdpBase->setChecked(SysConfig::getUdpBase() == 16 ? true : false);
}
