#include "centerwindow.h"
#include "shortcuteditdialog.h"
#include "tooleditdialog.h"
#include "toolswapdialog.h"
#include <DButtonBox>
#include <DFileDialog>
#include <DLabel>
#include <DMessageBox>
#include <DMessageManager>
#include <DTextBrowser>
#include <DTitlebar>
#include <DWidgetUtil>
#include <QButtonGroup>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFormLayout>
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
  vlayout->setMargin(20);
  auto l = new DLabel(tr("Common"), this);
  auto font = this->font();
  font.setUnderline(true);
  font.setBold(true);
  font.setItalic(true);
  l->setFont(font);

  vlayout->addWidget(l);
  vlayout->addSpacing(5);
  auto gw = new QWidget(w);
  vlayout->addWidget(gw);
  auto hlayout = new QHBoxLayout(gw);
  vlayout->addLayout(hlayout);
  cbToolWin = new DCheckBox(tr("EnabledToolWin"), this);
  hlayout->addWidget(cbToolWin);
  cbWinTool = new DCheckBox(tr("EnabledWinTool"), this);
  hlayout->addWidget(cbWinTool);
  gw = new QWidget(w);
  vlayout->addWidget(gw);
  hlayout = new QHBoxLayout(gw);
  hlayout->addWidget(new DLabel(tr("ToolWinGridSize"), this));
  hlayout->addSpacing(5);
  sbGridsize = new DSpinBox(w);
  sbGridsize->setRange(30, 60);
  sbGridsize->setValue(40); // 默认 40 先
  hlayout->addWidget(sbGridsize, 1);

  vlayout->addSpacing(10);
  l = new DLabel(tr("Shortcut"), this);
  l->setFont(font);
  vlayout->addWidget(l);
  vlayout->addSpacing(5);
  gw = new QWidget(w);
  vlayout->addWidget(gw);
  auto flayout = new QFormLayout(gw);
  kseqTool = new DKeySequenceEdit(gw);
  flayout->addRow(tr("ToolBox:"), kseqTool);
  hlayout = new QHBoxLayout(gw);
  cbMod = new DComboBox(gw);
  cbMod->addItems({"Ctrl", "Shift", "Alt", "Super"});
  cbMod->setCurrentIndex(0);
  hlayout->addWidget(cbMod);
  cbMouseBtn = new DComboBox(gw);
  cbMouseBtn->addItems(
      {tr("Left"), tr("Right"), tr("Middle"), tr("XButton1"), tr("XButton2")});
  cbMouseBtn->setCurrentIndex(2);
  hlayout->addWidget(cbMouseBtn);
  flayout->addRow(tr("ToolWin:"), hlayout);

  vlayout->addSpacing(10);
  l = new DLabel(tr("Config"), this);
  l->setFont(font);
  vlayout->addWidget(l);
  vlayout->addSpacing(5);
  auto group = new DButtonBox(this);
  QList<DButtonBoxButton *> blist;
  auto b = new DButtonBoxButton(tr("Export"), this);
  connect(b, &DButtonBoxButton::clicked, this,
          [=] { this->on_exportSettings(); });
  blist.append(b);
  b = new DButtonBoxButton(tr("Import"), this);
  connect(b, &DButtonBoxButton::clicked, this,
          [=] { this->on_importSettings(); });
  blist.append(b);
  b = new DButtonBoxButton(tr("Save"), this);
  connect(b, &DButtonBoxButton::clicked, this, [=] { sm->saveSettings(); });
  blist.append(b);
  b = new DButtonBoxButton(tr("Reset"), this);
  connect(b, &DButtonBoxButton::clicked, this,
          [=] { this->on_resetSettings(); });
  blist.append(b);
  group->setButtonList(blist, false);
  vlayout->addWidget(group);

  vlayout->addSpacing(10);
  l = new DLabel(tr("Software"), this);
  l->setFont(font);
  vlayout->addWidget(l);
  vlayout->addSpacing(5);

  vlayout->addWidget(new DLabel(QString(tr("%1 , Ver %2 , by WingSummer."))
                                    .arg(qApp->applicationName())
                                    .arg(qApp->applicationVersion()),
                                this));

  vlayout->addStretch();
  tabs->addTab(w, tr("General"));

  // Hotkeys
  w = new QWidget(this);
  vlayout = new QVBoxLayout(w);
  group = new DButtonBox(this);
  vlayout->setMargin(20);
  blist.clear(); // 征用
  b = new DButtonBoxButton(tr("Add"), this);
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
    auto flag = tbhotkeys->selectedItems().count() > 0;
    for (auto item : hkcmenu) {
      item->setEnabled(flag);
    }
    menu->popup(QCursor::pos());
  });
  connect(tbhotkeys, &DTableWidget::cellClicked, this, [=](int row, int col) {
    if (col)
      return;
    if (row < 0 || row >= scinfos.count())
      return;
    auto b = tbhotkeys->item(row, 0)->checkState() == Qt::Checked;
    auto hk = hotkeys[row];
    scinfos[hk].enabled = b;
    manager->enableHotKey(hk, b);
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
  gw = new QWidget(w);
  delete gw->layout();
  auto mlayout = new QGridLayout;
  gw->setLayout(mlayout);
  mlayout->setSpacing(1);
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
    connect(lbl, &DIconButton::toggled, this, [=](bool v) {
      if (v) {
        auto info = toolinfos[i];
        sellbl = i;
        if (info.isPlugin) {
          // 这里的含义是是否为 null
          if (info.enabled) {
            auto pi = info.pluginIndex;
            auto plg = plgsys->plugin(pi);
            auto e = QMetaEnum::fromType<IWingToolPlg::Catagorys>();

            tbtoolinfo->setText(tr("[Plugin]"));
            tbtoolinfo->append(QObject::tr("PluginName:") + plg->pluginName());
            tbtoolinfo->append(tr("Service:") + Utilities::getPluginServiceName(
                                                    plg, info.serviceID));
            tbtoolinfo->append(tr("Params:") + info.params);

            auto fw = tbtoolinfo->fontMetrics().horizontalAdvance('=');
            tbtoolinfo->append(QString(tbtoolinfo->width() / fw, '='));
            tbtoolinfo->append(
                QObject::tr("Catagory:") +
                QObject::tr(e.valueToKey(int(plg->pluginCatagory()))));
            tbtoolinfo->append(QObject::tr("Version:") +
                               QString::number(plg->pluginVersion()));
            tbtoolinfo->append(QObject::tr("Author:") + plg->pluginAuthor());
            tbtoolinfo->append(QObject::tr("Comment:") + plg->pluginComment());
            tbtoolinfo->append(QObject::tr("Provider:") + plg->provider());
          } else {
            tbtoolinfo->setText(tr("NoTool"));
          }
        } else {
          if (info.enabled) {
            tbtoolinfo->setText(tr("[File]"));
            tbtoolinfo->append(tr("FileName:") + info.process);
            tbtoolinfo->append(tr("Params:") + info.params);
          } else {
            tbtoolinfo->setText(tr("NoTool"));
          }
        }
      }
    });
  }
  auto lbl4 = lbls[4];
  lbl4->setIcon(ICONRES("close"));

  tvlayout->addWidget(gw, 0, Qt::AlignCenter);
  tbtoolinfo = new DTextBrowser(w);
  tbtoolinfo->setUndoRedoEnabled(false);
  tvlayout->addWidget(tbtoolinfo);

  group = new DButtonBox(this);
  blist.clear(); // 重新征用
  b = new DButtonBoxButton(ICONRES2("edit"));
  b->setParent(this);
  b->setIconSize(QSize(20, 20));
  b->setToolTip(tr("Edit"));
  connect(b, &DButtonBoxButton::clicked, this, [=] {
    ToolEditDialog d(toolinfos[sellbl]);
    if (d.exec()) {
      auto res = d.getResult();
      this->setToolWinInfo(sellbl, res);
      sm->setModified();
      emit lbls[sellbl]->toggled(true);
    }
  });
  blist.append(b);
  b = new DButtonBoxButton(ICONRES2("swap"));
  b->setParent(this);
  b->setIconSize(QSize(20, 20));
  b->setToolTip(tr("Swap"));
  connect(b, &DButtonBoxButton::clicked, this, [=] {
    QVector<QIcon> icons;
    for (auto i = 0; i < 9; i++) {
      icons.append(lbls[i]->icon());
    }

    ToolSwapDialog d(icons, this->sellbl);
    auto res = d.exec();
    if (res >= 0) {
      auto tmp = toolinfos[sellbl];
      toolinfos[sellbl] = toolinfos[res];
      toolinfos[res] = tmp;

      auto icon1 = lbls[sellbl]->icon();
      auto icon2 = lbls[res]->icon();
      lbls[sellbl]->setIcon(icon2);
      lbls[res]->setIcon(icon1);

      manager->setToolIcon(sellbl, icon2);
      manager->setToolIcon(res, icon1);
      lbls[res]->setChecked(true);
    }
  });
  blist.append(b);
  b = new DButtonBoxButton(ICONRES2("del"));
  b->setParent(this);
  b->setIconSize(QSize(20, 20));
  b->setToolTip(tr("Delete"));
  connect(b, &DButtonBoxButton::clicked, this, [=] {
    toolinfos[sellbl].enabled = false;
    auto ilbl = lbls[sellbl];
    ilbl->setIcon(QIcon());
    manager->setToolIcon(sellbl, QIcon());
    emit ilbl->toggled(true);
  });
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
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_addWinTool);
  blist.append(b);
  b = new DButtonBoxButton(ICONRES2("del"));
  b->setIconSize(QSize(20, 20));
  b->setParent(this);
  b->setToolTip(tr("Remove"));
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_removeWinTool);
  blist.append(b);
  b = new DButtonBoxButton(ICONRES2("edit"));
  b->setIconSize(QSize(20, 20));
  b->setParent(this);
  b->setToolTip(tr("Edit"));
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_editWinTool);
  blist.append(b);
  b = new DButtonBoxButton(ICONRES2("up"));
  b->setIconSize(QSize(20, 20));
  b->setParent(this);
  b->setToolTip(tr("Up"));
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_upWinTool);
  blist.append(b);
  b = new DButtonBoxButton(ICONRES2("down"));
  b->setIconSize(QSize(20, 20));
  b->setParent(this);
  b->setToolTip(tr("Down"));
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_downWinTool);
  blist.append(b);
  b = new DButtonBoxButton(ICONRES2("clear"));
  b->setIconSize(QSize(20, 20));
  b->setParent(this);
  b->setToolTip(tr("Clear"));
  connect(b, &DButtonBoxButton::clicked, this, &CenterWindow::on_clearWinTool);
  blist.append(b);
  group->setButtonList(blist, false);
  tvlayout->addWidget(group);

  lstoolwin = new DListWidget(w);
  menu = new DMenu(lstoolwin);
  AddMenuAction(tr("Add"), &CenterWindow::on_addWinTool);
  AddMenuAction(tr("Edit"), &CenterWindow::on_editWinTool);
  lscmenu.append(a);
  AddMenuAction(tr("Remove"), &CenterWindow::on_removeWinTool);
  lscmenu.append(a);
  AddMenuAction(tr("Clear"), &CenterWindow::on_clearWinTool);
  menu->addSeparator();
  AddMenuAction(tr("Up"), &CenterWindow::on_upWinTool);
  lscmenu.append(a);
  AddMenuAction(tr("Down"), &CenterWindow::on_downWinTool);
  lscmenu.append(a);
  AddMenuAction(tr("TopMost"), [=] {
    if (lstoolwin->count() == 1)
      return;

    auto sels = lstoolwin->selectionModel()->selectedRows();
    if (sels.isEmpty())
      return;
    QVector<int> nums;
    for (auto &item : sels) {
      nums.append(item.row());
    }

    std::sort(nums.begin(), nums.end());

    // 过滤掉不需移动的选项
    int i = 0;
    auto len = nums.count();
    for (; i < len; i++) {
      if (nums[i] != i)
        break;
    }

    // 下面就开始乾坤大挪移
    for (auto p = nums.rbegin(); p != nums.rend() - i; p++) {
      auto pi = *p;
      wintoolinfos.move(pi, i);
      auto item = lstoolwin->takeItem(pi);
      lstoolwin->insertItem(i, item);
      wintool.mvItem(pi, i);
    }
    sm->setModified();
  });
  lscmenu.append(a);
  AddMenuAction(tr("DownMost"), [=] {
    if (lstoolwin->count() == 1)
      return;

    auto sels = lstoolwin->selectionModel()->selectedRows();
    if (sels.isEmpty())
      return;
    QVector<int> nums;
    for (auto &item : sels) {
      nums.append(item.row());
    }

    std::sort(nums.begin(), nums.end(), std::greater<int>());

    // 过滤掉不需移动的选项
    int i = 0;
    auto len = nums.count();
    if (nums.first() == lstoolwin->count() - 1) {
      i++;
      int pi = nums.first();
      for (; i < len; i++) {
        pi--;
        if (nums[i] != pi)
          break;
      }
    }

    // 下面就开始乾坤大挪移
    for (auto p = nums.rbegin(); p != nums.rend() - i; p++) {
      auto pi = *p;
      wintoolinfos.move(pi, i);
      auto item = lstoolwin->takeItem(pi);
      lstoolwin->insertItem(i, item);
      wintool.mvItem(pi, i);
    }
    sm->setModified();
  });
  lscmenu.append(a);
  lstoolwin->setContextMenuPolicy(Qt::CustomContextMenu);
  lstoolwin->setSelectionMode(QListWidget::SelectionMode::ExtendedSelection);
  lstoolwin->setDragDropMode(QListWidget::DragDropMode::NoDragDrop);
  connect(lstoolwin, &DListWidget::customContextMenuRequested, this, [=] {
    auto flag = lstoolwin->selectedItems().count() > 0;
    for (auto item : lscmenu) {
      item->setEnabled(flag);
    }
    menu->popup(QCursor::pos());
  });
  tvlayout->addWidget(lstoolwin);

  tlayout->addLayout(tvlayout);
  tabs->addTab(w, tr("ToolBox"));

  // Plugins
  w = new QWidget(this);
  auto playout = new QHBoxLayout(w);
  auto pvlayout = new QVBoxLayout;
  lwplgs = new DListWidget(w);
  playout->addWidget(lwplgs);
  playout->addLayout(pvlayout);

  tbplginfo = new DTextBrowser(w);
  tbplginfo->setUndoRedoEnabled(false);
  tbplginfo->setText(tr("No selected plugin."));

  pvlayout->addWidget(tbplginfo, 1);
  auto btnplgset = new DPushButton(tr("Setting"), w);
  connect(btnplgset, &DPushButton::clicked, this, [=] {
    auto plg = plgsys->plugin(lwplgs->currentRow());
    plg->onSetting();
  });
  btnplgset->setEnabled(false);
  pvlayout->addWidget(btnplgset);

  connect(lwplgs, &DListWidget::itemSelectionChanged, this, [=] {
    if (lwplgs->currentRow() < 0) {
      tbplginfo->setText(tr("No selected plugin."));
      btnplgset->setEnabled(false);
      return;
    }
    tbplginfo->clear();
    btnplgset->setEnabled(true);
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

    auto c = plg->pluginServices();
    auto cn = plg->pluginServiceNames();
    if (c.count() == cn.count()) {
      for (int i = 0; i < c.count(); i++) {
        tbplginfo->append(
            QString("\t%1 : %2 ( %3 )").arg(i).arg(cn[i]).arg(c[i]));
      }
    } else {
      int i = 0;
      for (auto &item : plg->pluginServices()) {
        tbplginfo->append(QString("\t%1 : %2").arg(i++).arg(item));
      }
    }

    tbplginfo->append(tr("RegisteredHotkey:"));
    for (auto &item : plgsys->pluginRegisteredHotkey(plg)) {
      tbplginfo->append(QString("\t%1").arg(item.toString()));
    }
  });

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
  if (!record.enabled)
    return true;
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
  auto hk = hotkeys[index];
  auto &task = scinfos[hk];
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
    manager->editHotkey(hk, res.seq);
    sm->setModified();
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
      manager->unregisterHotkey(hk);
      tbhotkeys->removeRow(item);
    }
  } else {
    auto row = tbhotkeys->currentRow();
    auto hk = hotkeys.takeAt(row);
    scinfos.remove(hk);
    manager->unregisterHotkey(hk);
    tbhotkeys->removeRow(row);
  }
  sm->setModified();
}

