#include "shortcuteditdialog.h"
#include <DDialogButtonBox>
#include <DLabel>
#include <DMessageManager>
#include <QShortcut>

ShortCutEditDialog::ShortCutEditDialog(bool enabled, QKeySequence seq,
                                       QString process, QString params,
                                       DMainWindow *parent)
    : DDialog(parent), manager(AppManager::instance()) {

  // 处于编辑状态直接堵塞所有相应（屏蔽鼠标追踪和热键触发以防干扰）
  manager->blockSignals(true);

  setWindowTitle(tr("HotkeyEdit"));

  cb = new DCheckBox(tr("Enabled"), this);
  cb->setChecked(enabled);
  addContent(cb);
  addSpacing(10);

  addContent(new DLabel(tr("ShortCut"), this));
  addSpacing(5);
  ksedit = new DKeySequenceEdit(this);
  ksedit->setKeySequence(seq);
  addContent(ksedit);
  addSpacing(10);

  addContent(new DLabel(tr("FilePath"), this));
  addSpacing(5);
  fcedit = new DFileChooserEdit(this);
  fcedit->initDialog();
  fcedit->setText(process);
  addContent(fcedit);
  addSpacing(10);

  addContent(new DLabel(tr("Param"), this));
  addSpacing(5);
  dledit = new DLineEdit(this);
  dledit->setText(params);
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

ShortCutEditRes ShortCutEditDialog::getResult() { return res; }

void ShortCutEditDialog::on_accept() {
  res.enabled = cb->isChecked();
  res.seq = ksedit->keySequence();

  if (res.seq == QKeySequence()) {
    DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                             tr("NoHotkeySet"));
    return;
  }

  res.process = fcedit->text();

  if (res.process.isEmpty()) {
    DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                             tr("NoProcessSet"));
    return;
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