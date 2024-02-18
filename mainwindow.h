#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QMessageBox>
#include <QAbstractButton>
#include <QPushButton>
#include <QCheckBox>

#include "aboutdialog.h"
#include "setdialog.h"
#include "commondialog.h"

#include "udprecvthread.h"
#include "outlineimagethread.h"

/// @brief VTK headers
#include <vtkPLYReader.h>
#include <vtkPLYWriter.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPolyData.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkScalarBarActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkCallbackCommand.h>
#include <vtkAreaPicker.h>
#include <vtkRenderWindowInteractor.h>
#include <QVTKOpenGLWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkInteractorStyleRubberBandPick.h>

namespace {
/// @brief PickCallbackFunction VTK 事件回调函数
/// @param caller 指向触发事件的 VTK 对象的指针
/// @param eventId 表示触发的事件类型的整数值。例如鼠标点击、键盘按下等
/// @param clientData 指向用户自定义数据的指针
/// @param callData 指向与事件相关的数据的指针。例如鼠标位置、键盘按键信息等
void PickCallbackFunction(vtkObject* caller, long unsigned int eventId,
                          void* clientData, void* callData);
}

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    /// @brief 重写窗口的关闭事件处理
    /// @param event 窗口关闭事件的信息
    void closeEvent(QCloseEvent *event) override;

    /// @brief 准备退出
    void readyQuit();

public slots:
    /// @brief 窗口置顶
    void sltWindowOnTop();

    /// @brief 渲染背景
    /// @param index Default:默认 Gray:灰色 NavyBlue:藏青色 DarkBlue:深蓝色
    void sltRendererBackground(SysConfig::RendererBackground index);

    /// @brief 帧开始
    void sltFrameStart();

    /// @brief 帧数据
    /// @param x 坐标 X
    /// @param y 坐标 Y
    /// @param z 坐标 Z
    void sltFrameData(const double &x, const double &y, const double &z);

    /// @brief 帧结束
    void sltFrameEnd();

    /// @brief 程序退出
    void sltFrameQuit();

    /// @brief 控制轮廓图像线程
    /// @param enable 线程使能 true:启动轮廓图像线程 false:结束轮廓图像线程
    void sltCtrlOutlineThread(const bool &enable);

private slots:
    /// @brief 打开文件
    void slt_actOpen_triggered();

    /// @brief 自动接收
    /// @param arg1 true:自动接收 false:取消自动接收
    void slt_actAutoRecv_toggled(bool arg1);

    /// @brief 退出
    void slt_actQuit_triggered();

    /// @brief 重置
    void slt_actReset_triggered();

    /// @brief 添加
    void slt_actAdd_triggered();

    /// @brief 重启
    void slt_actRestart_triggered();

    /// @brief 系统关机
    void slt_actSysShutdown_triggered();

    /// @brief 系统重启
    void slt_actSysRestart_triggered();

    /// @brief 通用
    void slt_actUniversal_triggered();

    /// @brief 点云颜色
    /// @param arg1 true:显示点云颜色 false:不显示点云颜色
    void slt_actPointCloudColor_toggled(bool arg1);

    /// @brief 关于
    void slt_actAbout_triggered();

    /// @brief 关闭窗口点击的按钮
    /// @param butClicked 抽象按钮
    void buttonClicked(QAbstractButton *butClicked);

    /// @brief 窗口询问改变
    /// @param state 窗口询问状态
    void slt_chbInquiry_stateChanged(int state);

private:
    /// @brief 初始化
    void initial();

    /// @brief 初始化配置
    void initSysConfig();

    /// @brief 初始化关闭窗口
    void initCloseWindow();

    /// @brief 初始化状态栏显示消息
    void initStatusbarMessage();

    /// @brief 初始化 VTK
    void initVTK();

    /// @brief 初始化信号与槽函数
    void initSignalsAndSlots();

    /// @brief 渲染背景
    /// @param index Default:默认 Gray:灰色 NavyBlue:藏青色 DarkBlue:深蓝色
    void setRendererBackground(SysConfig::RendererBackground index);

    /// @brief 显示点云
    /// @param fileName 文件路径
    void showPointCloud(QString fileName);

    /// @brief 保存ply格式的点云文件
    void savePointCloudFile();

    /// @brief 显示点云信息
    /// @param fileName 文件路径
    /// @param numberOfPoints 点云个数
    void showPointCloudInfo(QString fileName, vtkIdType numberOfPoints);

    /// @brief 显示点云信息
    /// @param numberOfPoints 点云个数
    void showPointCloudInfo(vtkIdType numberOfPoints);

private:
    Ui::MainWindow *ui;

    /// @brief 状态栏消息标签
    QLabel statusLabel;

    /// @brief 关于对话框
    AboutDialog dlgAbout;

    /// @brief 设置对话框
    SetDialog dlgSet;

    /// @brief 通用对话框
    CommonDialog dlgCommon;

    /// @brief 关闭窗口消息对话框
    QMessageBox msgBox;
    QPushButton *btnAccept;
    QPushButton *btnReject;
    QCheckBox chbInquiry;
    bool bAcceptClose;

    /// @brief UDP 接收线程
    UDPRecvThread udpThread;

    /// @brief 轮廓数据
    OutlineImageThread outlineThread;

    /// @brief 点云颜色显示依据
    bool bColorBasis;

    /// @brief VTK variables
    vtkIdType pid[1]; // 存储一个点的标识符
    double bounds[6]; // 获取数据范围
    vtkSmartPointer<vtkPLYReader> reader; // 读取PLY文件
    vtkSmartPointer<vtkPLYWriter> writer; // 保存PLY文件
    vtkSmartPointer<vtkPoints> points; // 点云数据
    vtkSmartPointer<vtkCellArray> vertices; // 顶点
    vtkSmartPointer<vtkFloatArray> scalars; // 标量数据
    vtkSmartPointer<vtkUnsignedCharArray> colors; // 颜色数组
    vtkSmartPointer<vtkPolyData> polydata; // 多边形数据
    vtkSmartPointer<vtkLookupTable> lut; // 颜色映射
    vtkSmartPointer<vtkPolyDataMapper> mapper; // 点云映射器
    vtkSmartPointer<vtkActor> actor; // 点云演员
    vtkSmartPointer<vtkScalarBarActor> scalarBar; // 标量条
    vtkSmartPointer<vtkRenderer> renderer; // 渲染器
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow; // 渲染窗口
    vtkSmartPointer<vtkCallbackCommand> pickCallback;; // 回调函数
    vtkSmartPointer<vtkAreaPicker> areaPicker; // 选择框
    QVTKOpenGLWidget *renderWindowInteractor; // 交互器
    vtkSmartPointer<vtkAxesActor> axes; // 坐标系
    vtkSmartPointer<vtkOrientationMarkerWidget> marker; // 坐标系标记
    vtkSmartPointer<vtkInteractorStyleRubberBandPick> style;; // 交互器样式
};
#endif // MAINWINDOW_H