void CenterWindow::on_clearHotkey() {
  scinfos.clear();
  hotkeys.clear();
  manager->clearHotkey();
  tbhotkeys->setRowCount(0);
  sm->setModified();
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
    sm->setModified();
  }
}

void CenterWindow::enableSelectedHotkeys(bool enable) {
  auto selrows = tbhotkeys->selectionModel()->selectedRows();
  for (auto &item : selrows) {
    manager->enableHotKey(hotkeys[item.row()], enable);
    sm->setModified();
  }
}

void CenterWindow::on_editWinTool() {
  auto sels = lstoolwin->selectedItems().count();
  if (sels != 1) {
    DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                             tr("PleaseSelectOne"));
    return;
  }
  auto index = lstoolwin->currentRow();
  ToolEditDialog d(wintoolinfos[index]);
  if (d.exec()) {
    auto res = d.getResult();
    wintoolinfos[index] = res;
    auto item = lstoolwin->item(index);
    auto plg = plgsys->plugin(res.pluginIndex);
    item->setIcon(Utilities::trimIconFromInfo(plg, res));
    item->setText(Utilities::getProgramName(plg, res));
    item->setToolTip(res.process);
    sm->setModified();
  }
}

void CenterWindow::on_removeWinTool() {
  auto sels = lstoolwin->selectionModel()->selectedRows();

  if (sels.isEmpty())
    return;

  QVector<int> nums;
  for (auto &item : sels) {
    nums.append(item.row());
  }

  std::sort(nums.begin(), nums.end(), std::greater<int>());

  for (auto index : nums) {
    wintoolinfos.removeAt(index);
    auto item = lstoolwin->takeItem(index);
    delete item;
    wintool.rmItem(index);
  }
  sm->setModified();
}

