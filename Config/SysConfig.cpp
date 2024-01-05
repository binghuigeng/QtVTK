#include "SysConfig.h"
#include <QSettings>

SysConfig::SysConfig(QObject *parent) : QObject(parent)
{

}

SysConfig::~SysConfig()
{

}

void SysConfig::setWindowTop(bool checked)
{
    QSettings configIniWrite("./Config/config.ini", QSettings::IniFormat);
    configIniWrite.setValue("/Universal/WindowTop", checked);
}

bool SysConfig::getWindowTop()
{
    QSettings configIniRead("./Config/config.ini", QSettings::IniFormat);
    return configIniRead.value("/Universal/WindowTop", false).toBool();
}

void SysConfig::setWindowClose(bool checked)
{
    QSettings configIniWrite("./Config/config.ini", QSettings::IniFormat);
    configIniWrite.setValue("/Universal/WindowClose", checked);
}

bool SysConfig::getWindowClose()
{
    QSettings configIniRead("./Config/config.ini", QSettings::IniFormat);
    return configIniRead.value("/Universal/WindowClose", false).toBool();
}

void SysConfig::setRendererBackground(SysConfig::RendererBackground index)
{
    QSettings configIniWrite("./Config/config.ini", QSettings::IniFormat);
    configIniWrite.setValue("/Universal/RendererBackground", index);
}

SysConfig::RendererBackground SysConfig::getRendererBackground()
{
    QSettings configIniRead("./Config/config.ini", QSettings::IniFormat);
    return static_cast<RendererBackground>(configIniRead.value("/Universal/RendererBackground", SysConfig::Default).toInt());
}

void SysConfig::setPointCloudColor(bool enabled)
{
    QSettings configIniWrite("./Config/config.ini", QSettings::IniFormat);
    configIniWrite.setValue("/PointCloud/Color", enabled);
}

bool SysConfig::getPointCloudColor()
{
    QSettings configIniRead("./Config/config.ini", QSettings::IniFormat);
    return configIniRead.value("/PointCloud/Color", true).toBool();
}

void SysConfig::setUdpBase(int base)
{
    QSettings configIniWrite("./Config/config.ini", QSettings::IniFormat);
    configIniWrite.setValue("/UDP/Base", base);
}

int SysConfig::getUdpBase()
{
    QSettings configIniRead("./Config/config.ini", QSettings::IniFormat);
    return configIniRead.value("/UDP/Base", 10).toInt();
}

void SysConfig::setUdpPort(unsigned short port)
{
    QSettings configIniWrite("./Config/config.ini", QSettings::IniFormat);
    configIniWrite.setValue("/UDP/Port", port);
}

unsigned short SysConfig::getUdpPort()
{
    QSettings configIniRead("./Config/config.ini", QSettings::IniFormat);
    return configIniRead.value("/UDP/Port", 60000).toUInt();
}

void SysConfig::setAutoRecv(bool autoRecv)
{
    QSettings configIniWrite("./Config/config.ini", QSettings::IniFormat);
    configIniWrite.setValue("/UDP/AutoRecv", autoRecv);
}

bool SysConfig::getAutoRecv()
{
    QSettings configIniRead("./Config/config.ini", QSettings::IniFormat);
    return configIniRead.value("/UDP/AutoRecv", true).toBool();
}
