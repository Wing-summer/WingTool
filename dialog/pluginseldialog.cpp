#include "pluginseldialog.h"
#include "plugin/pluginsystem.h"
#include "utilities.h"
#include <DButtonBox>
#include <DMessageManager>
#include <QHBoxLayout>
#include <QMetaEnum>

PluginSelDialog::PluginSelDialog(DDialog *parent)
    : DDialog(parent), plgsys(PluginSystem::instance()) {
  setWindowTitle(tr("PluginSelectingDialog"));

  auto w = new QWidget(this);
  auto layout = new QHBoxLayout(w);

  lsplgs = new DListWidget(this);
  for (auto item : PluginSystem::instance()->plugins()) {
    new QListWidgetItem(Utilities::processPluginIcon(item), item->pluginName(),
                        lsplgs);
  }
  layout->addWidget(lsplgs);
  addSpacing(10);

  tbplginfo = new DTextBrowser(this);
  tbplginfo->setUndoRedoEnabled(false);
  tbplginfo->setText(tr("No selected plugin."));
  tbplginfo->setLineWrapMode(DTextBrowser::LineWrapMode::NoWrap);

  connect(lsplgs, &DListWidget::itemSelectionChanged, this, [=] {
    tbplginfo->clear();
    auto plg = PluginSystem::instance()->plugin(lsplgs->currentRow());
    tbplginfo->append(tr("Name:") + plg->pluginName());

    auto e = QMetaEnum::fromType<IWingToolPlg::Catagorys>();

    tbplginfo->append(QObject::tr("Catagory:") +
                      QObject::tr(e.valueToKey(int(plg->pluginCatagory()))));
    tbplginfo->append(QObject::tr("Version:") +
                      QString::number(plg->pluginVersion()));
    tbplginfo->append(QObject::tr("Author:") + plg->pluginAuthor());
    tbplginfo->append(QObject::tr("Comment:") + plg->pluginComment());
    tbplginfo->append(QObject::tr("Provider:") + plgsys->pluginProvider(plg));
    tbplginfo->append(QObject::tr("Services:"));

    auto &srvs = plgsys->pluginServiceNames(plg);
    auto &trsrvs = plgsys->pluginServicetrNames(plg);
    auto len = srvs.count();
    for (auto i = 0; i < len; i++) {
      tbplginfo->append(
          QString("\t%1 : %2 ( %3 )").arg(i).arg(trsrvs[i]).arg(srvs[i]));
    }
  });

  layout->addWidget(tbplginfo);
  addContent(w);
  tbplginfo->setMinimumHeight(400);
  addSpacing(10);

  auto group = new DButtonBox(this);
  QList<DButtonBoxButton *> blist;
  auto b = new DButtonBoxButton(tr("Select"), this);
  connect(b, &DButtonBoxButton::clicked, this, [=] {
    auto sel = lsplgs->currentIndex().row();
    if (sel < 0) {
      DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                               tr("NoSelection"));
      return;
    }
    this->done(sel);
  });
  blist.append(b);
  b = new DButtonBoxButton(tr("NoPlugin"), this);
  connect(b, &DButtonBoxButton::clicked, this, [=] { this->done(-1); });
  blist.append(b);
  b = new DButtonBoxButton(tr("Cancel"), this);
  connect(b, &DButtonBoxButton::clicked, this, [=] { this->done(-2); });
  blist.append(b);
  group->setButtonList(blist, false);
  addContent(group);
}

void PluginSelDialog::closeEvent(QCloseEvent *event) {
  Q_UNUSED(event);
  done(-2);
}
