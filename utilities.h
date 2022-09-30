#ifndef UTILIES_H
#define UTILIES_H

#include "plugin/iwingtoolplg.h"
#include <QIcon>
#include <QKeySequence>
#include <QMimeDatabase>
#include <QString>
#include <QtDBus>

#define ProgramIcon QIcon(":/images/logo.svg")
#define ICONRES(name) QIcon(":/images/" name ".png")
#define ICONRES2(name) QIcon(":/images/" name ".svg")

// 该结构体在不同使用场合下，每个成员含义可能有所变化
// 也并不是所有的成员在同一个场合用到的
struct ToolStructInfo {
  bool enabled = false;
  QKeySequence seq = QKeySequence();
  QString process = QString(); // 如果是文件是路径，如果是插件是插件名
  QString params = QString();

  // 以下仅供插件使用
  int serviceID = -1;
  int pluginIndex = -1;
  QString provider = QString();

  bool isPlugin = false;
};

Q_DECLARE_METATYPE(ToolStructInfo)

class Utilities {
public:
  static bool activeWindowFromDock(quintptr winId) {
    bool bRet = true;
    // new interface use application as id
    QDBusInterface dockDbusInterface("com.deepin.dde.daemon.Dock",
                                     "/com/deepin/dde/daemon/Dock",
                                     "com.deepin.dde.daemon.Dock");
    QDBusReply<void> reply = dockDbusInterface.call("ActivateWindow", winId);
    if (!reply.isValid()) {
      qDebug() << "call com.deepin.dde.daemon.Dock failed" << reply.error();
      bRet = false;
    }
    return bRet;
  }

  static QIcon processPluginIcon(IWingToolPlg *plg) {
    if (plg->pluginIcon().isNull()) {
      return ICONRES("plugin");
    }
    return plg->pluginIcon();
  }

  // 虽然 provider 是插件的唯一标识，需要进行校验
  // 但是，检查兼容性的时候第一关就是看看有没有它
  // 所以：没必要！
  static QByteArray getPUID(IWingToolPlg *plg) {
    if (!plg)
      return QByteArray();
    // 这个 PUID 似乎不那么高大上，仅仅就是拼凑
    // 但这几个是影响插件兼容的必要因素

    QByteArray buffer;
    QBuffer b(&buffer);
    QDataStream f(&b);

    f << plg->isTool() << plg->pluginServices();
    return buffer;
  }

  static bool isPluginCompatible(IWingToolPlg *plg, QByteArray &old) {
    if (!plg)
      return false;

    QBuffer b(&old);
    QDataStream f(&b);

    bool isTool;
    f >> isTool;
    if (isTool != plg->isTool()) // 以前你是工具项目之一，后面不是了，肯定不兼容
      return false;

    QStringList services;
    f >> services;

    auto srv = plg->pluginServices();
    auto len = services.count();

    // 服务比原来的都少了，肯定不兼容
    if (srv.count() < len)
      return false;

    // 开始评判函数，函数名不一致会导致错误的函数调用
    for (auto i = 0; i < len; i++) {
      if (srv[i] != services[i])
        return false;
    }

    return true;
  }

  static QIcon trimIconFromInfo(IWingToolPlg *plg, ToolStructInfo &info) {
    if (info.isPlugin) {
      if (plg == nullptr)
        return QIcon();
      return plg->pluginIcon();
    } else {
      QMimeDatabase db;
      auto t = db.mimeTypeForFile(info.process);
      return QIcon::fromTheme(t.iconName(), QIcon(t.iconName()));
    }
  }
};

#endif // UTILIES_H
