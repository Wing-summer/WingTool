#ifndef RUNDIALOG_H
#define RUNDIALOG_H

#include "control/pluginselector.h"
#include "plugin/pluginsystem.h"
#include <DComboBox>
#include <DDialog>
#include <DLabel>
#include <DLineEdit>
#include <QObject>

DWIDGET_USE_NAMESPACE

class RunDialog : public DDialog {
  Q_OBJECT
public:
  RunDialog(DDialog *parent = nullptr);

private:
  void on_accept();
  void on_reject();

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  PluginSystem *plgsys;

  PluginSelector *ps;
  DLineEdit *dledit;

  DComboBox *cbService;
};

#endif // RUNDIALOG_H
