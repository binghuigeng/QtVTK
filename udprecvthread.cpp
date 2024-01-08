#include "udprecvthread.h"
#include "SysConfig.h"
#include "protocol.h"
#include "util.h"
#include <iostream>
#include <QDateTime>

UdpReceiver::UdpReceiver(QObject *parent) : QObject(parent)
{
    connect(&udpSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
}

UdpReceiver::~UdpReceiver()
{
    /******************************************************************
    ** @brief udpSocket.close()
    **
    ** close()函数将关闭套接字并释放绑定的端口。
    ** 在关闭套接字之前，如果有数据等待发送，则会尝试将数据发送出去。
    ** 如果发送成功，则返回true，否则返回false。
    ** 如果套接字已经处于关闭状态，则不执行任何操作。
    **
    **
    ** @brief udpSocket.abort()
    ** abort()函数将强制关闭套接字，不管是否有数据等待发送。
    ** 调用该函数将立即终止套接字，并且不会尝试将任何数据发送出去。
    ** 在调用abort()函数后，套接字将无法再使用，必须重新创建一个新的套接字对象。
    **
    **
    ** 因此，如果您希望关闭套接字并释放绑定的端口，可以使用close()函数。
    ** 如果您需要立即终止套接字并释放所有资源，则可以使用abort()函数。
    ******************************************************************/
    udpSocket.abort();
}

void UdpReceiver::sltBindPort()
{
    // 允许其他服务绑定同样的地址和端口 | 在地址和端口已经被其他套接字绑定的情况下，也应该试着重新绑定
    if (udpSocket.bind(QHostAddress::AnyIPv4, SysConfig::getUdpPort(), QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
//        qDebug("recver bind ok");
    }
    else {
        qDebug("recver bind PORT failed, please check NETWORK");
    }
//    qDebug() << "bing port " << QString::number(udpSocket.localPort());
}

void UdpReceiver::sltUnbindPort()
{
    udpSocket.close(); // 关闭套接字并释放绑定的端口
//    qDebug("recver unbind ok");
}

void UdpReceiver::readPendingDatagrams()
{
    while (udpSocket.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket.pendingDatagramSize());
        udpSocket.readDatagram(datagram.data(),datagram.size());

        emit dataReceived(datagram);
    }
}


UDPRecvThread::UDPRecvThread(QObject *parent) : QThread(parent)
{

}

UDPRecvThread::~UDPRecvThread()
{

}

void UDPRecvThread::bindPort()
{
    emit sigBindPort();
}

void UDPRecvThread::unbindPort()
{
    emit sigUnbindPort();
}

void UDPRecvThread::stopThread()
{
    quit();
    wait();
}

void UDPRecvThread::run()
{
    UdpReceiver receiver;
    connect(this, &UDPRecvThread::sigBindPort, &receiver, &UdpReceiver::sltBindPort);
    connect(this, &UDPRecvThread::sigUnbindPort, &receiver, &UdpReceiver::sltUnbindPort);
    connect(&receiver, &UdpReceiver::dataReceived, this, &UDPRecvThread::unpackUDP);
    exec();
}

void UDPRecvThread::unpackUDP(const QByteArray &datagram)
{
    if (sizeof (ROBOT_TNFO) == datagram.size()) {
        // 解析报文的机器人状态
        ROBOT_TNFO *msg = (ROBOT_TNFO*)datagram.data();
        int robotState = Util::ntohl(msg->state);
        // 发布机器人状态
        publishRobotState(robotState);
        if (0 == robotState) {
            // 启动轮廓图像线程
            emit sigCtrlOutlineThread(true);

            timestampRobot = Util::ntohl(msg->timestamp); // 记录机器人数据时间戳
        } else if (1 == robotState) {
            if (totalRobotSize + datagram.size() <= MAX_SIZE) {
                // 使用 memcpy 进行内存拷贝
                memcpy(pRobot + totalRobotSize, datagram.data(), datagram.size());
                totalRobotSize += datagram.size();
            }
        } else if (2 == robotState) {
            // 结束轮廓图像线程
            emit sigCtrlOutlineThread(false);
        }
#if 0
        QString strMsg = QString("%1, %2, %3, %4, %5, %6 distance:%7 timestamp:%8 state:%9")
                .arg(QString::number(msg->x, 'f', 6),
                     QString::number(msg->y, 'f', 6),
                     QString::number(msg->z, 'f', 6),
                     QString::number(msg->w, 'f', 6),
                     QString::number(msg->p, 'f', 6),
                     QString::number(msg->r, 'f', 6),
                     QString::number(msg->distance, 'f', 6),
                     QString::number(msg->timestamp),
                     QString::number(msg->state));
        qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << strMsg;
#endif

#if 0
        Eigen::Matrix4d homogeneousTransform = Util::poseToMatrix(Eigen::Vector3d(msg->w, msg->p, msg->r),
                                                                  Eigen::Vector3d(msg->x, msg->y, msg->z));
        std::cout << "homogeneousTransform =" << std::endl << homogeneousTransform << std::endl;
#endif
    } else if (sizeof (PC_TRANSPORT) == datagram.size()) {
        // 接收数据
        PC_TRANSPORT *msg = (PC_TRANSPORT*)datagram.data();
        // 检测报文长度
        if (sizeof (PC_TRANSPORT) == msg->head.length) {
            // 检测报文标识
            if (ID_POINT_CLOUD == msg->head.id) {
                // 检测报文校验
                unsigned short crc_check = *(unsigned short *)(datagram.data()+datagram.size()-2);
                if (cal_crc((unsigned char *)datagram.data(), datagram.size()-2) == crc_check) {
                    // 检测报文标志
                    if (FS_POINT_CLOUD == msg->flag) {
                        // 帧头
//                        qDebug("Frame Start");
                        emit sigFrameStart();
                    } else if (FD_POINT_CLOUD == msg->flag) {
                        // 数据
//                        qDebug("Frame Data");
                        emit sigFrameData(msg->data.x, msg->data.y, msg->data.z);
                    } else if (FE_POINT_CLOUD == msg->flag) {
                        // 帧尾
//                        qDebug("Frame End");
                        emit sigFrameEnd();
                    } else if (FQ_POINT_CLOUD == msg->flag) {
                        // 程序退出
//                        qDebug("Frame Quit");
                        emit sigFrameQuit();
                    }
#if 0
                    QString strMsg = QString("length:0x%1, id:0x%2, X:%3, Y:%4, Z:%5")
                            .arg(QString::number(msg->head.length, 16).toUpper(),
                                 QString::number(msg->head.id, 16).toUpper(),
                                 QString::number(msg->data.x, 'f', 6),
                                 QString::number(msg->data.y, 'f', 6),
                                 QString::number(msg->data.z, 'f', 6));
                    qDebug() << strMsg;
#endif
                } else {
                    qDebug("crc error(id:0x%04X flag:0x%04X crc:0x%04X crc_check:0x%04X)",
                           msg->head.id, msg->flag, msg->check, crc_check);
                }
            }
        }
    } else {
        qDebug("Unknown datagram");
    }
}

/* Table of CRC-CCITT constants -implements x^16+x^12+x^5+1*/
static const unsigned short crc16_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

unsigned short UDPRecvThread::cal_crc(const unsigned char *bytes, int len)
{
    unsigned short crc_check = 0; // 初始化 CRC 值为 0

    while (len--) {
        // 取数据字节和当前 CRC 的高 8 位进行异或，并用结果作为查表索引
        crc_check = crc16_table[((crc_check >> 8) ^ *bytes++) & 0xff] ^ (crc_check << 8);
    }

    return crc_check; // 返回计算得到的 CRC 值
}

void UDPRecvThread::initStorage()
{
#if 0
    // 将数组全部置为 0
    memset(pRobot, 0, sizeof (pRobot));
    memset(pOutline, 0, sizeof (pOutline));
#endif
    totalRobotSize = 0; // 用于跟踪已添加的机器人数据大小
    totalOutlineSize = 0; // 用于跟踪已添加的轮廓数据大小
    timestampRobot = 0; // 机器人数据时间戳
    timestampOutline = 0; // 轮廓数据时间戳
}

void UDPRecvThread::publishRobotState(const int &state)
{
    static int lastState = -1;
    if (lastState != state) {
        if (0 == state) {
            initStorage(); // 初始化存储
        }
        lastState = state;
        emit sigRobotState(lastState);
        qDebug("publishRobotState: %d", lastState);
    }
}
