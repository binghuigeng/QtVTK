#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>

#define MAX_SIZE 50*1024*1024

extern char pRobot[MAX_SIZE];
extern char pOutline[MAX_SIZE];
extern size_t totalRobotSize; // 用于跟踪已添加的机器人数据大小
extern size_t totalOutlineSize; // 用于跟踪已添加的轮廓数据大小
extern unsigned int timestampRobot; // 机器人数据时间戳
extern long long timestampOutline; // 轮廓数据时间戳

const unsigned short UDP_PORT = 0x8090;       // 端口
const unsigned short ID_POINT_CLOUD = 0xABCD; // 点云处理标识
const unsigned short FS_POINT_CLOUD = 0x980A; // 点云帧头
const unsigned short FD_POINT_CLOUD = 0x980D; // 点云数据
const unsigned short FE_POINT_CLOUD = 0x980E; // 点云帧尾
const unsigned short FQ_POINT_CLOUD = 0x980F; // 程序退出

/************ 海康 3D 激光轮廓传感器（MV-DP2120-01H） ************/
// 点云图像数据传输报文头
typedef struct _PC_HEAD_
{
    unsigned short length; // 报文长度
    unsigned short id;     // 报文标识
} PC_HEAD;
void reverse_PC_HEAD(PC_HEAD *packet);

// 点云图像数据
typedef struct _PC_DATA_
{
    float x; // 坐标 X
    float y; // 坐标 Y
    float z; // 坐标 Z
} PC_DATA;
void reverse_PC_DATA(PC_DATA *packet);

// 点云图像数据传输内容
typedef struct _PC_TRANSPORT_
{
    PC_HEAD head;         // 报文头
    PC_DATA data;         // 报文内容
    unsigned short flag;  // 报文标志
    unsigned short check; // 报文校验
} PC_TRANSPORT;
void reverse_PC_TRANSPORT(PC_TRANSPORT *packet);

// 轮廓图像坐标
typedef struct _Outline_POS_
{
    short x; // 坐标 X
    short y; // 坐标 Y
    short z; // 坐标 Z
} Outline_POS;
void reverse_Outline_POS(Outline_POS *packet);

// 轮廓图像数据
typedef struct _Outline_IMAGE_
{
    Outline_POS pos[2048]; // 轮廓图像坐标
    long long timestamp;   // 数据帧时间戳
} Outline_IMAGE;
void reverse_Outline_IMAGE(Outline_IMAGE *packet);

/************ 机器人 ************/
// 机器人实时坐标
typedef struct _ROBOT_TNFO_
{
    float x;                // 位置 X
    float y;                // 位置 Y
    float z;                // 位置 Z
    float w;                // 位姿 w
    float p;                // 位姿 p
    float r;                // 位姿 r
    float distance;         // 距离
    unsigned int timestamp; // 时间戳
    int state;              // 状态 0:开始 1:运行中 2:结束
} ROBOT_TNFO;
void reverse_ROBOT_TNFO(ROBOT_TNFO *packet);

#endif // PROTOCOL_H
