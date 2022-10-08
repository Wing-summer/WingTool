#ifndef PLUGINSELDIALOG_H
#define PLUGINSELDIALOG_H

#include "plugin/pluginsystem.h"
#include <DDialog>
#include <DListWidget>
#include <DTextBrowser>

DWIDGET_USE_NAMESPACE

class PluginSelDialog : public DDialog {
  Q_OBJECT
public:
  PluginSelDialog(DDialog *parent = nullptr);

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  DListWidget *lsplgs;
  DTextBrowser *tbplginfo;
  PluginSystem *plgsys;
};

#endif // PLUGINSELDIALOG_H
