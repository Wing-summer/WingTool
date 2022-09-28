#ifndef TOOLWINEDITDIALOG_H
#define TOOLWINEDITDIALOG_H

#include "utilities.h"

#include "class/appmanager.h"
#include "control/pluginselector.h"
#include <DCheckBox>
#include <DComboBox>
#include <DDialog>
#include <DFileChooserEdit>
#include <DKeySequenceEdit>
#include <DLabel>
#include <DLineEdit>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class ToolEditDialog : public DDialog {
public:
  ToolEditDialog(ToolStructInfo res = ToolStructInfo(),
                    DMainWindow *parent = nullptr);
  ToolStructInfo getResult(); // 这里的 enabled 和 seq 保留不使用

private:
  void on_accept();
  void on_reject();

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  AppManager *manager;
  ToolStructInfo res;

  PluginSelector *ps;
  DFileChooserEdit *fcedit;
  DLineEdit *dledit;

  DLabel *lblp;
  DComboBox *cbService;
};

#endif // TOOLWINEDITDIALOG_H
