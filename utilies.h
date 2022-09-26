#ifndef UTILIES_H
#define UTILIES_H

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

#endif // UTILIES_H
