#ifndef SHORTCUTEDITDIALOG_H
#define SHORTCUTEDITDIALOG_H

#include "utilies.h"

#include "class/appmanager.h"
#include <DCheckBox>
#include <DDialog>
#include <DFileChooserEdit>
#include <DKeySequenceEdit>
#include <DLineEdit>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class ShortCutEditDialog : public DDialog {
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
  DCheckBox *cb;
  DFileChooserEdit *fcedit;
  DLineEdit *dledit;
  DKeySequenceEdit *ksedit;
};

#endif // SHORTCUTEDITDIALOG_H
