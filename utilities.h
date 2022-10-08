#ifndef UTILIES_H
#define UTILIES_H

#include "plugin/iwingtoolplg.h"
#include <QComboBox>
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

    b.open(QBuffer::WriteOnly);
    f << plg->isTool() << plg->serviceMeta();
    b.close();
    return buffer;
  }

  static bool isPluginCompatible(IWingToolPlg *plg, const QStringList &newsrvs,
                                 QByteArray &old) {
    if (!plg)
      return false;

    QBuffer b(&old);
    QDataStream f(&b);
    b.open(QBuffer::ReadOnly);
    bool isTool;
    f >> isTool;

    // 以前你是工具项目之一，后面不是了，肯定不兼容
    if (isTool != plg->isTool()) {
      b.close();
      return false;
    }

    QStringList services;
    f >> services;

    auto len = services.count();

    // 服务比原来的都少了，肯定不兼容
    if (newsrvs.count() < len) {
      b.close();
      return false;
    }

    // 开始评判函数，函数名不一致会导致错误的函数调用
    for (auto i = 0; i < len; i++) {
      if (newsrvs[i] != services[i]) {
        b.close();
        return false;
      }
    }
    b.close();
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

  static QString getProgramName(const QStringList &services,
                                ToolStructInfo &info) {
    return info.isPlugin ? info.process + " | " + services[info.serviceID]
                         : QFileInfo(info.process).fileName();
  }

  static QString getToolTipContent(const QStringList &services,
                                   ToolStructInfo &info) {
    if (info.isPlugin) {
      return QObject::tr("Process:%1\nService:%2\nParams:%3")
          .arg(info.process)
          .arg(services[info.serviceID])
          .arg(info.params);
    } else {
      return QObject::tr("Process:%1\nParams:%2")
          .arg(info.process)
          .arg(info.params);
    }
  }

  static QStringList parseCmdParams(QString str) {
    static QRegularExpression regex("(\"[^\"]+\"|[^\\s\"]+)");
    QStringList args;
    int off = 0;
    while (true) {
      auto match = regex.match(str, off);
      if (!match.hasMatch()) {
        break;
      }
      auto res = match.captured();
      if (res[0] == '\"')
        res = res.replace("\"", "");
      if (res[0] == '\'')
        res = res.replace("'", "");
      args << res;
      off = match.capturedEnd();
    }
    return args;
  }
};

#endif // UTILIES_H
