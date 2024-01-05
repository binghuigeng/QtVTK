#include "util.h"

Util::Util()
{

}

Util::~Util()
{

}

unsigned long long Util::htonll(unsigned long long hostLongLong)
{
    if (IS_LITTLE_ENDIAN)  {
        return reverseByteOrderll(hostLongLong);
    }
    else {
        return hostLongLong;
    }
}

float Util::htonf(float hostFloat)
{
    if (IS_LITTLE_ENDIAN)  {
        unsigned int netInt = htonl(*((unsigned int *)&hostFloat));
        return *((float *)&netInt);
    }
    else {
        return hostFloat;
    }
}

unsigned int Util::htonl(unsigned int hostLong)
{
    if (IS_LITTLE_ENDIAN)  {
        return reverseByteOrderl(hostLong);
    }
    else {
        return hostLong;
    }
}

unsigned short Util::htons(unsigned short hostShort)
{
    if (IS_LITTLE_ENDIAN)  {
        return reverseByteOrders(hostShort);
    }
    else {
        return hostShort;
    }
}

unsigned long long Util::ntohll(unsigned long long netLongLong)
{
    return htonll(netLongLong);
}

float Util::ntohf(float netFloat)
{
    return htonf(netFloat);
}

unsigned int Util::ntohl(unsigned int netLong)
{
    return htonl(netLong);
}

unsigned short Util::ntohs(unsigned short netShort)
{
    return htons(netShort);
}

Eigen::Matrix4d Util::poseToMatrix(const Eigen::Vector3d &eulerAngle, const Eigen::Vector3d &translation)
{
#if 0
    Eigen::AngleAxisd rollAngle(Eigen::AngleAxisd(eulerAngle(2), Eigen::Vector3d::UnitX()));
    Eigen::AngleAxisd pitchAngle(Eigen::AngleAxisd(eulerAngle(1), Eigen::Vector3d::UnitY()));
    Eigen::AngleAxisd yawAngle(Eigen::AngleAxisd(eulerAngle(0), Eigen::Vector3d::UnitZ()));

    Eigen::Matrix3d rotationMatrix;
    rotationMatrix = yawAngle * pitchAngle * rollAngle;
#endif

    Eigen::AngleAxisd rollAngle(eulerAngle(2), Eigen::Vector3d::UnitX());
    Eigen::AngleAxisd pitchAngle(eulerAngle(1), Eigen::Vector3d::UnitY());
    Eigen::AngleAxisd yawAngle(eulerAngle(0), Eigen::Vector3d::UnitZ());

    Eigen::Quaterniond q = yawAngle * pitchAngle * rollAngle;
    Eigen::Matrix4d homogeneousTransform = Eigen::Matrix4d::Identity();
    homogeneousTransform.block<3, 3>(0, 0) = q.toRotationMatrix();
    homogeneousTransform.block<3, 1>(0, 3) = translation;

    return homogeneousTransform;
}

unsigned long long Util::reverseByteOrderll(unsigned long long x)
{
    return ((((x) >> 56) & 0x00000000000000FF) |
            (((x) >> 40) & 0x000000000000FF00) |
            (((x) >> 24) & 0x0000000000FF0000) |
            (((x) >> 8) & 0x00000000FF000000) |
            (((x) << 8) & 0x000000FF00000000) |
            (((x) << 24) & 0x0000FF0000000000) |
            (((x) << 40) & 0x00FF000000000000) |
            (((x) << 56) & 0xFF00000000000000));
}

unsigned int Util::reverseByteOrderl(unsigned int x)
{
    return ((((x) >> 24) & 0x000000FF) |
            (((x) >> 8) & 0x0000FF00) |
            (((x) << 8) & 0x00FF0000) |
            (((x) << 24) & 0xFF000000));
}

unsigned short Util::reverseByteOrders(unsigned short x)
{
    return ((((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00));
}
