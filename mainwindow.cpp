#include "mainwindow.h"
#include "./ui_mainwindow.h"

/// @brief VTK headers
#include <vtkPointData.h>
#include <vtkDataSet.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkProp3DCollection.h>

#include <QDateTime>
#include <QCloseEvent>
#include <QFileDialog>
#include <QProcess>
#include <QDebug>

namespace {
void PickCallbackFunction(vtkObject* caller,
                          long unsigned int vtkNotUsed(eventId),
                          void* vtkNotUsed(clientData),
                          void* vtkNotUsed(callData))
{
    std::cout << "Pick." << std::endl;
#if 0
    vtkAreaPicker* areaPicker = static_cast<vtkAreaPicker*>(caller);
    vtkProp3DCollection* props = areaPicker->GetProp3Ds();
    props->InitTraversal();

    for (vtkIdType i = 0; i < props->GetNumberOfItems(); i++)
    {
    vtkProp3D* prop = props->GetNextProp3D();
    std::cout << "Picked prop: " << prop << std::endl;
    }
#endif
}
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    udpThread.start(); // 启动 UDP 接收线程
    outlineThread.startMv3dLpSDK(); // 启动 Mv3dLpSDK

    // 设置窗口标志位
    if (SysConfig::getWindowTop()) {
        this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint); // 窗口置顶
    }
    this->setWindowTitle("复检软件"); // 设置窗口标题

    initial(); // 初始化
    initSysConfig(); // 初始化配置
    initCloseWindow(); // 初始化关闭窗口
    initStatusbarMessage(); // 初始化状态栏显示消息
    initSignalsAndSlots(); // 初始化信号与槽函数
    initVTK(); // 初始化VTK
}

MainWindow::~MainWindow()
{
    // 若 UDP 接收线程正在运行，则结束线程
    if (udpThread.isRunning())
    {
        udpThread.stopThread();
    }

    // 若轮廓图像数据线程正在运行，则结束线程
    if (outlineThread.isRunning())
    {
        outlineThread.setThreadFlag(false); // 设置线程运行标志
        outlineThread.wait();
    }
    outlineThread.quitMv3dLpSDK(); // 退出 Mv3dLpSDK

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
#if 0
    // 基于静态函数创建消息对话框
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, this->windowTitle(),
                                                                tr("Are you sure you want to close?\n"),
                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore(); // 忽略关闭事件
    } else {
        event->accept(); // 接受关闭事件
    }
#endif

#if 0
    // 基于属性的 API 创建消息对话框
    QMessageBox msgBox;
    msgBox.setWindowTitle(this->windowTitle());
    msgBox.setText("Are you sure you want to close?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIcon(QMessageBox::Question);

    // 显示对话框，并在用户点击确认按钮时关闭窗口
    if (msgBox.exec() == QMessageBox::Yes) {
        event->accept(); // 接受关闭事件
    } else {
        event->ignore(); // 忽略关闭事件
    }
#endif

    if (SysConfig::getWindowClose()) {
        QApplication::setActiveWindow(&msgBox); // 将指定窗口设置为活动窗口并将焦点设置到该窗口
        if (msgBox.isVisible() && bAcceptClose) {
            if (bAcceptClose) {
                readyQuit(); // 准备退出
                event->accept(); // 接受关闭事件
            }
        } else {
            event->ignore(); // 忽略关闭事件
            msgBox.show();
        }
    } else {
        readyQuit(); // 准备退出
        event->accept(); // 接受关闭事件
    }
}

void MainWindow::readyQuit()
{
    // 关闭关于对话框
    if (dlgAbout.isVisible()) {
        dlgAbout.close();
    }

    // 关闭设置对话框
    if (dlgSet.isVisible()) {
        dlgSet.close();
    }

    // 关闭通用对话框
    if (dlgCommon.isVisible()) {
        dlgCommon.close();
    }
}

