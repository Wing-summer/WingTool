#include "tooleditdialog.h"
#include <DDialogButtonBox>
#include <DMessageManager>
#include <QHBoxLayout>
#include <QShortcut>

ToolEditDialog::ToolEditDialog(ToolStructInfo res, DMainWindow *parent)
    : DDialog(parent), manager(AppManager::instance()),
      plgsys(PluginSystem::instance()), sm(SettingManager::instance()) {

  // 处于编辑状态直接堵塞所有相应（屏蔽鼠标追踪和热键触发以防干扰）
  manager->blockSignals(true);
  this->res = res;

  setWindowTitle(tr("ToolWinEdit"));

  lastusedpath = sm->loadFileDialogCurrent();

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
      cbService->addItems(plgsys->pluginServicetrNames(plg));
      cbService->setVisible(true);
    } else {
      lblp->setText(tr("FilePath"));
      fcedit->setVisible(true);
      cbService->setVisible(false);
    }
    this->refreshIcon();
  });
  ps->setSelectedIndex(res.pluginIndex);
  addContent(ps);
  addSpacing(10);

  iconpre = new DIconButton(this); // 提前创建好

  addContent(new DLabel(tr("Icon"), this));
  addSpacing(5);
  fcicon = new DFileChooserEdit(this);
  fcicon->initDialog();
  fcicon->setNameFilters({tr("Image Format (*.png *.svg *.jpg *.jpeg)")});
  fcicon->setDirectoryUrl(QUrl(lastusedpath));
  connect(fcicon, &DFileChooserEdit::textChanged, this, [=] {
    auto name = fcicon->text().trimmed();
    this->res.iconpath = name;
    if (name.isEmpty()) {
      sicon = QIcon();
      this->refreshIcon();
      return;
    }
    if (QIcon::hasThemeIcon(name)) {
      sicon = QIcon::fromTheme(name);
    } else {
      if (QFile::exists(name)) {
        QPixmap img;
        if (img.load(name)) {
          auto icon = QIcon((img.width() > 64 || img.height() > 64)
                                ? img.scaled(64, 64, Qt::KeepAspectRatio)
                                : img);
          sicon = icon;
        } else {
          sicon = QIcon();
          DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                                   tr("InvalidIcon"));
        }
      }
    }
    this->refreshIcon();
  });
  connect(fcicon, &DFileChooserEdit::fileChoosed, this,
          [=](const QString &fileName) {
            lastusedpath = QFileInfo(fileName).absoluteDir().absolutePath();
            fcicon->setDirectoryUrl(QUrl(lastusedpath));
            fcedit->setDirectoryUrl(QUrl(lastusedpath));
            emit fcicon->editingFinished();
          });
  fcicon->setText(res.iconpath);
  addContent(fcicon);
  addSpacing(10);

  lblp = new DLabel(res.isPlugin ? tr("Service") : tr("FilePath"), this);
  addContent(lblp);
  addSpacing(5);
  fcedit = new DFileChooserEdit(this);
  fcedit->initDialog();
  fcedit->setVisible(!res.isPlugin);
  fcedit->setDirectoryUrl(QUrl(lastusedpath));
  connect(fcedit, &DFileChooserEdit::textChanged, this, [=] {
    this->res.process = fcedit->text();
    ficon = Utilities::trimIconFromFile(this->res.process);
    this->refreshIcon();
  });
  connect(fcedit, &DFileChooserEdit::fileChoosed, this,
          [=](const QString &fileName) {
            lastusedpath = QFileInfo(fileName).absoluteDir().absolutePath();
            fcicon->setDirectoryUrl(QUrl(lastusedpath));
            fcedit->setDirectoryUrl(QUrl(lastusedpath));
            emit fcedit->editingFinished();
          });
  fcedit->setText(res.process);
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
  addSpacing(10);
  addContent(new DLabel(tr("FakeName"), this));
  addSpacing(5);
  dlfkname = new DLineEdit(this);
  dlfkname->setText(res.fakename);
  addContent(dlfkname);
  addSpacing(20);

  auto w = new QWidget(this);
  auto hlayout = new QHBoxLayout(w);
  iconpre->setToolTip(tr("IconPreview"));
  iconpre->setIcon(res.icon);
  iconpre->setIconSize(QSize(25, 25));
  connect(iconpre, &DIconButton::clicked, this, &ToolEditDialog::refreshIcon);
  hlayout->addWidget(iconpre, 0, Qt::AlignLeft);
  auto dbbox = new DDialogButtonBox(
      DDialogButtonBox::Ok | DDialogButtonBox::Cancel, this);
  connect(dbbox, &DDialogButtonBox::accepted, this, &ToolEditDialog::on_accept);
  connect(dbbox, &DDialogButtonBox::rejected, this, &ToolEditDialog::on_reject);
  auto key = QKeySequence(Qt::Key_Return);
  auto s = new QShortcut(key, this);
  connect(s, &QShortcut::activated, this, &ToolEditDialog::on_accept);
  hlayout->addWidget(dbbox, 0, Qt::AlignRight);
  addContent(w);

  refreshIcon();
}

ToolStructInfo ToolEditDialog::getResult() { return res; }

void ToolEditDialog::on_accept() {
  res.isPlugin = ps->getSelectedIndex() >= 0;

  res.icon = sicon.isNull() ? ficon : sicon;
  if (res.icon.isNull() && !res.isPlugin) {
    DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                             tr("NoVaildIconSet"));
    return;
  }

  res.enabled = true; // 指示该信息有效，否则会被忽略

  if (res.isPlugin) {
    auto sel = ps->getSelectedPlg();
    res.process = sel->pluginName();
    res.serviceID = cbService->currentIndex();
    res.serviceName = plgsys->pluginServiceNames(sel)[res.serviceID];
    res.provider = plgsys->pluginProvider(sel);
    res.pluginIndex = ps->getSelectedIndex();
  } else {
    res.process = fcedit->text().trimmed();
    if (res.process.isEmpty() || !QFile::exists(res.process)) {
      DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                               tr("NoVaildProcessSet"));
      return;
    }
  }

  res.fakename = dlfkname->text().trimmed();
  res.params = dledit->text().trimmed();

  manager->blockSignals(false); // 恢复能力
  done(1);
}

void ToolEditDialog::on_reject() {
  manager->blockSignals(false); // 恢复能力
  done(0);
}

void ToolEditDialog::refreshIcon() {
  if (sicon.isNull()) {
    auto plg = ps->getSelectedPlg();
    if (plg) {
      iconpre->setIcon(plg->pluginIcon());
    } else {
      if (!ficon.isNull()) {
        iconpre->setIcon(ficon);
      } else {
        iconpre->setIcon(QIcon());
      }
    }
  } else {
    iconpre->setIcon(sicon);
  }
}

void ToolEditDialog::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event);
  sm->saveFileDialogCurrent(lastusedpath);
  manager->blockSignals(false); // 恢复能力
  done(0);
}
