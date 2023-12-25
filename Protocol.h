#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <iostream>

const unsigned short UDP_PORT = 0x8090;       // 端口
const unsigned short ID_POINT_CLOUD = 0xABCD; // 点云处理标识
const unsigned short FS_POINT_CLOUD = 0x980A; // 点云帧头
const unsigned short FD_POINT_CLOUD = 0x980D; // 点云数据
const unsigned short FE_POINT_CLOUD = 0x980E; // 点云帧尾
const unsigned short FQ_POINT_CLOUD = 0x980F; // 程序退出

// 点云图像数据传输报文头
typedef struct _PC_HEAD_
{
    unsigned short length; // 报文长度
    unsigned short id;     // 报文标识
} PC_HEAD;

// 点云图像数据解析
typedef struct _PC_DATA_
{
    float x; // 坐标 X
    float y; // 坐标 Y
    float z; // 坐标 Z
} PC_DATA;

// 点云图像数据传输内容
typedef struct _PC_TRANSPORT_
{
    PC_HEAD head;         // 报文头
    PC_DATA data;         // 报文内容
    unsigned short flag;  // 报文标志
    unsigned short check; // 报文校验
} PC_TRANSPORT;

#endif // PROTOCOL_H
