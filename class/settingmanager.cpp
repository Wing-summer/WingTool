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
      m_runWin(QKeySequence(Qt::KeyboardModifier::MetaModifier | Qt::Key_R)),
      m_toolwinMod(Qt::KeyboardModifier::ControlModifier),
      m_toolMouse(Qt::MouseButton::MidButton), ismod(false), loaded(false) {

  auto pathdir =
      QString("%1/%2/%3")
          .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
          .arg(qApp->organizationName())
          .arg(qApp->applicationName());
  QDir().mkpath(pathdir);

  configfile = pathdir + "/config.conf";

  m_instance = this;
}

SettingManager *SettingManager::instance() { return m_instance; }

bool SettingManager::loadSettings(QString filename) {

#define FAILRETURN                                                             \
  {                                                                            \
    f.close();                                                                 \
    return false;                                                              \
  }

#define VAILDSTR(str)                                                          \
  if (!Utilities::isVaildString(arr, str))                                     \
    FAILRETURN;

#define CORRECTINFO(info)                                                      \
  if (info.isPlugin) {                                                         \
    info.pluginIndex = plgsys->pluginIndexByProvider(info.provider);           \
    info.process = plgsys->plugin(info.pluginIndex)->pluginName();             \
  }

  QString strConfigPath = filename.isEmpty() ? configfile : filename;

  QFile f(strConfigPath);
  QDataStream stream(&f);
  if (f.open(QFile::ReadOnly)) {
    // 开始读配置文件咯
    static char header[] = "WINGTOOL";
    char buffer[8];
    stream.readRawData(buffer, 8);
    if (memcmp(header, buffer, 8)) {
      // 如果文件头不对劲，就视为非法配置
      loadedGeneral();
      FAILRETURN;
    }

    // 读取配置文件
    char ver;
    stream.readRawData(&ver, 1);

    if (ver != CONFIGVER) {
      emit loadedGeneral();
      FAILRETURN;
    }

    if (filename.length())
      ismod = true;

    // 如果已经加载了设置，重置重新加载
    if (loaded)
      resetSettings();

    auto plgsys = PluginSystem::instance();

    // General
    stream >> m_toolwin >> m_wintool >> m_toolGridSize >> m_runWin >>
        m_toolBox >> m_toolwinMod >> m_toolMouse;

    // check
    if (m_toolGridSize < 30 || m_toolGridSize > 60)
      FAILRETURN;
    if (m_toolBox.isEmpty())
      FAILRETURN;
    switch (m_toolwinMod) {
    case Qt::AltModifier:
    case Qt::ControlModifier:
    case Qt::MetaModifier:
    case Qt::ShiftModifier:
      break;
    default:
      FAILRETURN;
    }
    switch (m_toolMouse) {
    case Qt::LeftButton:
    case Qt::MiddleButton:
    case Qt::RightButton:
    case Qt::XButton1:
    case Qt::XButton2:
      break;
    default:
      FAILRETURN;
    }

    // 读取结束，提示可以加载基础配置内容了
    emit loadedGeneral();

    // 读取 Hotkey 的相关信息
    int len;
    stream >> len; // 先读取一下有几个
    if (len < 0)
      return false;
    for (auto i = 0; i < len; i++) {
      ToolStructInfo buf;
      stream >> buf.enabled >> buf.isPlugin >> buf.seq;
      if (buf.seq.isEmpty())
        FAILRETURN;
      if (buf.isPlugin) {
        QByteArray arr;
        stream >> arr;
        VAILDSTR(buf.serviceName);
        stream >> arr;
        VAILDSTR(buf.provider);
        stream >> arr;
        VAILDSTR(buf.params);
        stream >> arr;
        VAILDSTR(buf.fakename);

        auto pi = plgsys->pluginIndexByProvider(buf.provider);
        // 找不到了，插件丢失或者不兼容
        if (pi < 0)
          continue;
        // 检查兼容性，看看有没有对应的服务名
        auto plg = plgsys->plugin(pi);
        auto sindex = plgsys->pluginServiceNames(plg).indexOf(buf.serviceName);
        if (sindex < 0) {
          continue;
        }
        buf.serviceID = sindex; // 修复填进去
        // 经历过重重检验，合格入库
        CORRECTINFO(buf);
        emit addHotKeyInfo(buf);
      } else {
        // 如果是打开文件就没这么多事情了
        QByteArray arr;
        stream >> arr;
        VAILDSTR(buf.process);
        stream >> arr;
        VAILDSTR(buf.params);
        stream >> arr;
        VAILDSTR(buf.fakename);
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
        QByteArray arr;
        stream >> arr;
        VAILDSTR(buf.serviceName);
        stream >> arr;
        VAILDSTR(buf.iconpath);
        buf.icon = Utilities::trimIconFromFile(buf.iconpath);
        if (buf.icon.isNull())
          buf.iconpath.clear();
        stream >> arr;
        VAILDSTR(buf.provider);
        stream >> arr;
        VAILDSTR(buf.params);
        stream >> arr;
        VAILDSTR(buf.fakename);

        auto pi = plgsys->pluginIndexByProvider(buf.provider);
        // 找不到了，插件丢失或者不兼容
        if (pi < 0)
          continue;
        // 检查兼容性，看看有没有对应的服务名
        auto plg = plgsys->plugin(pi);
        auto sindex = plgsys->pluginServiceNames(plg).indexOf(buf.serviceName);
        if (sindex < 0) {
          continue;
        }
        buf.serviceID = sindex; // 修复填进去
        // 经历过重重检验，合格入库
        CORRECTINFO(buf);
        emit setToolWinInfo(i, buf);
      } else { // 如果是打开文件就没这么多事情了
        QByteArray arr;
        stream >> arr;
        VAILDSTR(buf.process);
        stream >> arr;
        VAILDSTR(buf.params);
        stream >> arr;
        VAILDSTR(buf.fakename);
        stream >> arr;
        VAILDSTR(buf.iconpath);
        buf.icon = Utilities::trimIconFromFile(buf.iconpath);
        if (buf.icon.isNull())
          buf.iconpath.clear();
        emit setToolWinInfo(i, buf);
      }
    }

    emit setToolFinished();

    // 下面读取 WinTool 相关信息
    stream >> len; // 先读一下有几个
    for (auto i = 0; i < len; i++) {
      ToolStructInfo buf;
      buf.enabled = true; // 修复信息
      // 对于 WinTool 来说， enabled 就没用了
      // 只存储相关基础信息就可以了
      stream >> buf.isPlugin;
      if (buf.isPlugin) {
        QByteArray arr;
        stream >> arr;
        VAILDSTR(buf.serviceName);
        stream >> arr;
        VAILDSTR(buf.iconpath);
        buf.icon = Utilities::trimIconFromFile(buf.iconpath);
        stream >> arr;
        VAILDSTR(buf.provider);
        stream >> arr;
        VAILDSTR(buf.params);
        stream >> arr;
        VAILDSTR(buf.fakename);

        auto pi = plgsys->pluginIndexByProvider(buf.provider);
        // 找不到了，插件丢失或者不兼容
        if (pi < 0)
          continue;
        // 检查兼容性，看看有没有对应的服务名
        auto plg = plgsys->plugin(pi);
        auto sindex = plgsys->pluginServiceNames(plg).indexOf(buf.serviceName);
        if (sindex < 0) {
          continue;
        }
        buf.serviceID = sindex; // 修复填进去
        // 经历过重重检验，合格入库
        CORRECTINFO(buf);
        emit addWinToolInfo(buf);
      } else {
        QByteArray arr;
        stream >> arr;
        VAILDSTR(buf.process);
        stream >> arr;
        VAILDSTR(buf.params);
        stream >> arr;
        VAILDSTR(buf.fakename);
        stream >> arr;
        VAILDSTR(buf.iconpath);
        buf.icon = Utilities::trimIconFromFile(buf.iconpath);
        if (buf.icon.isNull())
          buf.iconpath.clear();
        emit addWinToolInfo(buf);
      }
    }
  } else {
    // 如果没有，就加载默认配置
    emit loadedGeneral();
  }
  f.close();
  loaded = true;
  return true;
}

