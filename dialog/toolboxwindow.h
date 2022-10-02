#ifndef TOOLBOXWINDOW_H
#define TOOLBOXWINDOW_H

#include "utilities.h"
#include <DDialog>
#include <DGuiApplicationHelper>
#include <DListWidget>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class ToolBoxWindow : public DDialog {
  Q_OBJECT
public:
  explicit ToolBoxWindow(DMainWindow *parent = nullptr);

public:
  void addItem(ToolStructInfo &info, int index = -1); // -1 表示追加
  void rmItem(int index);                             // -1 表示清空
  void mvItem(int from, int to);

private:
  void setTheme(DGuiApplicationHelper::ColorType theme);

protected:
  void leaveEvent(QEvent *e) override;
  void focusOutEvent(QFocusEvent *event) override;

signals:
  void sigRun(int index);

private:
  DListWidget *lstool;
  int itemheight;
};

#endif // TOOLBOXWINDOW_H
