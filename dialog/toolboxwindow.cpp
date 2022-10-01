#include "toolboxwindow.h"
#include "control/wintoolitem.h"
#include <DLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

ToolBoxWindow::ToolBoxWindow(DMainWindow *parent) : DDialog(parent) {
  lstool = new DListWidget(this);
  addContent(lstool);
}

void ToolBoxWindow::addItem(ToolStructInfo &info, int index) {
  auto pitem = new QListWidgetItem;

  auto item = new WinToolItem(this);
}

void ToolBoxWindow::rmItem(int index) {
  if (index < 0) {
    lstool->clear();
    return;
  }

  if (index >= lstool->count()) {
    return;
  }

  auto k = lstool->takeItem(index);
  delete k;
}

void ToolBoxWindow::mvItem(int from, int to) {
  auto len = lstool->count();
  if (from < 0 || from >= len || to < 0 || to >= len)
    return;
}
