#ifndef PLUGINSELECTOR_H
#define PLUGINSELECTOR_H

#include "dialog/pluginseldialog.h"
#include "plugin/pluginsystem.h"
#include <DPushButton>

DWIDGET_USE_NAMESPACE

class PluginSelector : public DPushButton {
  Q_OBJECT
public:
  explicit PluginSelector(QWidget *parent = nullptr);

public slots:
  void selectPlugin();

public:
  int getSelectedIndex();
  IWingToolPlg *getSelectedPlg();

  bool setSelectedIndex(int index);

signals:
  void finished();

private:
  PluginSystem *plgsys;

  int selplgindex = -1;
};

#endif // PLUGINSELECTOR_H
