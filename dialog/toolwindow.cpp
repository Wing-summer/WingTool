#include "toolwindow.h"
#include "utilies.h"
#include <QMouseEvent>

#define GridSize 40
#define GridTotal (GridSize * 3)

ToolWindow::ToolWindow(DDialog *parent) : DDialog(parent) {

  setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint |
                 Qt::X11BypassWindowManagerHint);
  setWindowFlag(Qt::FramelessWindowHint);
  setFixedSize(GridTotal, GridTotal);
  delete layout();

  mlayout = new QGridLayout(this);
  setLayout(mlayout);
  mlayout->setMargin(1);

  for (int i = 0; i < 9; i++) {
    auto lbl = new DIconButton(this);
    lbl->setFixedSize(GridSize - 2, GridSize - 2);
    lbl->setIconSize(QSize(GridSize / 2, GridSize / 2));
    auto in = std::div(i, 3);
    mlayout->addWidget(lbl, in.quot, in.rem, Qt::AlignCenter);
    lbls[i] = lbl;
  }

  lbls[4]->setIcon(ICONRES("close"));
}

void ToolWindow::setIcons(QVector<QIcon> icons) {
  if (icons.count() != 8)
    return;
  for (int i = 0; i < 9; i++) {
    if (i == 4)
      continue;
    if (i < 4) {
      lbls[i]->setIcon(icons[i]);
    } else {
      lbls[i]->setIcon(icons[i - 1]);
    }
  }
}

void ToolWindow::setIcon(int index, QIcon icon) {
  // index 取值 0-8 ，但是索引 4 被保留不做处理，是正中间的按钮
  if (index < 0 || index == 4 || index > 8)
    return;
  lbls[index]->setIcon(icon);
}

void ToolWindow::popup(QPoint pos) {
  this->move(pos.x() - GridTotal / 2, pos.y() - GridTotal / 2);
  show();
  setFocus();
}

void ToolWindow::sendMousePosUpdated() {
  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      lbls[x * 3 + y]->setDown(
          mlayout->cellRect(x, y).contains(mapFromGlobal(QCursor::pos())));
    }
  }
}