void CenterWindow::on_clearWinTool() {
  lstoolwin->clear();
  wintoolinfos.clear();
  wintool.rmItem(-1); // 清空数据
  sm->setModified();
  DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                           tr("ClearSuccess"));
}

void CenterWindow::on_addWinTool() {
  ToolEditDialog d;
  if (d.exec()) {
    auto res = d.getResult();
    auto index = lstoolwin->currentRow();
    if (index < 0) {
      wintoolinfos.append(res);
      auto plg = plgsys->plugin(res.pluginIndex);
      auto item = new QListWidgetItem(Utilities::trimIconFromInfo(plg, res),
                                      Utilities::getProgramName(plg, res));
      item->setToolTip(Utilities::getToolTipContent(plg, res));
      lstoolwin->addItem(item);
      wintool.addItem(res);
    } else {
      wintoolinfos.insert(index + 1, res);
      auto plg = plgsys->plugin(res.pluginIndex);
      auto item = new QListWidgetItem(Utilities::trimIconFromInfo(plg, res),
                                      Utilities::getProgramName(plg, res));
      item->setToolTip(Utilities::getToolTipContent(plg, res));
      lstoolwin->insertItem(index + 1, item);
      wintool.addItem(res, index + 1);
    }
    sm->setModified();
  }
}

