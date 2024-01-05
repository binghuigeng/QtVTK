#include "protocol.h"
#include "util.h"

char pRobot[MAX_SIZE] = {0};
char pOutline[MAX_SIZE] = {0};
size_t totalRobotSize = 0; // 用于跟踪已添加的机器人数据大小
size_t totalOutlineSize = 0; // 用于跟踪已添加的轮廓数据大小
unsigned int timestampRobot = 0; // 机器人数据时间戳
long long timestampOutline = 0; // 轮廓数据时间戳

void reverse_PC_HEAD(PC_HEAD *packet)
{
    CONVERT_MEMBER_BYTE_ORDER(packet, length, Util::ntohs);
    CONVERT_MEMBER_BYTE_ORDER(packet, id, Util::ntohs);
}

void reverse_PC_DATA(PC_DATA *packet)
{
    CONVERT_MEMBER_BYTE_ORDER(packet, x, Util::ntohf);
    CONVERT_MEMBER_BYTE_ORDER(packet, y, Util::ntohf);
    CONVERT_MEMBER_BYTE_ORDER(packet, z, Util::ntohf);
}

void reverse_PC_TRANSPORT(PC_TRANSPORT *packet)
{
    reverse_PC_HEAD((PC_HEAD*)packet);
    reverse_PC_DATA((PC_DATA*)&packet->data);
    CONVERT_MEMBER_BYTE_ORDER(packet, flag, Util::ntohs);
    CONVERT_MEMBER_BYTE_ORDER(packet, check, Util::ntohs);
}

void reverse_Outline_POS(Outline_POS *packet)
{
    CONVERT_MEMBER_BYTE_ORDER(packet, x, Util::htons);
    CONVERT_MEMBER_BYTE_ORDER(packet, y, Util::htons);
    CONVERT_MEMBER_BYTE_ORDER(packet, z, Util::htons);
}

void reverse_Outline_IMAGE(Outline_IMAGE *packet)
{
    for (int i = 0; i < 2048; i++)
    {
        reverse_Outline_POS(&packet->pos[i]);
    }
    CONVERT_MEMBER_BYTE_ORDER(packet, timestamp, Util::htonll);
}

void reverse_ROBOT_TNFO(ROBOT_TNFO *packet)
{
    CONVERT_MEMBER_BYTE_ORDER(packet, x, Util::ntohf);
    CONVERT_MEMBER_BYTE_ORDER(packet, y, Util::ntohf);
    CONVERT_MEMBER_BYTE_ORDER(packet, z, Util::ntohf);
    CONVERT_MEMBER_BYTE_ORDER(packet, w, Util::ntohf);
    CONVERT_MEMBER_BYTE_ORDER(packet, p, Util::ntohf);
    CONVERT_MEMBER_BYTE_ORDER(packet, r, Util::ntohf);
    CONVERT_MEMBER_BYTE_ORDER(packet, distance, Util::ntohf);
    CONVERT_MEMBER_BYTE_ORDER(packet, timestamp, Util::ntohl);
    CONVERT_MEMBER_BYTE_ORDER(packet, state, Util::ntohl);
}
