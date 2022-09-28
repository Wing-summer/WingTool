#include "centerwindow.h"
#include "shortcuteditdialog.h"
#include <DButtonBox>
#include <DLabel>
#include <DMessageBox>
#include <DMessageManager>
#include <DTextBrowser>
#include <DTitlebar>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMimeDatabase>
#include <QProcess>
#include <QScrollArea>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QVector>

CenterWindow::CenterWindow(DMainWindow *parent) : DMainWindow(parent) {
  QIcon picon = ProgramIcon;
  setWindowTitle(tr("CenterWindow"));
  setMinimumSize(700, 500);
  setWindowIcon(picon);

  auto title = titlebar();
  title->setTitle(tr("CenterWindow"));
  title->setIcon(picon);

  tabs = new DTabWidget(this);
  setCentralWidget(tabs);

  // General
  auto w = new QWidget(this);
  auto scview = new QScrollArea;
  scview->setWidget(w);
  auto vlayout = new QVBoxLayout(w);

  auto l = new DLabel(tr("Common"), this);
  auto font = this->font();
  font.setUnderline(true);
  font.setBold(true);
  font.setItalic(true);
  l->setFont(font);

  vlayout->addWidget(l);
  vlayout->addSpacing(5);
  cbauto = new DCheckBox(tr("AutoStart"), this);
  vlayout->setMargin(20);
  vlayout->addWidget(cbauto, Qt::AlignTop);
  vlayout->addStretch();
  tabs->addTab(w, tr("General"));

  // Hotkeys
  w = new QWidget(this);
  vlayout = new QVBoxLayout(w);
  auto group = new DButtonBox(this);
  vlayout->setMargin(20);
  QList<DButtonBoxButton *> blist;
  auto b = new DButtonBoxButton(tr("Add"), this);
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_addHotkey);
  blist.append(b);
  b = new DButtonBoxButton(tr("Remove"), this);
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_removeHotkey);
  blist.append(b);
  b = new DButtonBoxButton(tr("Edit"), this);
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_editHotkey);
  blist.append(b);
  b = new DButtonBoxButton(tr("Clear"), this);
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_clearHotkey);
  blist.append(b);
  group->setButtonList(blist, false);
  vlayout->addWidget(group, Qt::AlignTop);
  vlayout->addSpacing(10);

  tbhotkeys = new DTableWidget(this);
  tbhotkeys->setEditTriggers(DTableWidget::EditTrigger::NoEditTriggers);
  tbhotkeys->setSelectionBehavior(QAbstractItemView::SelectRows);
  tbhotkeys->setSelectionMode(QAbstractItemView::ExtendedSelection);
  tbhotkeys->setColumnCount(4);
  tbhotkeys->setHorizontalHeaderLabels(
      {tr("Enable"), tr("HotKey"), tr("Exec"), tr("Params")});
  tbhotkeys->setCornerButtonEnabled(true);
  tbhotkeys->horizontalHeader()->setStretchLastSection(true);

  auto *menu = new DMenu(tbhotkeys);
  QAction *a;