void MainWindow::sltWindowOnTop()
{
    // 设置窗口标志位 隐藏窗口的最小化和最大化按钮，以及窗口置顶
    if (this->windowFlags() & Qt::WindowStaysOnTopHint) {
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowStaysOnTopHint); // 取消窗口置顶
    } else {
        this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint); // 窗口置顶
    }
    this->show(); // 重新显示窗口

#if 0
    // 设置窗口标志位 隐藏窗口的最小化和最大化按钮，以及窗口置顶
    Qt::WindowFlags flags = this->windowFlags(); // 获取窗口标志位
    if (enable) {
        this->setWindowFlags(flags | Qt::WindowStaysOnTopHint); // 窗口置顶
    } else {
        this->setWindowFlags(flags & ~Qt::WindowStaysOnTopHint); // 取消窗口置顶
    }
    this->show(); // 重新显示窗口
#endif
}

void MainWindow::sltRendererBackground(SysConfig::RendererBackground index)
{
    setRendererBackground(index); // 设置渲染器背景颜色
    renderWindow->Render(); // 刷新渲染窗口
}

void MainWindow::sltFrameStart()
{
    qDebug("sltFrameStart");
    ui->lbPointNum->clear(); // 清空点云个数内容

    // 清除现有的点云显示
    points->Initialize();
    vertices->Initialize();
}

void MainWindow::sltFrameData(const double &x, const double &y, const double &z)
{
    pid[0] = points->InsertNextPoint(x, y, z);
    vertices->InsertNextCell(1, pid);
}

void MainWindow::sltFrameEnd()
{
    qDebug("sltFrameEnd");
    if (points->GetNumberOfPoints() > 0) {
        if (bColorBasis) {
            if (ui->actPointCloudColor->isChecked()) {
                // 设置一个包含标量数据的数组的大小，使其与点云数据中的点数相匹配
                scalars->SetNumberOfTuples(points->GetNumberOfPoints());
                // 插入顶点 创建一个包含单个顶点的单元，并将点云数据中的每个点与这些单元关联起来，从而定义了点云数据的拓扑结构
                for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
                {
                    scalars->SetValue(i, points->GetPoint(i)[2]); // 将 z 值设置为标量值
                }
            } else {
                scalars->Initialize();
                actor->GetProperty()->SetColor(1.0, 1.0, 1.0); // 取消设置属性，恢复默认颜色
            }
        } else {
            // 设置颜色
            colors->SetNumberOfTuples(points->GetNumberOfPoints());
            for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
            {
                if (0 == i % 3) {
                    colors->SetTuple3(i, 0, 255, 0); // 绿色
                } else if (1 == i % 3) {
                    colors->SetTuple3(i, 255, 0, 0); // 红色
                } else {
                    colors->SetTuple3(i, 255, 255, 0); // 黄色
                }
            }
        }

        polydata->GetBounds(bounds);
        mapper->SetScalarRange(bounds[4], bounds[5]);

        // 根据点云数据自适应地设置相机位置和视角
        renderer->ResetCamera();

        scalarBar->SetVisibility(true); // 显示标量条
        marker->SetEnabled(1); // 启用坐标系标记

        // 刷新渲染窗口以显示新的点云数据
        renderWindow->Render();

        // 保存ply格式的点云文件
        savePointCloudFile();

        // 显示点云信息
        showPointCloudInfo(points->GetNumberOfPoints());
    }
}

void MainWindow::sltFrameQuit()
{
    ui->actReset->trigger();
}

void MainWindow::sltCtrlOutlineThread(const bool &enable)
{
    outlineThread.setThreadFlag(enable); // 设置线程运行标志
    if (enable) {
        outlineThread.start(); // 启动线程
    } else {
        outlineThread.wait(); // 阻止线程执行，直到线程结束
    }
}

