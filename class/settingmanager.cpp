#include "settingmanager.h"
#include "plugin/pluginsystem.h"
#include <QApplication>
#include <QFile>
#include <QStandardPaths>

SettingManager *SettingManager::m_instance = nullptr;

SettingManager::SettingManager(QObject *parent)
    : QObject(parent), m_toolwin(true), m_wintool(true),
      m_toolGridSize(TOOLGRIDSIZE),
      m_toolBox(
          QKeySequence(Qt::KeyboardModifier::ShiftModifier | Qt::Key_Space)),
      m_toolwinMod(Qt::KeyboardModifier::ControlModifier),
      m_toolMouse(Qt::MouseButton::MidButton) {
  m_instance = this;
}

SettingManager *SettingManager::instance() { return m_instance; }

bool SettingManager::loadSettings(QString filename) {

#define CORRECTINFO(info)                                                      \
  info.pluginIndex = plgsys->pluginIndexByProvider(info.provider);             \
  if (info.isPlugin) {                                                         \
    info.process = plgsys->plugin(info.pluginIndex)->pluginName();             \
  }

  QString strConfigPath = filename.isEmpty()
                              ? QString("%1/%2/%3/config.conf")
                                    .arg(QStandardPaths::writableLocation(
                                        QStandardPaths::ConfigLocation))
                                    .arg(qApp->organizationName())
                                    .arg(qApp->applicationName())
                              : filename;

  QFile f(strConfigPath);
  QDataStream stream(&f);
  if (f.open(QFile::ReadOnly)) {
    // 开始读配置文件咯
    static char header[] = "WINGTOOL";
    char buffer[8];
    stream.readRawData(buffer, 8);
    if (memcmp(header, buffer, 8)) {
      // 如果文件头不对劲，就视为非法配置
      return false;
    }

    auto plgsys = PluginSystem::instance();

    // General
    stream >> m_toolwin >> m_wintool >> m_toolGridSize >> m_toolBox >>
        m_toolwinMod >> m_toolMouse;

    // 读取结束，提示可以加载基础配置内容了
    emit loadedGeneral();

    // 读取 Hotkey 的相关信息
    int len;
    stream >> len; // 先读取一下有几个
    for (auto i = 0; i < len; i++) {
      ToolStructInfo buf;
      stream >> buf.enabled >> buf.isPlugin >> buf.seq;
      if (buf.isPlugin) {
        stream >> buf.serviceID;
        QByteArray arr;
        stream >> arr;
        buf.provider = QString::fromUtf8(arr);
        stream >> arr;
        buf.params = QString::fromUtf8(arr);
        stream >> arr;

        auto pi = plgsys->pluginIndexByProvider(buf.provider);
        // 找不到了，插件丢失或者不兼容
        if (pi < 0)
          continue;
        // 检查兼容性，不兼容出门左拐
        if (!Utilities::isPluginCompatible(plgsys->plugin(pi), arr)) {
          continue;
        }
        // 经历过重重检验，合格入库
        CORRECTINFO(buf);
        emit addHotKeyInfo(buf);
      } else {
        // 如果是打开文件就没这么多事情了
        QByteArray arr;
        stream >> arr;
        buf.process = QString::fromUtf8(arr);
        stream >> arr;
        buf.params = QString::fromUtf8(arr);
        emit addHotKeyInfo(buf);
      }
    }

    // 下面继续读取 ToolWin 相关信息
    for (auto i = 0; i < 9; i++) {
      ToolStructInfo buf;

      // 对于 ToolWin 来说，这个成员是决定性的
      // 只有这个标志位有效，这个工具才有意义

      stream >> buf.enabled;
      if (!buf.enabled)
        continue;
      stream >> buf.isPlugin;
      if (buf.isPlugin) {
        stream >> buf.serviceID;
        QByteArray arr;
        stream >> arr;
        buf.provider = QString::fromUtf8(arr);
        stream >> arr;
        buf.params = QString::fromUtf8(arr);
        auto pi = plgsys->pluginIndexByProvider(buf.provider);
        // 找不到了，插件丢失或者不兼容
        if (pi < 0)
          continue;
        // 检查兼容性，不兼容出门左拐
        if (!Utilities::isPluginCompatible(plgsys->plugin(pi), arr)) {
          continue;
        }
        // 经历过重重检验，合格入库
        CORRECTINFO(buf);
        emit setToolWinInfo(i, buf);
      } else { // 如果是打开文件就没这么多事情了
        QByteArray arr;
        stream >> arr;
        buf.process = QString::fromUtf8(arr);
        stream >> arr;
        buf.params = QString::fromUtf8(arr);
        emit setToolWinInfo(i, buf);
      }
    }

    // 下面读取 WinTool 相关信息
    stream >> len; // 先读一下有几个
    for (auto i = 0; i < len; i++) {
      ToolStructInfo buf;

      // 对于 WinTool 来说， enabled 就没用了
      // 只存储相关基础信息就可以了
      stream >> buf.isPlugin;
      if (buf.isPlugin) {
        stream >> buf.serviceID;
        QByteArray arr;
        stream >> arr;
        buf.provider = QString::fromUtf8(arr);
        stream >> arr;
        buf.params = QString::fromUtf8(arr);
        stream >> arr;
        auto pi = plgsys->pluginIndexByProvider(buf.provider);
        // 找不到了，插件丢失或者不兼容
        if (pi < 0)
          continue;
        // 检查兼容性，不兼容出门左拐
        if (!Utilities::isPluginCompatible(plgsys->plugin(pi), arr)) {
          continue;
        }

        // 经历过重重检验，合格入库
        CORRECTINFO(buf);
        emit addWinToolInfo(buf);
      } else {
        QByteArray arr;
        stream >> arr;
        buf.process = QString::fromUtf8(arr);
        stream >> arr;
        buf.params = QString::fromUtf8(arr);
        emit addWinToolInfo(buf);
      }
    }
  } else {
    // 如果没有，就加载默认配置
    emit loadedGeneral();
  }
  f.close();
  return true;
}

