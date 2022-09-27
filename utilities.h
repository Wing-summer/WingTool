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
  QString params;
};

class Utilities {
public:
  static QIcon processPluginIcon(IWingToolPlg *plg) {
    if (plg->pluginIcon().availableSizes().count()) {
      return plg->pluginIcon();
    }
    return ICONRES("plugin");
  }
};

#endif // UTILIES_H