void MainWindow::slt_actOpen_triggered()
{
    // 选择 .ply 文件
    QString fileName = QFileDialog::getOpenFileName(
                this, "打开文件", "", "点云文件 (*.ply)");

    if (fileName.isEmpty()) {
#if 0
        // 通用对话框
        dlgCommon.setWidgetTitle("读取 3D 对象模型失败");
        dlgCommon.setWidgetIcon(":/Dialog/qmessagebox-crit.png");
        dlgCommon.setWidgetContent(fileName);
        if (!dlgCommon.isVisible()) {
            dlgCommon.show(); //非模态显示对话框
        }
        dlgCommon.setDefaultButton(); //设置默认按钮
#endif
    } else {
        showPointCloud(fileName); // 显示点云
    }
}

void MainWindow::slt_actAutoRecv_toggled(bool arg1)
{
//    qDebug() << "arg1 " << arg1;
    if (arg1) {
        udpThread.bindPort(); // 绑定端口
        SysConfig::setAutoRecv(true);
    } else {
        udpThread.unbindPort(); // 关闭套接字并释放绑定的端口
        SysConfig::setAutoRecv(false);
        ui->actReset->trigger();
    }
}

void MainWindow::slt_actQuit_triggered()
{
    this->close();
}

void MainWindow::slt_actReset_triggered()
{
    ui->lbPointNum->clear(); // 清空点云个数内容

    // 清除现有的点云显示
    points->Initialize();
    vertices->Initialize();

    scalarBar->SetVisibility(false); // 隐藏标量条
    marker->SetEnabled(0); // 禁用坐标系标记

    // 刷新渲染窗口
    renderWindow->Render();
}

void MainWindow::slt_actAdd_triggered()
{
    ui->lbPointNum->clear(); // 清空点云个数内容

    // 清除现有的点云显示
    points->Initialize();
    vertices->Initialize();

    pid[0] = points->InsertNextPoint(0.0, 0.0, 0.0);
    vertices->InsertNextCell(1, pid);
    pid[0] = points->InsertNextPoint(1.0, 0.0, 0.0);
    vertices->InsertNextCell(1, pid);
    pid[0] = points->InsertNextPoint(0.0, 1.0, 0.0);
    vertices->InsertNextCell(1, pid);
    pid[0] = points->InsertNextPoint(0.0, 0.0, 1.0);
    vertices->InsertNextCell(1, pid);

    if (bColorBasis) {
        if (ui->actPointCloudColor->isChecked()) {
            // 设置一个包含标量数据的数组的大小，使其与点云数据中的点数相匹配
            scalars->SetNumberOfTuples(points->GetNumberOfPoints());
            // 插入顶点 创建一个包含单个顶点的单元，并将点云数据中的每个点与这些单元关联起来，从而定义了点云数据的拓扑结构
            for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
            {
                scalars->SetValue(i, points->GetPoint(i)[2]); // 将 z 值设置为标量值
            }
        } else {
            scalars->Initialize();
            actor->GetProperty()->SetColor(1.0, 1.0, 1.0); // 取消设置属性，恢复默认颜色
        }
    } else {
        // 设置颜色
        colors->SetNumberOfTuples(points->GetNumberOfPoints());
        for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
        {
            if (0 == i % 3) {
                colors->SetTuple3(i, 0, 255, 0); // 绿色
            } else if (1 == i % 3) {
                colors->SetTuple3(i, 255, 0, 0); // 红色
            } else {
                colors->SetTuple3(i, 255, 255, 0); // 黄色
            }
        }
    }

    polydata->GetBounds(bounds);
    mapper->SetScalarRange(bounds[4], bounds[5]);

    // 根据点云数据自适应地设置相机位置和视角
    renderer->ResetCamera();

    scalarBar->SetVisibility(true); // 显示标量条
    marker->SetEnabled(1); // 启用坐标系标记

    // 刷新渲染窗口以显示新的点云数据
    renderWindow->Render();

    // 显示点云信息
    showPointCloudInfo(points->GetNumberOfPoints());
}

