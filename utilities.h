#ifndef UTILIES_H
#define UTILIES_H

#include "plugin/iwingtoolplg.h"
#include <QIcon>
#include <QKeySequence>
#include <QString>

#define ProgramIcon QIcon(":/images/logo.svg")
#define ICONRES(name) QIcon(":/images/" name ".png")
#define ICONRES2(name) QIcon(":/images/" name ".svg")

// 该结构体在不同使用场合下，含义可能有所变化
// 也并不是所有的成员在同一个场合用到的
struct ToolStructInfo {
  bool enabled = true;
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
};

#endif // UTILIES_H
