#ifndef SETDIALOG_H
#define SETDIALOG_H

#include <QWidget>
#include <QButtonGroup>
#include "SysConfig.h"

namespace Ui {
class SetDialog;
}

class SetDialog : public QWidget
{
    Q_OBJECT

public:
    explicit SetDialog(QWidget *parent = nullptr);
    ~SetDialog();

    /// @brief 设置内容
    /// @param windowInquiry 窗口询问状态
    void setWidgetContent(bool windowInquiry);

    /// @brief 设置默认按钮
    void setDefaultButton();

    /// @brief 设置控件显示
    /// @param windowTop 窗口置顶状态
    /// @param windowInquiry 窗口询问状态
    /// @param index 渲染背景选择
    void setControlShow(bool windowTop, bool windowInquiry, int index);

signals:
    /// @brief 窗口置顶
    void sigWindowOnTop();

    /// @brief 渲染背景
    void sigRendererBackground(SysConfig::RendererBackground index);

    /// @brief 重启
    void sigRestart();

private slots:
    /// @brief 窗口置顶改变
    /// @param state 窗口置顶状态
    void slt_chbTop_stateChanged(int state);

    /// @brief 窗口询问改变
    /// @param state 窗口询问状态
    void slt_chbInquiry_stateChanged(int state);

    /// @brief 点云颜色依据
    /// @param arg1 坐标值或点云轮廓
    void slt_chbColorBasis_stateChanged(int arg1);

    /// @brief 默认渲染背景
    void slt_rdoDefault_clicked();

    /// @brief 灰色渲染背景
    void slt_rdoGray_clicked();

    /// @brief 藏青色渲染背景
    void slt_rdoNavyBlue_clicked();

    /// @brief 深蓝色渲染背景
    void slt_rdoDarkBlue_clicked();

    /// @brief UDP 端口发生改变
    /// @param arg1 改变后的值
    void slt_spbUdpPort_valueChanged(int arg1);

    /// @brief UDP 端口进制转换
    /// @param checked true:选中 false:未选中
    void slt_btnUdpBase_toggled(bool checked);

private:
    /// @brief 单选框互斥
    void radioButtonExclusive();

    /// @brief 初始化
    void initial();

    /// @brief 初始化信号与槽连接
    void initSignalAndSlot();

    /// @brief 设置选中状态
    void setChecked();

private:
    Ui::SetDialog *ui;

    /// @brief 渲染背景单选框互斥容器
    QButtonGroup btnGroupRendererBackground;
};

#endif // SETDIALOG_H