bool SettingManager::saveSettings() {
  if (!ismod && QFile::exists(configfile))
    return true;
  auto res = exportSettings(configfile);
  if (res)
    ismod = false;
  return res;
}

bool SettingManager::exportSettings(QString filename) {
  QFile f(filename);
  QDataStream stream(&f);
  if (f.open(QFile::WriteOnly)) {
    // 写文件头
    static char header[] = {'W', 'I', 'N', 'G', 'T', 'O', 'O', 'L', CONFIGVER};
    stream.writeRawData(header, sizeof(header));
    // General
    stream << m_toolwin << m_wintool << m_toolGridSize << m_runWin << m_toolBox
           << m_toolwinMod << m_toolMouse;
    // 有些配置直接保存到 CenterWindow 里面了，为了减少内存占用
    emit sigSaveConfig(stream);
    // 至此，保存完毕
    f.close();
    return true;
  }
  return false;
}

void SettingManager::resetSettings() {
  emit sigReset();
  m_toolwin = true;
  m_wintool = true;
  m_toolGridSize = TOOLGRIDSIZE;

  m_toolBox = QKeySequence(Qt::KeyboardModifier::MetaModifier | Qt::Key_Space);
  m_runWin = QKeySequence(Qt::KeyboardModifier::MetaModifier | Qt::Key_R);
  m_toolwinMod = Qt::KeyboardModifier::ControlModifier;
  m_toolMouse = Qt::MouseButton::MidButton;
  ismod = true;

  emit sigToolwinEnabledChanged(m_toolwin);
  emit sigWintoolEnabledChanged(m_wintool);
  emit sigToolGridSizeChanged(m_toolGridSize);
  emit sigToolBoxHotkeyChanged(m_toolBox);
  emit sigToolwinModChanged(m_toolwinMod);
  emit sigToolwinMouseBtnChanged(m_toolMouse);
  emit sigRunWinHotkeyChanged(m_runWin);
}

