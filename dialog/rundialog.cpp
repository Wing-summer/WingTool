#include "rundialog.h"
#include <DDialogButtonBox>
#include <DMessageManager>
#include <QShortcut>

RunDialog::RunDialog(DDialog *parent)
    : DDialog(parent), plgsys(PluginSystem::instance()) {
  setWindowTitle(tr("RunPlugin"));
  addContent(new DLabel(tr("Plugin"), this));
  addSpacing(5);
  ps = new PluginSelector(this);
  connect(ps, &PluginSelector::finished, this, [=] {
    auto plg = ps->getSelectedPlg();
    cbService->clear();
    cbService->addItems(plgsys->pluginServicetrNames(plg));
    cbService->setVisible(true);
  });
  addContent(ps);
  addSpacing(10);

  addContent(new DLabel(tr("Service"), this));
  addSpacing(5);

  cbService = new DComboBox(this);
  cbService->addItems(plgsys->pluginServicetrNames(ps->getSelectedPlg()));
  cbService->setCurrentIndex(0);
  addContent(cbService);

  addSpacing(10);

  addContent(new DLabel(tr("Param"), this));
  addSpacing(5);
  dledit = new DLineEdit(this);
  addContent(dledit);

  addSpacing(20);
  auto dbbox = new DDialogButtonBox(
      DDialogButtonBox::Ok | DDialogButtonBox::Cancel, this);
  connect(dbbox, &DDialogButtonBox::accepted, this, &RunDialog::on_accept);
  connect(dbbox, &DDialogButtonBox::rejected, this, &RunDialog::on_reject);
  auto key = QKeySequence(Qt::Key_Return);
  auto s = new QShortcut(key, this);
  connect(s, &QShortcut::activated, this, &RunDialog::on_accept);
  addContent(dbbox);
}

void RunDialog::on_accept() {
  auto plg = ps->getSelectedPlg();
  if (plg == nullptr) {
    DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                             tr("NoProcessSet"));
    return;
  }
  QVector<QVariant> params;
  for (auto &item : Utilities::parseCmdParams(dledit->text())) {
    params.append(item);
  }
  auto res = plgsys->pluginCall(plgsys->pluginProvider(plg),
                                cbService->currentIndex() + 1, params);
  done(res);
}

void RunDialog::on_reject() { done(-1); }

void RunDialog::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event);
  done(-1);
}
