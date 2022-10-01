#include "toolswapdialog.h"
#include "class/settingmanager.h"
#include <DDialogButtonBox>
#include <DLabel>
#include <DMessageManager>
#include <QButtonGroup>
#include <QShortcut>

ToolSwapDialog::ToolSwapDialog(const QVector<QIcon> &icons, int selindex,
                               DDialog *parent)
    : DDialog(parent) {
  setWindowTitle(tr("ToolSwapDialog"));

  addContent(new DLabel(tr("PleaseSelect"), this), Qt::AlignCenter);
  addSpacing(10);
  auto gridsize = SettingManager::instance()->toolGridSize();
  auto gw = new QWidget(this);
  gw->setFixedSize(gridsize * 3, gridsize * 3);
  delete gw->layout();
  auto mlayout = new QGridLayout;
  gw->setLayout(mlayout);
  mlayout->setMargin(1);
  auto btngs = new QButtonGroup(this);
  btngs->setExclusive(true); // 设置按钮选中互斥
  for (int i = 0; i < 9; i++) {
    auto lbl = new DIconButton(this);
    lbl->setFixedSize(gridsize - 2, gridsize - 2);
    lbl->setIconSize(QSize(gridsize / 2, gridsize / 2));
    lbl->setIcon(icons[i]);
    lbl->setCheckable(true);
    auto in = std::div(i, 3);
    mlayout->addWidget(lbl, in.quot, in.rem, Qt::AlignCenter);
    lbls[i] = lbl;
    btngs->addButton(lbl);
    connect(lbl, &DIconButton::toggled, this, [=](bool b) {
      if (b)
        cur = i;
    });
  }
  auto lbl4 = lbls[4];
  lbl4->setIcon(ICONRES("close"));
  lbl4->setEnabled(false);
  lbls[selindex]->setEnabled(false);
  addContent(gw, Qt::AlignCenter);
  addSpacing(20);
  auto dbbox = new DDialogButtonBox(
      DDialogButtonBox::Ok | DDialogButtonBox::Cancel, this);
  connect(dbbox, &DDialogButtonBox::accepted, this, &ToolSwapDialog::on_accept);
  connect(dbbox, &DDialogButtonBox::rejected, this, &ToolSwapDialog::on_reject);
  auto key = QKeySequence(Qt::Key_Return);
  auto s = new QShortcut(key, this);
  connect(s, &QShortcut::activated, this, &ToolSwapDialog::on_accept);
  addContent(dbbox);
}

void ToolSwapDialog::on_accept() {
  if (cur == -1) {
    DMessageManager::instance()->sendMessage(this, ProgramIcon, tr("NoSelect"));
    return;
  }
  done(cur);
}

void ToolSwapDialog::on_reject() { done(-1); }

void ToolSwapDialog::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event);
  done(-1);
}