#define AddMenuAction(title, slot)                                             \
  a = new QAction(title, menu);                                                \
  connect(a, &QAction::triggered, this, slot);                                 \
  menu->addAction(a);

  AddMenuAction(tr("Enable"), [=] { this->enableSelectedHotkeys(true); });
  hkcmenu.append(a);
  AddMenuAction(tr("Disable"), [=] { this->enableSelectedHotkeys(false); });
  hkcmenu.append(a);
  menu->addSeparator();
  AddMenuAction(tr("Add"), &CenterWindow::on_addHotkey);
  AddMenuAction(tr("Edit"), &CenterWindow::on_editHotkey);
  hkcmenu.append(a);
  AddMenuAction(tr("Remove"), &CenterWindow::on_removeHotkey);
  hkcmenu.append(a);
  AddMenuAction(tr("Clear"), &CenterWindow::on_clearHotkey);

  tbhotkeys->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(tbhotkeys, &DTabWidget::customContextMenuRequested, this, [=] {
    auto flag = tbhotkeys->currentRow() >= 0;
    for (auto item : hkcmenu) {
      item->setEnabled(flag);
    }
    menu->popup(QCursor::pos());
  });
  connect(tbhotkeys, &DTableWidget::cellClicked, this, [=](int row, int) {
    if (row < 0 || row >= scinfos.count())
      return;
    auto b = tbhotkeys->item(row, 0)->checkState() == Qt::Checked;
    scinfos[hotkeys[row]].enabled = b;
    manager->enableHotKey(row, b);
  });
  connect(tbhotkeys, &DTableWidget::cellDoubleClicked, this,
          [=](int row, int) { this->editTask(row); });

  vlayout->addWidget(tbhotkeys);
  tabs->addTab(w, tr("Hotkeys"));

  // ToolBox
  w = new QWidget(this);

  // 左侧
  auto tlayout = new QHBoxLayout(w);
  auto tvlayout = new QVBoxLayout;

  auto gridsize = 40; // 默认 40 先
  auto gw = new QWidget(w);
  gw->setFixedSize(gridsize * 3, gridsize * 3);
  auto mlayout = new QGridLayout(gw);
  mlayout->setMargin(1);
  auto btngs = new QButtonGroup(w);
  btngs->setExclusive(true); // 设置按钮选中互斥

  for (int i = 0; i < 9; i++) {
    auto lbl = new DIconButton(this);
    lbl->setFixedSize(gridsize - 2, gridsize - 2);
    lbl->setIconSize(QSize(gridsize / 2, gridsize / 2));
    lbl->setCheckable(true);
    auto in = std::div(i, 3);
    mlayout->addWidget(lbl, in.quot, in.rem, Qt::AlignCenter);
    lbls[i] = lbl;
    btngs->addButton(lbl);
    connect(lbl, &DIconButton::clicked, this, [=] {

    });
  }
  lbls[0]->setChecked(true);
  auto lbl4 = lbls[4];
  lbl4->setIcon(ICONRES("close"));
  lbl4->setCheckable(false);

  tvlayout->addWidget(gw, 0, Qt::AlignCenter);
  tbtoolinfo = new DTextBrowser(w);
  tbtoolinfo->setUndoRedoEnabled(false);
  tvlayout->addWidget(tbtoolinfo);

  group = new DButtonBox(this);
  blist.clear(); // 重新征用
  b = new DButtonBoxButton(tr("Edit"), this);
  connect(b, &DButtonBoxButton::clicked, this, [=] {});
  blist.append(b);
  b = new DButtonBoxButton(tr("Delete"), this);
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_removeHotkey);
  blist.append(b);
  group->setButtonList(blist, false);
  tvlayout->addWidget(group);
  tlayout->addLayout(tvlayout);

  // 右侧
  tvlayout = new QVBoxLayout;

  group = new DButtonBox(this);
  // 再来征用一次
  blist.clear();
  b = new DButtonBoxButton(ICONRES2("add"));
  b->setIconSize(QSize(20, 20));
  b->setParent(this);
  b->setToolTip(tr("Add"));
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_addToolWin);
  blist.append(b);
  b = new DButtonBoxButton(ICONRES2("del"));
  b->setIconSize(QSize(20, 20));
  b->setParent(this);
  b->setToolTip(tr("Remove"));
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_removeToolWin);
  blist.append(b);
  b = new DButtonBoxButton(ICONRES2("edit"));
  b->setIconSize(QSize(20, 20));
  b->setParent(this);
  b->setToolTip(tr("Edit"));
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_editToolWin);
  blist.append(b);
  b = new DButtonBoxButton(ICONRES2("up"));
  b->setIconSize(QSize(20, 20));
  b->setParent(this);
  b->setToolTip(tr("Up"));
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_upToolWin);
  blist.append(b);
  b = new DButtonBoxButton(ICONRES2("down"));
  b->setIconSize(QSize(20, 20));
  b->setParent(this);
  b->setToolTip(tr("Down"));
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_downToolWin);
  blist.append(b);
  b = new DButtonBoxButton(ICONRES2("clear"));
  b->setIconSize(QSize(20, 20));
  b->setParent(this);
  b->setToolTip(tr("Clear"));
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_clearToolWin);
  blist.append(b);
  group->setButtonList(blist, false);
  tvlayout->addWidget(group);

  lstoolwin = new DListWidget(w);
  menu = new DMenu(lstoolwin);
  AddMenuAction(tr("Add"), &CenterWindow::on_addToolWin);
  AddMenuAction(tr("Edit"), &CenterWindow::on_editToolWin);
  AddMenuAction(tr("Remove"), &CenterWindow::on_removeToolWin);
  AddMenuAction(tr("Clear"), &CenterWindow::on_clearToolWin);
  menu->addSeparator();
  AddMenuAction(tr("Up"), &CenterWindow::on_upToolWin);
  AddMenuAction(tr("Down"), &CenterWindow::on_downToolWin);
  AddMenuAction(tr("TopMost"), [=] {

  });
  AddMenuAction(tr("DownMost"), [=] {

  });
  lstoolwin->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(lstoolwin, &DListWidget::customContextMenuRequested, this,
          [=] { menu->popup(QCursor::pos()); });
  tvlayout->addWidget(lstoolwin);

  tlayout->addLayout(tvlayout);
  tabs->addTab(w, tr("ToolBox"));

  // Plugins
  w = new QWidget(this);
  auto playout = new QHBoxLayout(w);
  lwplgs = new DListWidget(w);
  playout->addWidget(lwplgs);

  tbplginfo = new DTextBrowser(w);
  tbplginfo->setUndoRedoEnabled(false);
  tbplginfo->setText(tr("No selected plugin."));

  connect(lwplgs, &DListWidget::itemSelectionChanged, this, [=] {
    tbplginfo->clear();
    auto plg = plgsys->plugin(lwplgs->currentRow());
    tbplginfo->append(tr("Name:") + plg->pluginName());

    auto e = QMetaEnum::fromType<IWingToolPlg::Catagorys>();

    tbplginfo->append(QObject::tr("Catagory:") +
                      QObject::tr(e.valueToKey(int(plg->pluginCatagory()))));
    tbplginfo->append(QObject::tr("Version:") +
                      QString::number(plg->pluginVersion()));
    tbplginfo->append(QObject::tr("Author:") + plg->pluginAuthor());
    tbplginfo->append(QObject::tr("Comment:") + plg->pluginComment());
    tbplginfo->append(QObject::tr("Provider:") + plg->provider());

    tbplginfo->append(QObject::tr("Services:"));
    int i = 0;
    for (auto &item : plg->pluginServices()) {
      tbplginfo->append(QString("\t%1 : %2").arg(i++).arg(item));
    }

    tbplginfo->append(tr("RegisteredHotkey:"));
    for (auto &item : plgsys->pluginRegisteredHotkey(plg)) {
      tbplginfo->append(QString("\t%1").arg(item.toString()));
    }
  });
  playout->addWidget(tbplginfo);
  tabs->addTab(w, tr("Plugins"));

  // AboutAuthor
  w = new QWidget(this);
  auto alayout = new QVBoxLayout(w);
  l = new DLabel(this);
  l->setFixedSize(100, 100);
  l->setScaledContents(true);
  l->setPixmap(QPixmap(":/images/author.jpg"));
  alayout->addWidget(l);
  alayout->addSpacing(10);
  alayout->addWidget(l, 0, Qt::AlignCenter);

  auto tb = new DTextBrowser(this);
  tb->setSearchPaths(QStringList({":/", ":/images"}));
  tb->setSource(QUrl("README.md"), QTextDocument::MarkdownResource);
  tb->setOpenExternalLinks(true);
  alayout->addWidget(tb);

  tabs->addTab(w, tr("About"));

  // Sponsor
  w = new QWidget(this);
  auto slayout = new QVBoxLayout(w);
  slayout->addWidget(new DLabel(tr("ThanksForSponsor"), this), 0,
                     Qt::AlignCenter);
  slayout->addSpacing(5);

  l = new DLabel(this);
  l->setPixmap(QPixmap(":/sponsor.png"));
  l->setScaledContents(true);
  slayout->addWidget(l);
  tabs->addTab(w, tr("Sponsor"));
}

