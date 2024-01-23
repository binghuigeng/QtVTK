#include "outlineimagethread.h"
#include "protocol.h"
#include "util.h"
#include <thread>
#include <QDebug>
#include <QDateTime>

void __stdcall VirtualImageCallBack(MV3D_LP_IMAGE_DATA *pstImageData, void *pUser)
{
    OutlineImageThread *pThis = (OutlineImageThread *)pUser;
    if (pThis)
    {
        pThis->CallBackFunc(pstImageData, pUser);
    }
}

OutlineImageThread::OutlineImageThread(QObject *parent) : QThread(parent)
{
    m_thread_flag = false; // 线程运行标志
    m_robot_state = -1; // 机器人状态
    m_last_state = -1; // 上一次的机器人状态
}

OutlineImageThread::~OutlineImageThread()
{

}

void OutlineImageThread::startMv3dLpSDK()
{
    qDebug("dll version: %s.", MV3D_LP_GetVersion());

    ASSERT_OK(MV3D_LP_Initialize());

    unsigned int nDevNum = 0;
    ASSERT_OK(MV3D_LP_GetDeviceNumber(&nDevNum));
    qDebug("MV3D_LP_GetDeviceNumber success! nDevNum:%d.", nDevNum);
    ASSERT(nDevNum);

    // 查找设备
    qDebug("---------------------------------------------------------------\r\n");
    std::vector<MV3D_LP_DEVICE_INFO> devs(nDevNum);
    ASSERT_OK(MV3D_LP_GetDeviceList(&devs[0], nDevNum, &nDevNum));
    for (unsigned int i = 0; i < nDevNum; i++)
    {
        qDebug("Index[%d]. SerialNum[%s] IP[%s] name[%s].\r\n", i, devs[i].chSerialNumber, devs[i].chCurrentIp, devs[i].chModelName);
    }
    qDebug("---------------------------------------------------------------");

    //打开设备，默认打开第一个
    unsigned int nIndex  = 0;
    handle = NULL;
    ASSERT_OK(MV3D_LP_OpenDeviceBySN(&handle, devs[nIndex].chSerialNumber));
    qDebug("OpenDevice success.");

    // 设置轮廓图模式
    MV3D_LP_PARAM pstValue;
    memset(&pstValue, 0, sizeof(MV3D_LP_PARAM));
    pstValue.ParamInfo.stEnumParam.nCurValue = ImageMode_PointCloud_Image;
    ASSERT_OK(MV3D_LP_SetParam(handle, "ImageMode", &pstValue));
    qDebug("Set Param success.");

    ASSERT_OK(MV3D_LP_RegisterImageDataCallBack(handle, VirtualImageCallBack, this));

    // 开始工作流程
    ASSERT_OK(MV3D_LP_StartMeasure(handle));
    qDebug("Start measure success.");
}

void OutlineImageThread::quitMv3dLpSDK()
{
    ASSERT_OK(MV3D_LP_StopMeasure(handle));
    ASSERT_OK(MV3D_LP_CloseDevice(&handle));
    ASSERT_OK(MV3D_LP_Finalize());

    qDebug("Main done!");
}

void OutlineImageThread::setThreadFlag(bool flag)
{
    m_thread_flag = flag;
}

void OutlineImageThread::CallBackFunc(MV3D_LP_IMAGE_DATA *pstImageData, void *pUser)
{
    if (NULL != pstImageData)
    {
//        qDebug("get image success: framenum (%d) height(%d) width(%d)  len (%d)!", pstImageData->nFrameNum,
//            pstImageData->nHeight, pstImageData->nWidth, pstImageData->nDataLen);
//        qDebug("get image success: timeStamp (%lld)!\r\n", pstImageData->nTimeStamp);
        if (0 == m_robot_state) {
            if (m_last_state != m_robot_state) {
                timestampOutline = pstImageData->nTimeStamp; // 记录轮廓图像数据时戳
                m_last_state = m_robot_state;
                // 轮廓重置
                emit sigOutlineReset();
                qDebug("record timestamp");
            }
        } else if (1 == m_robot_state) {
            if (totalOutlineSize + pstImageData->nDataLen + sizeof (int64_t) <= MAX_SIZE) {
                // 使用 memcpy 进行内存拷贝，存储图像数据
                memcpy(pOutline + totalOutlineSize, pstImageData->pData, pstImageData->nDataLen);
                totalOutlineSize += pstImageData->nDataLen;
                // 存储设备上报的时间戳
                memcpy(pOutline + totalOutlineSize, &pstImageData->nTimeStamp, sizeof (int64_t));
                totalOutlineSize += sizeof (int64_t);

//                qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << " nTimeStamp " << pstImageData->nTimeStamp;
            }
//            qDebug("get image success: framenum (%d) height(%d) width(%d)  len (%d)!", pstImageData->nFrameNum,
//                pstImageData->nHeight, pstImageData->nWidth, pstImageData->nDataLen);
//            qDebug("get image success: timeStamp (%lld)!\r\n", pstImageData->nTimeStamp);
        } else if (2 == m_robot_state) {
            m_robot_state = -1;
            m_last_state = -1;
            // 点云拼接
            pointCloudStitching(totalRobotSize, totalOutlineSize);
        }
    }
    else
    {
        qDebug("pstImageData is null!\r\n");
    }
}

