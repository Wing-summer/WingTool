#ifndef TOOLSWAPDIALOG_H
#define TOOLSWAPDIALOG_H

#include "utilities.h"
#include <DDialog>
#include <DIconButton>
#include <QGridLayout>

DWIDGET_USE_NAMESPACE

class ToolSwapDialog : public DDialog {
  Q_OBJECT
public:
  ToolSwapDialog(const QVector<QIcon> &icons, int selindex,
                 DDialog *parent = nullptr);

private:
  void on_accept();
  void on_reject();

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  QGridLayout *mlayout;
  DIconButton *lbls[9] = {nullptr};

  int cur = -1;
};

#endif // TOOLSWAPDIALOG_H
