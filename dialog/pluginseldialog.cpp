#include "pluginseldialog.h"
#include "plugin/pluginsystem.h"
#include "utilities.h"
#include <DButtonBox>
#include <DMessageManager>
#include <QHBoxLayout>
#include <QMetaEnum>

PluginSelDialog::PluginSelDialog(DDialog *parent) : DDialog(parent) {
  setWindowTitle(tr("PluginSelectingDialog"));

  auto w = new QWidget(this);
  auto layout = new QHBoxLayout(w);

  lsplgs = new DListWidget(this);
  int i = -1;
  for (auto item : PluginSystem::instance()->plugins()) {
    i++;
    if (!item->isTool())
      continue;
    auto l = new QListWidgetItem(Utilities::processPluginIcon(item),
                                 item->pluginName());
    l->setData(Qt::UserRole, i);
    lsplgs->addItem(l);
  }
  layout->addWidget(lsplgs);
  addSpacing(10);

  tbplginfo = new DTextBrowser(this);
  tbplginfo->setUndoRedoEnabled(false);
  tbplginfo->setText(tr("No selected plugin."));

  connect(lsplgs, &DListWidget::itemSelectionChanged, this, [=] {
    tbplginfo->clear();
    auto plg = PluginSystem::instance()->plugin(lsplgs->currentRow());
    tbplginfo->append(tr("Name:") + plg->pluginName());

    auto e = QMetaEnum::fromType<IWingToolPlg::Catagorys>();

    tbplginfo->append(QObject::tr("Catagory:") +
                      QObject::tr(e.valueToKey(int(plg->pluginCatagory()))));
    tbplginfo->append(QObject::tr("Version") +
                      QString::number(plg->pluginVersion()));
    tbplginfo->append(QObject::tr("Author:") + plg->pluginAuthor());
    tbplginfo->append(QObject::tr("Comment:") + plg->pluginComment());
    tbplginfo->append(QObject::tr("Provider:") + plg->provider());
    tbplginfo->append(QObject::tr("Services:"));
    int i = 0;
    for (auto &item : plg->pluginServices()) {
      tbplginfo->append(QString("\t%1 : %2").arg(i++).arg(item));
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
    auto item = lsplgs->currentItem();
    if (!item) {
      DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                               tr("NoSelection"));
      return;
    }
    auto sel = item->data(Qt::UserRole).toInt();
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
