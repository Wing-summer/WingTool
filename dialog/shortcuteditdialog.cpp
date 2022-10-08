#include "shortcuteditdialog.h"
#include <DDialogButtonBox>
#include <DLabel>
#include <DMessageManager>
#include <QShortcut>

ShortCutEditDialog::ShortCutEditDialog(ToolStructInfo res, DMainWindow *parent)
    : DDialog(parent), manager(AppManager::instance()),
      plgsys(PluginSystem::instance()) {

  // 处于编辑状态直接堵塞所有相应（屏蔽鼠标追踪和热键触发以防干扰）
  manager->blockSignals(true);

  setWindowTitle(tr("HotkeyEdit"));

  cb = new DCheckBox(tr("Enabled"), this);
  if (res.seq.isEmpty()) {
    cb->setChecked(true);
  } else {
    cb->setChecked(res.enabled);
  }

  addContent(cb);
  addSpacing(10);

  addContent(new DLabel(tr("ShortCut"), this));
  addSpacing(5);
  ksedit = new DKeySequenceEdit(this);
  ksedit->setKeySequence(res.seq);
  addContent(ksedit);
  addSpacing(10);

  addContent(new DLabel(tr("Plugin"), this));
  addSpacing(5);
  ps = new PluginSelector(this);
  connect(ps, &PluginSelector::finished, this, [=] {
    auto plg = ps->getSelectedPlg();
    if (plg) {
      lblp->setText(tr("Service"));
      fcedit->setVisible(false);
      cbService->clear();
      cbService->addItems(plgsys->pluginServicetrNames(plg));
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
    cbService->addItems(plgsys->pluginServicetrNames(ps->getSelectedPlg()));
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
  connect(dbbox, &DDialogButtonBox::accepted, this,
          &ShortCutEditDialog::on_accept);
  connect(dbbox, &DDialogButtonBox::rejected, this,
          &ShortCutEditDialog::on_reject);
  auto key = QKeySequence(Qt::Key_Return);
  auto s = new QShortcut(key, this);
  connect(s, &QShortcut::activated, this, &ShortCutEditDialog::on_accept);
  addContent(dbbox);
}

ToolStructInfo ShortCutEditDialog::getResult() { return res; }

void ShortCutEditDialog::on_accept() {
  res.enabled = cb->isChecked();
  res.seq = ksedit->keySequence();

  if (res.seq == QKeySequence()) {
    DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                             tr("NoHotkeySet"));
    return;
  }

  res.isPlugin = ps->getSelectedIndex() >= 0;

  if (res.isPlugin) {
    auto sel = ps->getSelectedPlg();
    res.process = sel->pluginName();
    res.serviceID = cbService->currentIndex();
    res.provider = plgsys->pluginProvider(sel);
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

void ShortCutEditDialog::on_reject() {
  manager->blockSignals(false); // 恢复能力
  done(0);
}

void ShortCutEditDialog::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event);
  manager->blockSignals(false); // 恢复能力
  done(0);
}
