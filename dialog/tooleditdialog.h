#ifndef TOOLWINEDITDIALOG_H
#define TOOLWINEDITDIALOG_H

#include "utilities.h"

#include "class/appmanager.h"
#include "class/settingmanager.h"
#include "control/pluginselector.h"
#include <DCheckBox>
#include <DComboBox>
#include <DDialog>
#include <DFileChooserEdit>
#include <DIconButton>
#include <DKeySequenceEdit>
#include <DLabel>
#include <DLineEdit>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class ToolEditDialog : public DDialog {
  Q_OBJECT
public:
  ToolEditDialog(ToolStructInfo res = ToolStructInfo(),
                 DMainWindow *parent = nullptr);
  ToolStructInfo getResult(); // 这里的 enabled 和 seq 保留不使用

private:
  void on_accept();
  void on_reject();

  void refreshIcon();

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  AppManager *manager;
  PluginSystem *plgsys;
  SettingManager *sm;
  ToolStructInfo res;

  PluginSelector *ps;
  DFileChooserEdit *fcedit;
  DFileChooserEdit *fcicon;
  DLineEdit *dledit, *dlfkname;

  DLabel *lblp;
  DComboBox *cbService;
  DIconButton *iconpre;

  QString lastusedpath;
  QIcon ficon, sicon;
};

#endif // TOOLWINEDITDIALOG_H