void CenterWindow::on_upWinTool() {
  if (lstoolwin->count() == 1)
    return;

  auto sels = lstoolwin->selectionModel()->selectedRows();
  if (sels.isEmpty())
    return;
  QVector<int> nums;
  for (auto &item : sels) {
    nums.append(item.row());
  }

  std::sort(nums.begin(), nums.end());

  // 过滤掉不需移动的选项
  int i = 0;
  auto len = nums.count();
  for (; i < len; i++) {
    if (nums[i] != i)
      break;
  }

  // 下面就开始乾坤大挪移
  for (; i < len; i++) {
    auto p = nums[i];
    wintoolinfos.move(p, p - 1);
    auto item = lstoolwin->takeItem(p);
    lstoolwin->insertItem(p - 1, item);
    wintool.mvItem(p, p - 1);
  }

  sm->setModified();
}

void CenterWindow::on_downWinTool() {
  if (lstoolwin->count() == 1)
    return;

  auto sels = lstoolwin->selectionModel()->selectedRows();
  if (sels.isEmpty())
    return;
  QVector<int> nums;
  for (auto &item : sels) {
    nums.append(item.row());
  }

  std::sort(nums.begin(), nums.end(), std::greater<int>());

  // 过滤掉不需移动的选项
  int i = 0;
  auto len = nums.count();
  if (nums.first() == lstoolwin->count() - 1) {
    i++;
    int pi = nums.first();
    for (; i < len; i++) {
      pi--;
      if (nums[i] != pi)
        break;
    }
  }

  // 下面就开始乾坤大挪移
  for (; i < len; i++) {
    auto p = nums[i];
    wintoolinfos.move(p, p + 1);
    auto item = lstoolwin->takeItem(p);
    lstoolwin->insertItem(p + 1, item);
    wintool.mvItem(p, p + 1);
  }
  sm->setModified();
}