bool SettingManager::saveSettings() {
  QString strConfigPath =
      QString("%1/%2/%3/config.conf")
          .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
          .arg(qApp->organizationName())
          .arg(qApp->applicationName());
  return exportSettings(strConfigPath);
}

bool SettingManager::exportSettings(QString filename) {
  QFile f(filename);
  QDataStream stream(&f);
  if (f.open(QFile::WriteOnly)) {
    // 写文件头
    static char header[] = "WINGTOOL";
    stream.writeRawData(header, 8);
    // General
    stream << m_toolwin << m_wintool << m_toolGridSize << m_toolBox
           << m_toolwinMod << m_toolMouse;
    // 有些配置直接保存到 CenterWindow 里面了，为了减少内存占用
    emit sigSaveConfig(stream);
    // 至此，保存完毕
    f.close();
    return true;
  }
  return false;
}

void SettingManager::resetSettings() {}

int SettingManager::toolGridSize() const { return m_toolGridSize; }

void SettingManager::setToolGridSize(const int v) {
  if (v > 0) {
    m_toolGridSize = v;
    emit sigToolGridSizeChanged(v);
  }
}

QKeySequence SettingManager::toolBoxHotkey() const { return m_toolBox; }

void SettingManager::setToolBoxHotkey(const QKeySequence seq) {
  m_toolBox = seq;
  emit sigToolBoxHotkeyChanged(seq);
}

Qt::KeyboardModifier SettingManager::toolwinMod() const { return m_toolwinMod; }

void SettingManager::setToolwinMod(const Qt::KeyboardModifier &toolwinMod) {
  m_toolwinMod = toolwinMod;
  emit sigToolwinModChanged(toolwinMod);
}

Qt::MouseButton SettingManager::toolwinMouseBtn() const { return m_toolMouse; }

void SettingManager::setToolMouseBtn(const Qt::MouseButton &toolMouse) {
  m_toolMouse = toolMouse;
  emit sigToolwinMouseBtnChanged(toolMouse);
}

bool SettingManager::toolwinEnabled() const { return m_toolwin; }

void SettingManager::setToolwinEnabled(bool toolwin) {
  m_toolwin = toolwin;
  emit sigToolwinEnabledChanged(toolwin);
}

bool SettingManager::wintoolEnabled() const { return m_wintool; }

void SettingManager::setWintoolEnabled(bool wintool) {
  m_wintool = wintool;
  emit sigWintoolEnabledChanged(wintool);
}
