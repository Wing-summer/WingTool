#include "toolwindow.h"
#include "class/settingmanager.h"
#include "utilities.h"
#include <QDebug>
#include <QMouseEvent>
#include <QToolTip>

ToolWindow::ToolWindow(DDialog *parent) : DDialog(parent) {

  setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint |
                 Qt::X11BypassWindowManagerHint);
  setWindowFlag(Qt::FramelessWindowHint);

  auto gridsize = 40; // 默认 40 先
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
    lbl->setEnabled(false);
    auto in = std::div(i, 3);
    mlayout->addWidget(lbl, in.quot, in.rem, Qt::AlignCenter);
    lbls[i] = lbl;
    connect(lbl, &DIconButton::pressed, this,
            [=] { QToolTip::showText(QCursor::pos(), lbl->toolTip()); });
  }

  lbls[4]->setIcon(ICONRES("close"));
  lbls[4]->setEnabled(true);
}

void ToolWindow::loadSettings() {
  auto sm = SettingManager::instance();

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

void ToolWindow::setIcon(int index, QIcon icon, QString tip) {
  // index 取值 0-8 ，但是索引 4 被保留不做处理，是正中间的按钮
  if (index < 0 || index >= 8)
    return;

  auto lbl = lbls[index];
  lbl->setIcon(icon);
  lbl->setEnabled(!icon.isNull());
  lbl->setToolTip(tip);
}

void ToolWindow::popup(QPoint pos) {
  this->move(pos.x() - gridtotal / 2, pos.y() - gridtotal / 2);
  show();
  raise();
}

void ToolWindow::sendMousePosUpdated() {
  bool found = false;
  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      auto pos = QCursor::pos();
      auto res = mlayout->cellRect(x, y).contains(mapFromGlobal(pos));
      auto lbl = lbls[x * 3 + y];
      if (res) {
        m_sel = QPoint(x, y);
        lbl->pressed();
        found = true;
      }
      lbl->setDown(res);
    }
  }
  if (!found) {
    m_sel = QPoint(1, 1);
  }
}

void ToolWindow::finished() {
  auto res = m_sel.x() * 3 + m_sel.y();
  if (res != 4)
    emit triggered(res);
  hide();
}