void CenterWindow::on_exportSettings() {
  auto path = DFileDialog::getSaveFileName(this, tr(""), QString(),
                                           tr("Config (*.wtcfg)"));
  if (path.isEmpty())
    return;

  if (sm->exportSettings(path)) {
    DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                             tr("ExportSuccess"));
  }
}

void CenterWindow::on_importSettings() {
  auto path = DFileDialog::getOpenFileName(this, tr(""), QString(),
                                           tr("Config (*.wtcfg)"));
  if (path.isEmpty())
    return;

  if (sm->loadSettings(path)) {
    DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                             tr("ImportSuccess"));
  }
}

void CenterWindow::on_resetSettings() { sm->resetSettings(); }

void CenterWindow::addHotKeyInfo(ToolStructInfo &info) {
  // 添加 UI 项目
  auto index = tbhotkeys->rowCount();
  tbhotkeys->setRowCount(index + 1);
  auto wt = new QTableWidgetItem;
  wt->setCheckState(info.enabled ? Qt::Checked : Qt::Unchecked);
  tbhotkeys->setItem(index, 0, wt);
  tbhotkeys->setItem(index, 1, new QTableWidgetItem(info.seq.toString()));
  wt = new QTableWidgetItem(info.process);
  wt->setToolTip(info.process);
  tbhotkeys->setItem(index, 2, wt);
  wt = new QTableWidgetItem(info.params);
  wt->setToolTip(info.params);
  tbhotkeys->setItem(index, 3, wt);

  // 注册热键
  auto hk = manager->registerHotkey(info.seq, true);
  hotkeys << hk;
  scinfos.insert(hk, info);

  // 修正热键状态
  manager->enableHotKey(hk, info.enabled);
}