void MainWindow::slt_actRestart_triggered()
{
#if 0
    qDebug() << "applicationFilePath " << QCoreApplication::applicationFilePath();
    qDebug() << "arguments " << QCoreApplication::arguments();

    /// 法一
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());

    /// 法二
    QString program = QCoreApplication::applicationFilePath(); // 应用程序的路径
    QStringList arguments = QCoreApplication::arguments(); // 应用程序的参数列表
    QProcess::startDetached(program, arguments);
    qApp->exit();

    /// 法三
    // 退出当前进程
    QCoreApplication::exit(0);
    // 启动新的进程
    QProcess::startDetached(qApp->applicationFilePath(), QStringList());
#endif

    // 退出应用程序的主事件循环，从而关闭应用程序
    QCoreApplication::quit();

    // 启动新的进程来运行应用程序
    QProcess::startDetached(QCoreApplication::applicationFilePath(), QCoreApplication::arguments());
}

void MainWindow::slt_actUniversal_triggered()
{
    // 设置对话框
    if (!dlgSet.isVisible()) {
        dlgSet.show(); //非模态显示对话框
        dlgSet.setDefaultButton(); //设置默认按钮
    }
}

void MainWindow::slt_actPointCloudColor_toggled(bool arg1)
{
    SysConfig::setPointCloudColor(arg1);
}

void MainWindow::slt_actAbout_triggered()
{
    // 关于对话框
    if (!dlgAbout.isVisible()) {
        dlgAbout.show(); //非模态显示对话框
        dlgAbout.setDefaultButton(); //设置默认按钮
    }
}

void MainWindow::buttonClicked(QAbstractButton *butClicked)
{
    if(butClicked == (QAbstractButton*)btnAccept){
        bAcceptClose = true;
        this->close();
    } else if (butClicked == (QAbstractButton*)btnReject) {
//        this->close();
    }
}

void MainWindow::slt_chbInquiry_stateChanged(int state)
{
    if (state == Qt::Checked) {
        SysConfig::setWindowClose(false);
        dlgSet.setWidgetContent(false);
    } else if (state == Qt::Unchecked) {
        SysConfig::setWindowClose(true);
        dlgSet.setWidgetContent(true);
    }
}

void MainWindow::initial()
{
    ui->lbPointNum->clear(); // 清空点云个数内容
    ui->lbPointNum->setAlignment(Qt::AlignCenter); // 居中对齐
    ui->lbPointNum->setFont(QFont("Microsoft YaHei UI", 10, QFont::Normal, false)); // 设置字体、字号、粗体、斜体
}

void MainWindow::initSysConfig()
{
    // 设置界面参数显示
    dlgSet.setControlShow(SysConfig::getWindowTop(), SysConfig::getWindowClose(), SysConfig::getRendererBackground());

    // 点云颜色选中状态
    ui->actPointCloudColor->setChecked(SysConfig::getPointCloudColor());

    // 点云颜色显示
    ui->actPointCloudColor->setVisible(SysConfig::getPointCloudColorBasis());

    // 自动接收显示
    ui->actAutoRecv->setChecked(SysConfig::getAutoRecv());

    // 点云颜色显示依据
    bColorBasis = SysConfig::getPointCloudColorBasis();
}

void MainWindow::initCloseWindow()
{
    bAcceptClose = false;
    msgBox.setWindowTitle(this->windowTitle()); // 设置消息对话框的标题
    msgBox.setText("您确定要关闭吗？"); // 设置消息对话框的具体内容
    msgBox.setIcon(QMessageBox::Question); // 设置消息对话框的图标
    // 自定义两个按钮
    btnAccept = msgBox.addButton("是(Y)", QMessageBox::AcceptRole); // 使用给定文本创建一个按钮，将其添加到指定角色的消息框中
    btnReject = msgBox.addButton("否(N)", QMessageBox::RejectRole); // 使用给定文本创建一个按钮，将其添加到指定角色的消息框中
    msgBox.setDefaultButton(btnAccept); // 设置消息对话框的默认按钮，即按下回车键会触发的按钮
//    chbInquiry = new QCheckBox("不再询问", &msgBox);
    chbInquiry.setText("不再询问");
    msgBox.setCheckBox(&chbInquiry); // 设置消息对话框的复选框
    /********************************************************************************
    ** @brief 设置窗口的模态性与设置窗口的标志执行顺序不能互换，否则会引发一连串问题
    **
    ** 1. 窗口置顶：初次启动窗口置顶生效，点击其他窗口窗口置顶失效
    ** 2. 窗口标志：标题栏会显示 Dialog 的 ? 按钮
    ********************************************************************************/
    msgBox.setWindowModality(Qt::NonModal); // 设置消息对话框的模态 -> 非模态
    msgBox.setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint); // 设置消息对话框的标志 -> 显示关闭按钮窗口置顶
}

