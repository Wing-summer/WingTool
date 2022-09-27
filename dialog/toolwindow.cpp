#include "toolwindow.h"
#include "class/settingmanager.h"
#include "utilities.h"
#include <QMouseEvent>

ToolWindow::ToolWindow(DDialog *parent) : DDialog(parent) {

  setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint |
                 Qt::X11BypassWindowManagerHint);
  setWindowFlag(Qt::FramelessWindowHint);

  auto sm = SettingManager::instance();
  auto gridsize = sm->toolGridSize();
  gridtotal = gridsize * 3;
  setFixedSize(gridtotal, gridtotal);
  delete layout();

  mlayout = new QGridLayout(this);
  setLayout(mlayout);
  mlayout->setMargin(1);

  for (int i = 0; i < 9; i++) {
    auto lbl = new DIconButton(this);
    lbl->setFixedSize(gridsize - 2, gridsize - 2);
    lbl->setIconSize(QSize(gridsize / 2, gridsize / 2));
    auto in = std::div(i, 3);
    mlayout->addWidget(lbl, in.quot, in.rem, Qt::AlignCenter);
    lbls[i] = lbl;
  }

  lbls[4]->setIcon(ICONRES("close"));

  // 当设置修改时，响应调整大小
  connect(sm, &SettingManager::sigToolGridSizeChanged, this, [=](int v) {
    gridtotal = v * 3;
    this->setFixedSize(gridtotal, gridtotal);
    for (int i = 0; i < 9; i++) {
      lbls[i]->setFixedSize(v - 2, v - 2);
      lbls[i]->setIconSize(QSize(v / 2, v / 2));
    }
  });
}

void ToolWindow::setIcons(QVector<QIcon> icons) {
  for (int i = 0; i < icons.count(); i++) {
    if (i < 4) {
      lbls[i]->setIcon(icons[i]);
    } else {
      lbls[i + 1]->setIcon(icons[i]);
    }
  }
}

void ToolWindow::setIcon(int index, QIcon icon) {
  // index 取值 0-8 ，但是索引 4 被保留不做处理，是正中间的按钮，需要进行处理
  if (index < 0 || index >= 8)
    return;
  if (index >= 4)
    index++;
  lbls[index]->setIcon(icon);
}

void ToolWindow::popup(QPoint pos) {
  this->move(pos.x() - gridtotal / 2, pos.y() - gridtotal / 2);
  show();
}

void ToolWindow::sendMousePosUpdated() {
  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      auto res =
          mlayout->cellRect(x, y).contains(mapFromGlobal(QCursor::pos()));
      if (res) {
        m_sel = QPoint(x, y);
      }
      lbls[x * 3 + y]->setDown(res);
    }
  }
}

void ToolWindow::finished() {
  auto res = m_sel.x() * 3 + m_sel.y();
  if (res >= 4)
    res--;
  emit triggered(res);
}
