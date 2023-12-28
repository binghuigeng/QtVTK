#ifndef SYSCONFIG_H
#define SYSCONFIG_H

#include <QObject>

/*
 * @file SysConfig.h
 * @brief 与配置文件相关的函数
 */
class SysConfig : public QObject
{
    Q_OBJECT
public:
    explicit SysConfig(QObject *parent = nullptr);
    ~SysConfig();

    /// @brief 渲染背景
    enum RendererBackground {
        Default,
        Gray,
        NavyBlue,
        DarkBlue
    };

    /// @brief 设置窗口置顶
    /// @param checked true:窗口置顶 false:取消窗口置顶
    static void setWindowTop(bool checked);

    /// @brief 获取窗口置顶
    /// @return true:窗口置顶 false:取消窗口置顶
    static bool getWindowTop();

    /// @brief 设置窗口询问
    /// @param checked true:窗口询问 false:取消窗口询问
    static void setWindowClose(bool checked);

    /// @brief 获取窗口询问
    /// @return true:窗口询问 false:取消窗口询问
    static bool getWindowClose();

    /// @brief 设置渲染背景
    /// @param index Default:默认 Gray:灰色 NavyBlue:藏青色 DarkBlue:深蓝色
    static void setRendererBackground(SysConfig::RendererBackground index);

    /// @brief 获取渲染背景
    /// @return Default:默认 Gray:灰色 NavyBlue:藏青色 DarkBlue:深蓝色
    static RendererBackground getRendererBackground();

    /// @brief 设置点云颜色
    /// @param enabled true:显示颜色 false:取消显示颜色
    static void setPointCloudColor(bool enabled);

    /// @brief 获取点云颜色
    /// @return true:显示颜色 false:取消显示颜色
    static bool getPointCloudColor();

    /// @brief 设置 UDP 端口的进制显示
    /// @param port UDP 端口的进制显示
    static void setUdpBase(int base);

    /// @brief 获取 UDP 端口的进制显示
    /// @return UDP 端口的进制显示
    static int getUdpBase();

    /// @brief 设置 UDP 端口
    /// @param port UDP 端口
    static void setUdpPort(unsigned short port);

    /// @brief 获取 UDP 端口
    /// @return UDP 端口
    static unsigned short getUdpPort();

    /// @brief 设置自动接收
    /// @param autoRecv true:自动接收 false:取消自动接收
    static void setAutoRecv(bool autoRecv);

    /// @brief 获取自动接收
    /// @return true:自动接收 false:取消自动接收
    static bool getAutoRecv();
};

#endif // SYSCONFIG_H
