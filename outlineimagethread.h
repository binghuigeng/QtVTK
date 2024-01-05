#ifndef OUTLINEIMAGETHREAD_H
#define OUTLINEIMAGETHREAD_H

#include <QThread>
#include "common/common.hpp"

class OutlineImageThread : public QThread
{
    Q_OBJECT
public:
    explicit OutlineImageThread(QObject *parent = nullptr);
    ~OutlineImageThread();

    /// @brief 启动 Mv3dLpSDK
    void startMv3dLpSDK();

    /// @brief 退出 Mv3dLpSDK
    void quitMv3dLpSDK();

    /// @brief 设置线程运行标志
    /// @param flag 线程运行标志
    void setThreadFlag(bool flag);

    /// @brief 获取图像数据的回调函数
    /// @param pstImageData 回调函数指针
    /// @param pUser 用户自定义变量
    void CallBackFunc(MV3D_LP_IMAGE_DATA *pstImageData, void *pUser);

public slots:
    /// @brief 机器人状态
    /// @param state 状态 0:开始 1:运行中 2:结束
    void sltRobotState(int state);

protected:
    /// @brief 线程任务
    void run() override;

signals:
    /// @brief 轮廓重置
    void sigOutlineReset();

    /// @brief 轮廓开始
    void sigOutlineStart();

    /// @brief 轮廓数据
    /// @param x 坐标 X
    /// @param y 坐标 Y
    /// @param z 坐标 Z
    void sigOutlineData(double x, double y, double z);

    /// @brief 轮廓结束
    void sigOutlineEnd();

private:
    /// @brief 点云拼接
    /// @param totalRobotSize 机器人数据大小
    /// @param totalOutlineSize 轮廓数据大小
    void pointCloudStitching(const size_t &robotSize, const size_t &outlineSize);

private:
    /// @brief 线程运行标志
    bool m_thread_flag;

    /// @brief 机器人状态
    int m_robot_state;

    /// @brief 设备句柄
    void *handle;
};

#endif // OUTLINEIMAGETHREAD_H