void CenterWindow::show(CenterWindow::TabPage index) {
  tabs->setCurrentIndex(int(index));
  DMainWindow::show();
}

QStringList CenterWindow::parseCmdParams(QString str) {
  static QRegularExpression regex("(\"[^\"]+\"|[^\\s\"]+)");
  QStringList args;
  int off = 0;
  while (true) {
    auto match = regex.match(str, off);
    if (!match.hasMatch()) {
      break;
    }
    auto res = match.captured();
    if (res[0] == '\"')
      res = res.replace("\"", "");
    if (res[0] == '\'')
      res = res.replace("'", "");
    args << res;
    off = match.capturedEnd();
  }
  return args;
}

bool CenterWindow::runTask(ToolStructInfo record) {

  if (record.isPlugin) {
    auto params = parseCmdParams(record.params);
    QList<QVariant> ps;
    for (auto &item : params) {
      ps.append(item);
    }
    plgsys->pluginCall(record.provider, record.serviceID, ps);
    return true;
  }

  QMimeDatabase db;

  QFileInfo info(record.process);
  auto absp = info.absoluteFilePath();

  auto mt = db.mimeTypeForFile(absp);
  auto n = mt.name();
  if (n == "application/x-executable") {
    if (!pstart.startDetached(absp, parseCmdParams(record.params))) {
      DMessageBox::critical(this, tr("runErr"), pstart.errorString());
      return false;
    }
  } else {
    if (!QDesktopServices::openUrl(QUrl("file://" + absp))) {
      DMessageBox::critical(this, tr("err"), tr("openErr"));
      return false;
    }
  }
  return true;
}