QString SettingManager::backupOrignSetting() {
  auto config =
      QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) +
      "/wingTool_" + QUuid::createUuid().toString() + ".bak";
  QFile::copy(configfile, config);
  return config;
}

void SettingManager::saveFileDialogCurrent(QString path) {
  QSettings settings(QApplication::organizationName(),
                     QApplication::applicationName());
  settings.setValue("curpath", path);
}

QString SettingManager::loadFileDialogCurrent() {
  QSettings settings(QApplication::organizationName(),
                     QApplication::applicationName());
  return settings.value("curpath").toString();
}

void SettingManager::setModified() { ismod = true; }

int SettingManager::toolGridSize() const { return m_toolGridSize; }

void SettingManager::setToolGridSize(const int v) {
  if (v > 0) {
    m_toolGridSize = v;
    ismod = true;
    emit sigToolGridSizeChanged(v);
  }
}

QKeySequence SettingManager::toolBoxHotkey() const { return m_toolBox; }

void SettingManager::setToolBoxHotkey(const QKeySequence seq) {
  m_toolBox = seq;
  ismod = true;
  emit sigToolBoxHotkeyChanged(seq);
}

Qt::KeyboardModifier SettingManager::toolwinMod() const { return m_toolwinMod; }

void SettingManager::setToolwinMod(const Qt::KeyboardModifier &toolwinMod) {
  m_toolwinMod = toolwinMod;
  ismod = true;
  emit sigToolwinModChanged(toolwinMod);
}

Qt::MouseButton SettingManager::toolwinMouseBtn() const { return m_toolMouse; }

void SettingManager::setToolMouseBtn(const Qt::MouseButton &toolMouse) {
  m_toolMouse = toolMouse;
  ismod = true;
  emit sigToolwinMouseBtnChanged(toolMouse);
}

bool SettingManager::toolwinEnabled() const { return m_toolwin; }

void SettingManager::setToolwinEnabled(bool toolwin) {
  m_toolwin = toolwin;
  ismod = true;
  emit sigToolwinEnabledChanged(toolwin);
}

bool SettingManager::wintoolEnabled() const { return m_wintool; }

void SettingManager::setWintoolEnabled(bool wintool) {
  m_wintool = wintool;
  ismod = true;
  emit sigWintoolEnabledChanged(wintool);
}

QKeySequence SettingManager::runWinHotkey() const { return m_runWin; }

void SettingManager::setRunWinHotkey(const QKeySequence &runWin) {
  m_runWin = runWin;
  ismod = true;
  emit sigRunWinHotkeyChanged(runWin);
}
