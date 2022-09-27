#ifndef UTILIES_H
#define UTILIES_H

#include "plugin/iwingtoolplg.h"
#include <QIcon>
#include <QKeySequence>
#include <QString>

#define ProgramIcon QIcon(":/images/logo.svg")
#define ICONRES(name) QIcon(":/images/" name ".png")

struct ShortCutEditRes {
  bool enabled;
  QKeySequence seq;
  QString process;
  int serviceID;
  QString provider;
  QString params;
  bool isPlugin;
};

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