void CenterWindow::setToolWinInfo(int index, ToolStructInfo &info) {
  toolinfos[index] = info;
  auto icon =
      Utilities::trimIconFromInfo(plgsys->plugin(info.pluginIndex), info);
  auto ilbl = lbls[index];
  ilbl->setIcon(icon);
  manager->setToolIcon(index, icon);
}

void CenterWindow::setToolFinished() { lbls[0]->setChecked(true); }

void CenterWindow::addWinToolInfo(ToolStructInfo &info) {
  wintoolinfos.append(info);
  auto plg = plgsys->plugin(info.pluginIndex);
  auto item = new QListWidgetItem(Utilities::trimIconFromInfo(plg, info),
                                  Utilities::getProgramName(plg, info));
  item->setToolTip(Utilities::getToolTipContent(plg, info));
  lstoolwin->addItem(item);
  wintool.addItem(info);
}

void CenterWindow::initGeneralSettings() {
  sm = SettingManager::instance();

  // 注册有关设置更改相关信号
  cbWinTool->setChecked(sm->wintoolEnabled());
  cbToolWin->setChecked(sm->toolwinEnabled());

  connect(cbToolWin, &DCheckBox::toggled, this,
          [=](bool v) { sm->setToolwinEnabled(v); });
  connect(cbWinTool, &DCheckBox::toggled, this, [=](bool v) {
    sm->setWintoolEnabled(v);
    hkwintool->setRegistered(v);
  });

  connect(sm, &SettingManager::sigToolGridSizeChanged, this, [=](int v) {
    for (auto i = 0; i < 9; i++) {
      lbls[i]->setFixedSize(QSize(v, v));
    }
  });
  sm->sigToolGridSizeChanged(sm->toolGridSize());
  connect(sbGridsize, QOverload<int>::of(&DSpinBox::valueChanged), sm,
          &SettingManager::setToolGridSize);
  connect(tabs, &DTabWidget::currentChanged, this, [=] { sm->saveSettings(); });

  // WinTool 相关
  wintool.setModal(true);
  connect(&wintool, &ToolBoxWindow::sigRun, this,
          [=](int index) { this->runTask(wintoolinfos[index]); });
  auto seq = sm->toolBoxHotkey();
  kseqTool->setKeySequence(seq);
  hkwintool = manager->registerHotkey(seq, false);
  hkwintool->disconnect(); // 断开所有自带连接,不能让它走默认处理
  connect(hkwintool, &Hotkey::activated, this, [&] {
    Dtk::Widget::moveToCenter(&wintool);
    wintool.show();
    Utilities::activeWindowFromDock(wintool.winId());
  });
  connect(kseqTool, &DKeySequenceEdit::editingFinished, this,
          [=](const QKeySequence &keySequence) {
            sm->setToolBoxHotkey(keySequence);
          });
  connect(sm, &SettingManager::sigToolBoxHotkeyChanged, this,
          [=](const QKeySequence seq) { hkwintool->setShortcut(seq, true); });

  connect(sm, &SettingManager::sigToolwinEnabledChanged, this,
          [=](bool b) { cbToolWin->setChecked(b); });
  connect(sm, &SettingManager::sigWintoolEnabledChanged, this,
          [=](bool b) { cbWinTool->setChecked(b); });

  connect(sm, &SettingManager::sigToolwinModChanged, this,
          [=](const Qt::KeyboardModifier mod) {
            switch (mod) {
            case Qt::KeyboardModifier::AltModifier:
              cbMod->setCurrentIndex(2);
              break;
            case Qt::KeyboardModifier::MetaModifier:
              cbMod->setCurrentIndex(3);
              break;
            case Qt::KeyboardModifier::ShiftModifier:
              cbMod->setCurrentIndex(1);
              break;
            default:
              cbMod->setCurrentIndex(0);
              break;
            }
          });
  sm->sigToolwinModChanged(sm->toolwinMod());

  connect(sm, &SettingManager::sigToolwinMouseBtnChanged, this,
          [=](const Qt::MouseButton btn) {
            switch (btn) {
            case Qt::MouseButton::RightButton:
              cbMouseBtn->setCurrentIndex(1);
              break;
            case Qt::MouseButton::MidButton:
              cbMouseBtn->setCurrentIndex(2);
              break;
            case Qt::MouseButton::XButton1:
              cbMouseBtn->setCurrentIndex(3);
              break;
            case Qt::MouseButton::XButton2:
              cbMouseBtn->setCurrentIndex(4);
              break;
            default:
              cbMouseBtn->setCurrentIndex(0);
              break;
            }
          });
  sm->sigToolwinMouseBtnChanged(sm->toolwinMouseBtn());

  connect(cbMod, QOverload<int>::of(&DComboBox::currentIndexChanged), this,
          [=](int index) {
            Qt::KeyboardModifier mod = Qt::KeyboardModifier::NoModifier;
            switch (index) {
            case 0:
              mod = Qt::KeyboardModifier::ControlModifier;
              break;
            case 1:
              mod = Qt::KeyboardModifier::ShiftModifier;
              break;
            case 2:
              mod = Qt::KeyboardModifier::AltModifier;
              break;
            case 3:
              mod = Qt::KeyboardModifier::MetaModifier;
              break;
            }
            sm->setToolwinMod(mod);
          });
  connect(cbMouseBtn, QOverload<int>::of(&DComboBox::currentIndexChanged), this,
          [=](int index) {
            Qt::MouseButton btn = Qt::MouseButton::NoButton;
            switch (index) {
            case 0:
              btn = Qt::MouseButton::LeftButton;
              break;
            case 1:
              btn = Qt::MouseButton::RightButton;
              break;
            case 2:
              btn = Qt::MouseButton::MidButton;
              break;
            case 3:
              btn = Qt::MouseButton::XButton1;
              break;
            case 4:
              btn = Qt::MouseButton::XButton2;
              break;
            }
            sm->setToolMouseBtn(btn);
          });
}

