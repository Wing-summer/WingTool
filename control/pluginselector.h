#ifndef PLUGINSELECTOR_H
#define PLUGINSELECTOR_H

#include "dialog/pluginseldialog.h"
#include "plugin/pluginsystem.h"
#include <DPushButton>

DWIDGET_USE_NAMESPACE

class PluginSelector : public DPushButton {
public:
  explicit PluginSelector(QWidget *parent = nullptr);

public slots:
  void selectPlugin();

public:
  int getSelectedIndex();
  IWingToolPlg *getSelectedPlg();

private:
  PluginSystem *plgsys;

  int selplgindex = -1;
};

#endif // PLUGINSELECTOR_H
