#ifndef TOOLBOXWINDOW_H
#define TOOLBOXWINDOW_H

#include <DDialog>
#include <DListWidget>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class ToolBoxWindow : public DDialog {
  Q_OBJECT
public:
  ToolBoxWindow(DMainWindow *parent = nullptr);

private:
  DListWidget *lstool;
};

#endif // TOOLBOXWINDOW_H