void CenterWindow::initPluginSys() {
  plgsys = PluginSystem::instance();
  for (auto item : plgsys->plugins()) {
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
  connect(manager, &AppManager::hotkeyReleased, this, [=](const Hotkey *) {});
  connect(manager, &AppManager::hotkeyEnableChanged, this,
          [=](bool value, const Hotkey *hotkey) {
            if (hotkey->isHostHotkey()) {
              tbhotkeys->item(hotkeys.indexOf(const_cast<Hotkey *>(hotkey)), 0)
                  ->setCheckState(value ? Qt::Checked : Qt::Unchecked);
            }
          });
  connect(manager, &AppManager::toolSelTriggered, this,
          [=](int index) { this->runTask(toolinfos[index]); });
}

void CenterWindow::getConfig(QDataStream &f) {
  QVector<int> plgindices;

  // 先保存 Hotkey 的相关信息
  f << hotkeys.count(); // 先存一下有几个
  for (auto &p : scinfos) {
    f << p.enabled << p.isPlugin << p.seq;
    if (p.isPlugin) {
      f << p.serviceID << p.provider.toUtf8() << p.params.toUtf8();
      auto i = plgindices.indexOf(p.pluginIndex);
      if (i >= 0) {
        f << true << i;
      } else {
        f << false << plgsys->pluginHash(p.pluginIndex);
      }
    } else {
      f << p.process.toUtf8()
        << p.params.toUtf8(); // 如果是打开文件就没这么多事情了
    }
  }

  // 下面继续存储 ToolWin 相关信息
  for (auto i = 0; i < 9; i++) {
    auto &p = toolinfos[i];
    // 对于 ToolWin 来说，enabled 是决定性的
    // 只有这个标志位有效，这个工具才有意义
    // 只存有意义的即可
    f << p.enabled;
    if (p.enabled) {
      f << p.isPlugin;
      if (p.isPlugin) {
        f << p.serviceID << p.provider.toUtf8() << p.params.toUtf8();
        auto i = plgindices.indexOf(p.pluginIndex);
        if (i >= 0) {
          f << true << i;
        } else {
          f << false << plgsys->pluginHash(p.pluginIndex);
        }
      } else {
        f << p.process.toUtf8()
          << p.params.toUtf8(); // 如果是打开文件就没这么多事情了
      }
    }
  }

  // 下面存取 WinTool 相关信息
  f << wintoolinfos.count(); // 先存一下有几个
  for (auto &p : wintoolinfos) {
    // 对于 WinTool 来说， enabled 就没用了
    // 只存储相关基础信息就可以了
    f << p.isPlugin;
    if (p.isPlugin) {
      f << p.serviceID << p.provider.toUtf8() << p.params.toUtf8();
      auto i = plgindices.indexOf(p.pluginIndex);
      if (i >= 0) {
        f << true << i;
      } else {
        f << false << plgsys->pluginHash(p.pluginIndex);
      }
    } else {
      f << p.process.toUtf8()
        << p.params.toUtf8(); // 如果是打开文件就没这么多事情了
    }
  }

  // 保存完毕，可以返回了
}

void CenterWindow::resetConfig() {
  ToolStructInfo toolinfo;
  for (auto i = 0; i < 9; i++) {
    toolinfos[i] = toolinfo;
    QIcon icon;
    auto ilbl = lbls[i];
    ilbl->setIcon(icon);
    manager->setToolIcon(i, icon);
  }
  lstoolwin->clear();
  wintoolinfos.clear();
  wintool.rmItem(-1);
}

void CenterWindow::closeEvent(QCloseEvent *event) {
  event->ignore();
  hide();
}

void CenterWindow::showEvent(QShowEvent *event) {
  Q_UNUSED(event);
  sm->saveSettings();
}
