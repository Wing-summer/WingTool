#include "tooleditdialog.h"
#include <DDialogButtonBox>
#include <DMessageManager>
#include <QShortcut>

ToolEditDialog::ToolEditDialog(ToolStructInfo res, DMainWindow *parent)
    : DDialog(parent), manager(AppManager::instance()) {

  // 处于编辑状态直接堵塞所有相应（屏蔽鼠标追踪和热键触发以防干扰）
  manager->blockSignals(true);

  setWindowTitle(tr("ToolWinEdit"));

  res.enabled = true;
  addContent(new DLabel(tr("Plugin"), this));
  addSpacing(5);
  ps = new PluginSelector(this);
  connect(ps, &PluginSelector::finished, this, [=] {
    auto plg = ps->getSelectedPlg();
    if (plg) {
      lblp->setText(tr("Service"));
      fcedit->setVisible(false);
      cbService->clear();
      cbService->addItems(plg->pluginServices());
      cbService->setVisible(true);
    } else {
      lblp->setText(tr("FilePath"));
      fcedit->setVisible(true);
      cbService->setVisible(false);
    }
  });
  ps->setSelectedIndex(res.pluginIndex);
  addContent(ps);
  addSpacing(10);

  lblp = new DLabel(res.isPlugin ? tr("Service") : tr("FilePath"), this);
  addContent(lblp);
  addSpacing(5);
  fcedit = new DFileChooserEdit(this);
  fcedit->initDialog();
  fcedit->setText(res.process);
  fcedit->setVisible(!res.isPlugin);
  addContent(fcedit);

  cbService = new DComboBox(this);
  if (res.isPlugin) {
    cbService->addItems(ps->getSelectedPlg()->pluginServices());
    cbService->setCurrentIndex(res.serviceID);
  }
  cbService->setVisible(res.isPlugin);
  addContent(cbService);

  addSpacing(10);

  addContent(new DLabel(tr("Param"), this));
  addSpacing(5);
  dledit = new DLineEdit(this);
  dledit->setText(res.params);
  addContent(dledit);

  addSpacing(20);
  auto dbbox = new DDialogButtonBox(
      DDialogButtonBox::Ok | DDialogButtonBox::Cancel, this);
  connect(dbbox, &DDialogButtonBox::accepted, this, &ToolEditDialog::on_accept);
  connect(dbbox, &DDialogButtonBox::rejected, this, &ToolEditDialog::on_reject);
  auto key = QKeySequence(Qt::Key_Return);
  auto s = new QShortcut(key, this);
  connect(s, &QShortcut::activated, this, &ToolEditDialog::on_accept);
  addContent(dbbox);
}

ToolStructInfo ToolEditDialog::getResult() { return res; }

void ToolEditDialog::on_accept() {
  res.isPlugin = ps->getSelectedIndex() >= 0;
  res.enabled = true; // 指示该信息有效，否则会被忽略

  if (res.isPlugin) {
    auto sel = ps->getSelectedPlg();
    res.process = sel->pluginName();
    res.serviceID = cbService->currentIndex();
    res.provider = sel->provider();
    res.pluginIndex = ps->getSelectedIndex();
  } else {
    res.process = fcedit->text();
    if (res.process.isEmpty()) {
      DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                               tr("NoProcessSet"));
      return;
    }
  }

  res.params = dledit->text();

  manager->blockSignals(false); // 恢复能力
  done(1);
}

void ToolEditDialog::on_reject() {
  manager->blockSignals(false); // 恢复能力
  done(0);
}

void ToolEditDialog::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event);
  manager->blockSignals(false); // 恢复能力
  done(0);
}
