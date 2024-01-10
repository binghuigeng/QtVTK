#ifndef UDPRECVTHREAD_H
#define UDPRECVTHREAD_H

#include <QThread>
#include <QUdpSocket>

class UdpReceiver : public QObject
{
    Q_OBJECT
public:
    explicit UdpReceiver(QObject *parent = nullptr);
    ~UdpReceiver();

public slots:
    /// @brief 绑定端口
    void sltBindPort();

    /// @brief 关闭套接字并释放绑定的端口
    void sltUnbindPort();

signals:
    /// @brief dataReceived 读取 UDP 传入的数据
    /// @param data UDP 传入的数据
    void dataReceived(const QByteArray &datagram);

private slots:
    /// @brief 读取 UDP 传入的数据
    void readPendingDatagrams();

private:
    /// @brief udpSocket UDP 套接字
    QUdpSocket udpSocket;
};


/// @brief UDP 接收线程
class UDPRecvThread : public QThread
{
    Q_OBJECT
public:
    explicit UDPRecvThread(QObject *parent = nullptr);
    ~UDPRecvThread();

    /// @brief 绑定端口
    void bindPort();

    /// @brief 关闭套接字并释放绑定的端口
    void unbindPort();

    /// @brief 结束线程
    void stopThread();

protected:
    void run() override;

signals:
    /// @brief 绑定端口
    void sigBindPort();

    /// @brief 关闭套接字并释放绑定的端口
    void sigUnbindPort();

    /// @brief 帧开始
    void sigFrameStart();

    /// @brief 帧数据
    /// @param x 坐标 X
    /// @param y 坐标 Y
    /// @param z 坐标 Z
    void sigFrameData(const double &x, const double &y, const double &z);

    /// @brief 帧结束
    void sigFrameEnd();

    /// @brief 程序退出
    void sigFrameQuit();

    /// @brief 机器人状态
    /// @param state 状态 0:开始 1:运行中 2:结束
    void sigRobotState(const int &state);

    /// @brief 控制轮廓图像线程
    /// @param enable 线程使能 true:启动轮廓图像线程 false:结束轮廓图像线程
    void sigCtrlOutlineThread(const bool &enable);

private slots:
    /// @brief 解析 UDP 传入的数据
    /// @param data UDP 传入的数据
    void unpackUDP(const QByteArray &datagram);

private:
    /// @brief CRC16 校验函数
    /// @param bytes 指向要计算 CRC 的数据的指针
    /// @param len 要计算 CRC 的数据的长度
    /// @return 计算得到的 CRC16 校验值
    unsigned short cal_crc(const unsigned char *bytes, int len);

    /// @brief 初始化存储
    void initStorage();

    /// @brief 发布机器人状态
    /// @param state 状态 0:开始 1:运行中 2:结束
    void publishRobotState(const int &state);
};

#endif // UDPRECVTHREAD_H