void OutlineImageThread::sltRobotState(const int &state)
{
    m_robot_state = state;
}

void OutlineImageThread::run()
{
    // 循环主体
    while (m_thread_flag) {
        ASSERT_OK(MV3D_LP_SoftTrigger(handle));
#if 0
        msleep(8); // 线程休眠8ms
        Sleep(8); // 休眠8ms
#endif
        /*****************************************************************************
        ** 1）使用说明
        ** std::this_thread::sleep_for函数是C11的休眠函数，表示当前线程休眠一段时间，
        ** 休眠期间不与其他线程竞争CPU，根据线程需求，等待若干时间。
        ** 由于是一个跨平台的函数，因此在代码中大量应用，避免了在不同平台之间所以通过宏定义编译问题。
        ** 在windows下，可以简单替代Sleep, 在Linux下，替代usleep
        **
        ** 2）调用例子
        ** 头文件定义：#include <thread>
        ** std::this_thread::sleep_for(std::chrono::seconds(1)); // 休眠 1 秒
        ** std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 休眠50毫秒
        ** std::this_thread::sleep_for(std::chrono::microseconds(1)); // 休眠 1 微秒
        ** std::this_thread::sleep_for(std::chrono::nanoseconds(100)); // 休眠 100 纳秒
        *****************************************************************************/
        std::this_thread::sleep_for(std::chrono::milliseconds(8)); // 休眠8毫秒
    }

    // 在 m_thread_flag == false 时结束线程任务
    quit(); // 相当于 exit(0)，退出线程的事件循环
}

