#ifndef UTIL_H
#define UTIL_H

/*****************************************
** @brief 判断本机的主机字节序
** @note 网络字节序为大端模式
** 大端模式：低地址存放高字节，高地址存放低字节
** 小端模式：低地址存放低字节，高地址存放高字节
*****************************************/
static union
{
    unsigned char ct_order[4];
    unsigned int ut_order;
} host_order = {{'L', '?', '?', 'B'}};

/// @brief 判断本机的主机字节序是否为大端模式
#define IS_BIG_ENDIAN ('B' == (char)host_order.ut_order)

/// @brief 判断本机的主机字节序是否为小端模式
#define IS_LITTLE_ENDIAN ('L' == (char)host_order.ut_order)

/// @brief 转换数据包PACKET中的成员MEMBER的字节序，转换函数为FUNC
#define CONVERT_MEMBER_BYTE_ORDER(PACKET, MEMBER, FUNC) \
    (PACKET->MEMBER = FUNC(PACKET->MEMBER))

/// @brief 使用宏来定义一个计算最小值的函数
#ifndef MIN
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#endif

#include <QObject>
#include <Eigen/Dense>

class Util
{
public:
    Util();
    ~Util();

    /// @brief 把uint64_t类型从主机序转换到网络序
    static unsigned long long htonll(unsigned long long hostLongLong);

    /// @brief 把float类型从主机序转换到网络序
    static float htonf(float hostFloat);

    /// @brief 把uint32_t类型从主机序转换到网络序
    static unsigned int htonl(unsigned int hostLong);

    /// @brief 把uint16_t类型从主机序转换到网络序
    static unsigned short htons(unsigned short hostShort);

    /// @brief 把uint64_t类型从网络序转换到主机序
    static unsigned long long ntohll(unsigned long long netLongLong);

    /// @brief 把float类型从网络序转换到主机序
    static float ntohf(float netFloat);

    /// @brief 把uint32_t类型从网络序转换到主机序
    static unsigned int ntohl(unsigned int netLong);

    /// @brief 把uint16_t类型从网络序转换到主机序
    static unsigned short ntohs(unsigned short netShort);

    /// @brief 位姿（位置和欧拉角）转齐次变换矩阵
    /// @param eulerAngle 欧拉角
    /// @param translation 平移向量
    /// @return 齐次变换矩阵
    static Eigen::Matrix4d poseToMatrix(const Eigen::Vector3d& eulerAngle,
                                        const Eigen::Vector3d& translation);

private:
    /// @brief 将一8字节整数反序后返回。即若输入0xABCD5678，则返回0x7856CDAB
    static unsigned long long reverseByteOrderll(unsigned long long x);

    /// @brief 将一4字节整数反序后返回。即若输入0xABCD5678，则返回0x7856CDAB
    static unsigned int reverseByteOrderl(unsigned int x);

    /// @brief 将一2字节整数反序后返回。即若输入0xABCD，则返回0xCDAB
    static unsigned short reverseByteOrders(unsigned short x);
};

#endif // UTIL_H