void CenterWindow::editTask(int index) {
  if (index < 0 || index >= scinfos.count())
    return;
  auto &task = scinfos[hotkeys[index]];
  ShortCutEditDialog d(task);
  if (d.exec()) {
    auto res = d.getResult();
    auto wt = new QTableWidgetItem;
    wt->setCheckState(res.enabled ? Qt::Checked : Qt::Unchecked);
    tbhotkeys->setItem(index, 0, wt);
    tbhotkeys->setItem(index, 1, new QTableWidgetItem(res.seq.toString()));
    wt = new QTableWidgetItem(QString(res.process));
    wt->setToolTip(res.process);
    tbhotkeys->setItem(index, 2, wt);
    wt = new QTableWidgetItem(res.params);
    wt->setToolTip(res.params);
    tbhotkeys->setItem(index, 3, wt);

    task = res;
    manager->editHotkey(index, res.seq);
  }
}

void CenterWindow::on_editHotkey() { this->editTask(tbhotkeys->currentRow()); }

void CenterWindow::on_removeHotkey() {
  auto selrows = tbhotkeys->selectionModel()->selectedRows();
  if (!selrows.length())
    return;
  if (selrows.length() > 1) {
    QVector<int> nums;
    for (auto &item : selrows)
      nums.append(item.row());
    std::sort(nums.begin(), nums.end(), std::greater<int>());
    for (auto item : nums) {
      auto hk = hotkeys.takeAt(item);
      scinfos.remove(hk);
      manager->unregisterHotkey(item);
      tbhotkeys->removeRow(item);
    }
  } else {
    auto row = tbhotkeys->currentRow();
    auto hk = hotkeys.takeAt(row);
    scinfos.remove(hk);
    manager->unregisterHotkey(row);
    tbhotkeys->removeRow(row);
  }
}