void MainWindow::initStatusbarMessage()
{
//    ui->statusbar->showMessage(tr("Ready"));

    // 创建一个标签并设置居中对齐
    statusLabel.setAlignment(Qt::AlignCenter);
    statusLabel.setText("<a href=\"https://www.guchi-robotics.com\" style=\"text-decoration: none; color: #000000;\">Copyright 2023-2024 The guchi Company Ltd. All rights reserved.</a>");
    statusLabel.setOpenExternalLinks(true);

    // 将标签添加到状态栏，并设置其占用状态栏的比例为1
    ui->statusbar->addWidget(&statusLabel, 1);
}

void MainWindow::initVTK()
{
    // 读取PLY文件
    reader = vtkSmartPointer<vtkPLYReader>::New();

    // 保存PLY文件
    writer = vtkSmartPointer<vtkPLYWriter>::New();
//    writer->AddComment("This is a custom comment line."); // 添加注释信息
    writer->SetFileTypeToASCII(); // 设置文件类型为 ASCII
    writer->SetArrayName(bColorBasis ? "Scalars" : "Colors");

    // 创建点云数据
    points = vtkSmartPointer<vtkPoints>::New();
    // 创建顶点
    vertices = vtkSmartPointer<vtkCellArray>::New();
#if 0
    pid[0] = points->InsertNextPoint(0.0, 0.0, 0.0);
    vertices->InsertNextCell(1, pid);
    pid[0] = points->InsertNextPoint(1.0, 0.0, 0.0);
    vertices->InsertNextCell(1, pid);
    pid[0] = points->InsertNextPoint(0.0, 1.0, 0.0);
    vertices->InsertNextCell(1, pid);
    pid[0] = points->InsertNextPoint(0.0, 0.0, 1.0);
    vertices->InsertNextCell(1, pid);
#endif

    if (bColorBasis) {
        // 创建标量数据
        scalars = vtkSmartPointer<vtkFloatArray>::New();
        scalars->SetNumberOfComponents(1);
        scalars->SetName("Scalars");
        if (ui->actPointCloudColor->isChecked()) {
            scalars->SetNumberOfTuples(points->GetNumberOfPoints());
            for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
            {
                scalars->SetValue(i, points->GetPoint(i)[2]); // 将 z 值设置为标量值
            }
        } else {
            scalars->Initialize();
        }
    } else {
        // 创建颜色数组
        colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
        colors->SetNumberOfComponents(3);
        colors->SetName("Colors");
        colors->SetNumberOfTuples(points->GetNumberOfPoints());
        for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
        {
            switch (i) {
            case 0:
                colors->SetTuple3(i, 255, 0, 0); // 红色
                break;
            case 1:
                colors->SetTuple3(i, 0, 255, 0); // 绿色
                break;
            case 2:
                colors->SetTuple3(i, 0, 0, 255); // 蓝色
                break;
            case 3:
                colors->SetTuple3(i, 255, 255, 0); // 黄色
                break;
            default:
                colors->SetTuple3(i, 255, 0, 0); // 红色
                break;
            }
//            colors->InsertNextTuple3(255, 0, 0); // 红色
        }
    }

    // 创建多边形数据
    polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);
    polydata->SetVerts(vertices);
    if (bColorBasis) {
        polydata->GetPointData()->SetScalars(scalars); // 将标量属性设置到点云
    } else {
        polydata->GetPointData()->SetScalars(colors); // 将颜色数组与 PolyData 关联
    }

    // 创建颜色映射
    lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetNumberOfColors(256);
    lut->SetHueRange(0.67, 0.0); // 色调范围从红色到蓝色
    lut->Build();

    // 创建点云映射器
    mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polydata);
    polydata->GetBounds(bounds);
    mapper->SetScalarRange(bounds[4], bounds[5]);
    mapper->SetLookupTable(lut);

    // 创建点云演员
    actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