void OutlineImageThread::pointCloudStitching(const size_t &robotSize, const size_t &outlineSize)
{
    /*****************************************************************************
    ** 点云拼接规则
    ** 机器人实时坐标(4*4) * 滚压头求逆(4*4) * 线扫相机(4*4) * 线扫相机轮廓图像数据(4*1)
    *****************************************************************************/
    // 轮廓开始
    emit sigOutlineStart();

    // 机器人实时坐标的齐次变换矩阵
    Eigen::Matrix4d robotMatrix = Eigen::Matrix4d::Identity();

    // 滚压头的齐次变换矩阵
    Eigen::Matrix4d rollHeadMatrix = Util::poseToMatrix(Eigen::Vector3d(-90.146, 0.071, 150.123),
                                                        Eigen::Vector3d(-111.651, -226.086, 512.811));

    // 滚压头的齐次变换矩阵的逆矩阵
    Eigen::Matrix4d rollHeadMatrixInverse = rollHeadMatrix.inverse();

    // 线扫相机的齐次变换矩阵
    Eigen::Matrix4d lineScanCameraMatrix = Util::poseToMatrix(Eigen::Vector3d(-180.000, 53.017, -120.016),
                                                              Eigen::Vector3d(-52.754, -232.347, 492.449));

    // 初始化齐次变换矩阵
    Eigen::Matrix4d homogeneousTransform = Eigen::Matrix4d::Identity();

    // 机器人实时坐标
    ROBOT_TNFO robot;

    // 轮廓图像数据
    Outline_IMAGE outline;

    // 计算出最小轮廓帧数量
    int minFrameNum = MIN(robotSize/sizeof (ROBOT_TNFO), outlineSize/(2048*6+8));

    qDebug("totalRobotSize(%llu) totalOutlineSize(%llu) minFrameNum(%d)", totalRobotSize, totalOutlineSize, minFrameNum);
#if 1
    int index = 0;
    for (int i = 0; i < minFrameNum; i++)
    {
        // 若索引不小于帧数量，则结束循环
        if (index >= minFrameNum)
        {
            break;
        }

        // 读取机器人实时坐标
        memcpy(&robot, pRobot + i * sizeof (ROBOT_TNFO), sizeof (ROBOT_TNFO));
        reverse_ROBOT_TNFO(&robot);
#if 0
        QString strMsg = QString("%1, %2, %3, %4, %5, %6 distance:%7 timestamp:%8 state:%9")
                .arg(QString::number(robot.x, 'f', 6),
                     QString::number(robot.y, 'f', 6),
                     QString::number(robot.z, 'f', 6),
                     QString::number(robot.w, 'f', 6),
                     QString::number(robot.p, 'f', 6),
                     QString::number(robot.r, 'f', 6),
                     QString::number(robot.distance, 'f', 6),
                     QString::number(robot.timestamp),
                     QString::number(robot.state));
        qDebug() << "unpack " << strMsg;
#endif
        // 计算出每帧机器人实时坐标的齐次变换矩阵
        robotMatrix = Util::poseToMatrix(Eigen::Vector3d(robot.w, robot.p, robot.r),
                                         Eigen::Vector3d(robot.x, robot.y, robot.z));
        // 计算出每帧轮廓数据所需的齐次变换矩阵
        homogeneousTransform = robotMatrix * rollHeadMatrixInverse * lineScanCameraMatrix;
//        homogeneousTransform = robotMatrix * rollHeadMatrix * lineScanCameraMatrix;
//        homogeneousTransform = robotMatrix * rollHeadMatrixInverse;
//        homogeneousTransform = robotMatrix;

        // 读取轮廓图像数据
        memcpy(&outline, pOutline + index * sizeof (Outline_IMAGE), sizeof (Outline_IMAGE));

        // 计算时间戳差值
        long long stepSize = abs((outline.timestamp - timestampOutline) - (robot.timestamp - timestampRobot));

        // 计算离第 i 帧机器人实时坐标最近的轮廓图像数据索引
        for (int j = index+1; j < minFrameNum; j++)
        {
            // 读取轮廓图像数据
            memcpy(&outline, pOutline + j * sizeof (Outline_IMAGE), sizeof (Outline_IMAGE));

            // 计算时间戳差值
            if (abs((outline.timestamp - timestampOutline) - (robot.timestamp - timestampRobot)) > stepSize)
            {
                break;
            }
            stepSize = abs((outline.timestamp - timestampOutline) - (robot.timestamp - timestampRobot));
            index = j;
        }

        // 读取离第 i 帧机器人实时坐标最近的轮廓图像数据
        memcpy(&outline, pOutline + index * sizeof (Outline_IMAGE), sizeof (Outline_IMAGE));

        // 读取轮廓图像数据
        for (int k = 0; k < 2048; k++)
        {
//            qDebug("******PointCloud data: x(%d) y(%d) z (%d)!", outline.pos[k].x, outline.pos[k].y, outline.pos[k].z);
            // 数据过滤
            if (outline.pos[k].x != -32768 && outline.pos[k].y != -32768 && outline.pos[k].z != -32768)
            {
//                qDebug("PointCloud data: x(%d) y(%d) z (%d)!", outline.pos[k].x * 0.001 * 2, outline.pos[k].y * 0.001 * 2, outline.pos[k].z * 0.001 * 2);
                Eigen::Vector4d pos = homogeneousTransform * Eigen::Vector4d(outline.pos[k].x * 0.001 * 2,
                                                                             outline.pos[k].y * 0.001 * 2,
                                                                             outline.pos[k].z * 0.001 * 2,
                                                                             1);
//                Eigen::Vector4d pos = homogeneousTransform * (lineScanCameraMatrix * Eigen::Vector4d(outline.pos[k].x * 0.001 * 2,
//                                                                                                     outline.pos[k].y * 0.001 * 2,
//                                                                                                     outline.pos[k].z * 0.001 * 2,
//                                                                                                     1));
                // 轮廓数据
//                emit sigOutlineData(outline.pos[k].x * 0.001 * 2, outline.pos[k].y * 0.001 * 2, outline.pos[k].z * 0.001 * 2);
                emit sigOutlineData(pos(0), pos(1), pos(2));
            }
        }

        // 轮廓图像数据索引加一
        index++;
    }
#endif

    // 轮廓结束
    emit sigOutlineEnd();
}
