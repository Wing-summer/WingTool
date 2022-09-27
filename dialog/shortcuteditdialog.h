#ifndef SHORTCUTEDITDIALOG_H
#define SHORTCUTEDITDIALOG_H

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

class ShortCutEditDialog : public DDialog {
  Q_OBJECT
public:
  ShortCutEditDialog(bool enabled = true, QKeySequence seq = QKeySequence(),
                     QString process = QString(), QString params = QString(),
                     DMainWindow *parent = nullptr);
  ShortCutEditRes getResult();

private:
  void on_accept();
  void on_reject();

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  AppManager *manager;
  ShortCutEditRes res;

  PluginSelector *ps;
  DCheckBox *cb;
  DFileChooserEdit *fcedit;
  DLineEdit *dledit;
  DKeySequenceEdit *ksedit;

  DLabel *lblp;
  DComboBox *cbService;
};

#endif // SHORTCUTEDITDIALOG_H