//    actor->GetProperty()->SetPointSize(4);
    if (bColorBasis) {
        if (false == ui->actPointCloudColor->isChecked()) {
            actor->GetProperty()->SetColor(1.0, 1.0, 1.0); // 取消设置属性，恢复默认颜色
        }
    }

    // 创建标量条
    scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
    scalarBar->SetLookupTable(mapper->GetLookupTable());
    scalarBar->SetTitle("Color Scale");
    scalarBar->SetHeight(0.8);
    scalarBar->SetWidth(0.1);
    scalarBar->SetNumberOfLabels(5);
    scalarBar->GetLabelTextProperty()->SetFontSize(9);
    scalarBar->SetOrientationToVertical();
    scalarBar->SetTextPositionToSucceedScalarBar();
    scalarBar->SetPosition(0.82, 0.1); // 这里的坐标表示相对于窗口的位置，(0,0)是左下角，(1,1)是右上角
    scalarBar->SetVisibility(false); // 隐藏标量条

    // 创建渲染器
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(actor);
    renderer->AddActor(scalarBar); // 将标量条添加到渲染器中
//    renderer->SetBackground(0.0, 0.0, 0.0); // 设置渲染器背景颜色
    setRendererBackground(SysConfig::getRendererBackground()); // 依据配置文件设置渲染器背景颜色

    // 创建渲染窗口
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renderWindow->AddRenderer(renderer); // 将渲染器添加到渲染窗口中
//    renderWindow->SetSize(800, 600); // 设置渲染窗口的大小为 800x600
//    renderWindow->SetWindowName("AreaPicking");

    // 创建选择框回调命令
    pickCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    pickCallback->SetCallback(PickCallbackFunction);

    // 创建选择框
    areaPicker = vtkSmartPointer<vtkAreaPicker>::New();
    areaPicker->AddObserver(vtkCommand::EndPickEvent, pickCallback);

    // 创建交互器
    renderWindowInteractor = new QVTKOpenGLWidget(this);
    renderWindowInteractor->SetRenderWindow(renderWindow); // 设置渲染窗口
    renderWindowInteractor->GetInteractor()->SetPicker(areaPicker);

    // 创建坐标系
    axes = vtkSmartPointer<vtkAxesActor>::New();

    // 创建坐标系标记
    marker = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    marker->SetOrientationMarker(axes); // 设置坐标系
    marker->SetInteractor(renderWindowInteractor->GetInteractor()); // 设置交互器
    marker->SetViewport(0.0, 0.0, 0.2, 0.2); // 设置坐标系标记的位置和大小

    // 创建交互器样式 使用“r”和鼠标左键绘制用于拾取的选择框。
    style = vtkSmartPointer<vtkInteractorStyleRubberBandPick>::New();
    style->SetCurrentRenderer(renderer);
    renderWindowInteractor->GetInteractor()->SetInteractorStyle(style);

    // 开启渲染和交互
    renderWindow->Render();
    /*************************************************************************
    ** @brief 移除 qvtkWidget->GetInteractor()->Start(); 的原因
    ** 因为在使用 QVTKOpenGLWidget 时，不需要显式调用 Start() 方法
    ** 相反，我们直接调用 qvtkWidget->GetInteractor()->Render() 来触发交互器的渲染
    *************************************************************************/
    renderWindowInteractor->GetInteractor()->Render();

    // 添加到布局中
    ui->verticalLayout->addWidget(renderWindowInteractor);

    writer->SetFileName("Colors.ply");
    writer->SetInputData(polydata);
    writer->Write();
}

