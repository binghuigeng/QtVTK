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
    void    stopThread();

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
    void sigFrameData(double x, double y, double z);

    /// @brief 帧结束
    void sigFrameEnd();

    /// @brief 程序退出
    void sigFrameQuit();

private slots:
    /// @brief 解析 UDP 传入的数据
    /// @param data UDP 传入的数据
    void unpackUDP(const QByteArray &datagram);

private:
    /// @brief CRC16 校验函数
    unsigned short cal_crc(const unsigned char *bytes, int len);

private:
    /// @brief udpSocket UDP 套接字
//    QUdpSocket udpSocket;

//    UdpReceiver receiver;
};

#endif // UDPRECVTHREAD_H
