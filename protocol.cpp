#include "protocol.h"
#include "util.h"

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
}