void MainWindow::initSignalsAndSlots()
{
    /************ QAction 项对应的槽 ************/
    // 打开文件
    connect(ui->actOpen, &QAction::triggered, this, &MainWindow::slt_actOpen_triggered);
    // 自动接收
    connect(ui->actAutoRecv, &QAction::toggled, this, &MainWindow::slt_actAutoRecv_toggled);
    // 退出
    connect(ui->actQuit, &QAction::triggered, this, &MainWindow::slt_actQuit_triggered);
    // 重置
    connect(ui->actReset, &QAction::triggered, this, &MainWindow::slt_actReset_triggered);
    // 添加
    connect(ui->actAdd, &QAction::triggered, this, &MainWindow::slt_actAdd_triggered);
    // 重启
    connect(ui->actRestart, &QAction::triggered, this, &MainWindow::slt_actRestart_triggered);
    // 通用
    connect(ui->actUniversal, &QAction::triggered, this, &MainWindow::slt_actUniversal_triggered);
    // 点云颜色
    connect(ui->actPointCloudColor, &QAction::toggled, this, &MainWindow::slt_actPointCloudColor_toggled);
    // 关于
    connect(ui->actAbout, &QAction::triggered, this, &MainWindow::slt_actAbout_triggered);

    /************ 窗口关闭询问界面 ************/
    // 连接信号与槽，监听用户点击的按钮，如果用户同意关闭，则程序退出
    connect(&msgBox, &QMessageBox::buttonClicked, this, &MainWindow::buttonClicked);
    // 窗口询问改变
    connect(&chbInquiry, &QCheckBox::stateChanged, this, &MainWindow::slt_chbInquiry_stateChanged);

    /************ 设置界面 ************/
    // 窗口置顶
    connect(&dlgSet, &SetDialog::sigWindowOnTop, this, &MainWindow::sltWindowOnTop);
    // 渲染背景
    connect(&dlgSet, &SetDialog::sigRendererBackground, this, &MainWindow::sltRendererBackground);
    // 点云颜色
    connect(&dlgSet, &SetDialog::sigRestart, this, &MainWindow::slt_actRestart_triggered);

    /************ UDP 接收线程处理显示 ************/
    // 帧开始
    connect(&udpThread, &UDPRecvThread::sigFrameStart, this, &MainWindow::sltFrameStart);
    // 帧数据
    connect(&udpThread, &UDPRecvThread::sigFrameData, this, &MainWindow::sltFrameData);
    // 帧结束
    connect(&udpThread, &UDPRecvThread::sigFrameEnd, this, &MainWindow::sltFrameEnd);
    // 程序退出
    connect(&udpThread, &UDPRecvThread::sigFrameQuit, this, &MainWindow::sltFrameQuit);
    // 机器人状态
    connect(&udpThread, &UDPRecvThread::sigRobotState, &outlineThread, &OutlineImageThread::sltRobotState);
    // 控制轮廓图像线程
    connect(&udpThread, &UDPRecvThread::sigCtrlOutlineThread, this, &MainWindow::sltCtrlOutlineThread);

    /************ 轮廓图像数据显示 ************/
    // 轮廓重置
    connect(&outlineThread, &OutlineImageThread::sigOutlineReset, this, &MainWindow::sltFrameQuit);
    // 轮廓开始
    connect(&outlineThread, &OutlineImageThread::sigOutlineStart, this, &MainWindow::sltFrameStart);
    // 轮廓数据
    connect(&outlineThread, &OutlineImageThread::sigOutlineData, this, &MainWindow::sltFrameData);
    // 轮廓结束
    connect(&outlineThread, &OutlineImageThread::sigOutlineEnd, this, &MainWindow::sltFrameEnd);
}