void CenterWindow::on_clearHotkey() {
  scinfos.clear();
  hotkeys.clear();
  manager->clearHotkey();
  tbhotkeys->setRowCount(0);
  DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                           tr("ClearSuccess"));
}

void CenterWindow::on_addHotkey() {
  ShortCutEditDialog d;
  if (d.exec()) {
    auto res = d.getResult();
    auto hk = manager->registerHotkey(res.seq, true);
    if (hk == nullptr) {
      DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                               tr("HotkeyRegisterFail"));
      return;
    }
    scinfos.insert(hk, res);
    hotkeys.append(hk);

    auto index = tbhotkeys->rowCount();
    tbhotkeys->setRowCount(index + 1);
    auto wt = new QTableWidgetItem;
    wt->setCheckState(res.enabled ? Qt::Checked : Qt::Unchecked);
    tbhotkeys->setItem(index, 0, wt);
    tbhotkeys->setItem(index, 1, new QTableWidgetItem(res.seq.toString()));
    wt = new QTableWidgetItem(res.process);
    wt->setToolTip(res.process);
    tbhotkeys->setItem(index, 2, wt);
    wt = new QTableWidgetItem(res.params);
    wt->setToolTip(res.params);
    tbhotkeys->setItem(index, 3, wt);
  }
}

void CenterWindow::enableSelectedHotkeys(bool enable) {
  auto selrows = tbhotkeys->selectionModel()->selectedRows();
  for (auto &item : selrows) {
    manager->enableHotKey(item.row(), enable);
  }
}

void CenterWindow::on_editToolWin() {}

void CenterWindow::on_removeToolWin() {}

void CenterWindow::on_clearToolWin() {}

void CenterWindow::on_addToolWin() {}

void CenterWindow::on_upToolWin() {}

void CenterWindow::on_downToolWin() {}

void CenterWindow::getHokeysBuffer(QList<QHotkey *> &hotkeysBuf,
                                   QMap<QHotkey *, ToolStructInfo> &buffer) {}

void CenterWindow::getToolLeftBuffer(ToolStructInfo buffer[]) {}

void CenterWindow::getToolRightBuffer(QList<ToolStructInfo> &buffer) {}

void CenterWindow::loadingFinish() {
  sm = SettingManager::instance();
  auto gridsize = sm->toolGridSize();
  for (auto i = 0; i < 9; i++) {
    lbls[i]->setFixedSize(QSize(gridsize, gridsize));
  }
}

void CenterWindow::initPluginSys() {
  plgsys = PluginSystem::instance();
  for (auto item : PluginSystem::instance()->plugins()) {
    lwplgs->addItem(new QListWidgetItem(Utilities::processPluginIcon(item),
                                        item->pluginName()));
  }
}

void CenterWindow::initAppManger() {
  manager = AppManager::instance();

  //初始化热键事件处理函数
  connect(manager, &AppManager::hotkeyTirggered, this,
          [=](const Hotkey *hotkey) {
            if (hotkey->isHostHotkey()) {
              auto &task = scinfos[const_cast<Hotkey *>(hotkey)];
              this->runTask(task);
            }
          });
  connect(manager, &AppManager::hotkeyReleased, this, [=](const Hotkey *) {

  });
  connect(manager, &AppManager::hotkeyEnableChanged, this,
          [=](bool value, const Hotkey *hotkey) {
            if (hotkey->isHostHotkey()) {
              tbhotkeys->item(hotkeys.indexOf(const_cast<Hotkey *>(hotkey)), 0)
                  ->setCheckState(value ? Qt::Checked : Qt::Unchecked);
            }
          });
}

void CenterWindow::closeEvent(QCloseEvent *event) {
  event->ignore();
  hide();
}
