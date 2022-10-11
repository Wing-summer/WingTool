#ifndef SHORTCUTEDITDIALOG_H
#define SHORTCUTEDITDIALOG_H

#include "utilities.h"

#include "class/appmanager.h"
#include "class/settingmanager.h"
#include "control/pluginselector.h"
#include "plugin/pluginsystem.h"
#include <DCheckBox>
#include <DComboBox>
#include <DDialog>
#include <DFileChooserEdit>
#include <DKeySequenceEdit>
#include <DLabel>
#include <DLineEdit>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class ShortCutEditDialog : public DDialog {
  Q_OBJECT
public:
  ShortCutEditDialog(ToolStructInfo res = ToolStructInfo(),
                     DMainWindow *parent = nullptr);
  ToolStructInfo getResult();

private:
  void on_accept();
  void on_reject();

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  AppManager *manager;
  PluginSystem *plgsys;
  SettingManager *sm;

  ToolStructInfo res;

  PluginSelector *ps;
  DCheckBox *cb;
  DFileChooserEdit *fcedit;
  DLineEdit *dledit;
  DKeySequenceEdit *ksedit;
  DLineEdit *dlfkname;

  DLabel *lblp;
  DComboBox *cbService;

  QString lastusedpath;
};

#endif // SHORTCUTEDITDIALOG_H