void MainWindow::setRendererBackground(SysConfig::RendererBackground index)
{
    // 设置渲染器背景颜色
    switch (index) {
    case SysConfig::Default:
        renderer->SetBackground(0.0, 0.0, 0.0); // 黑色
        break;
    case SysConfig::Gray:
        renderer->SetBackground(0.5, 0.5, 0.5); // 灰色
        break;
    case SysConfig::NavyBlue:
        renderer->SetBackground(0.2, 0.3, 0.4); // 藏青色
        break;
    case SysConfig::DarkBlue:
        renderer->SetBackground(0.1, 0.2, 0.4); // 深蓝色
        break;
    default:
        renderer->SetBackground(0.0, 0.0, 0.0); // 黑色
        break;
    }
}

void MainWindow::showPointCloud(QString fileName)
{
    // 清除现有的点云显示
    points->Initialize();
    vertices->Initialize();

    // 读取 ply 文件
    reader->SetFileName(fileName.toStdString().c_str());
    reader->Update();

    for (vtkIdType i = 0; i < reader->GetOutput()->GetNumberOfPoints(); i++)
    {
        pid[0] = points->InsertNextPoint(reader->GetOutput()->GetPoints()->GetPoint(i));
        vertices->InsertNextCell(1, pid);
    }

    if (bColorBasis) {
        if (ui->actPointCloudColor->isChecked()) {
            // 设置一个包含标量数据的数组的大小，使其与点云数据中的点数相匹配
            scalars->SetNumberOfTuples(points->GetNumberOfPoints());
            // 插入顶点 创建一个包含单个顶点的单元，并将点云数据中的每个点与这些单元关联起来，从而定义了点云数据的拓扑结构
            for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
            {
                scalars->SetValue(i, points->GetPoint(i)[2]); // 将 z 值设置为标量值
            }
#if 0
            qDebug() << scalars->GetNumberOfTuples();
            qDebug() << scalars->GetNumberOfValues();
            qDebug() << reader->GetOutput()->GetNumberOfPoints();
            qDebug() << reader->GetOutput()->GetPoints()->GetNumberOfPoints();
#endif
        } else {
            scalars->Initialize();
            actor->GetProperty()->SetColor(1.0, 1.0, 1.0); // 取消设置属性，恢复默认颜色
        }
    } else {
        // 设置颜色
        colors->SetNumberOfTuples(points->GetNumberOfPoints());
        colors->ShallowCopy(reader->GetOutput()->GetPointData()->GetScalars());
    }

    // 获取数据范围
    polydata->GetBounds(bounds);
#if 0
    std::cout << "Minimum z value: " << bounds[4] << std::endl;
    std::cout << "Maximum z value: " << bounds[5] << std::endl;
#endif

    mapper->SetScalarRange(bounds[4], bounds[5]);

    // 根据点云数据自适应地设置相机位置和视角
    renderer->ResetCamera();

    scalarBar->SetVisibility(true); // 显示标量条
    marker->SetEnabled(1); // 启用坐标系标记

    // 刷新渲染窗口以显示新的点云数据
    renderWindow->Render();

    // 显示点云信息
    showPointCloudInfo(fileName, reader->GetOutput()->GetNumberOfPoints());
}

void MainWindow::savePointCloudFile()
{
    // 保存ply格式的点云文件
    writer->SetFileName(QString("%1.ply").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh_mm_ss.zzz")).toStdString().c_str());
    writer->SetInputData(polydata); // 设置输入数据
    writer->Write();
}

void MainWindow::showPointCloudInfo(QString fileName, vtkIdType numberOfPoints)
{
    ui->lbPointNum->setText(QString("打开文件：%1    点云个数：%2").arg(fileName).arg(numberOfPoints));
}

void MainWindow::showPointCloudInfo(vtkIdType numberOfPoints)
{
    ui->lbPointNum->setText(QString("点云个数：%1").arg(numberOfPoints));
}
