#include "centerwindow.h"
#include "shortcuteditdialog.h"
#include <DButtonBox>
#include <DLabel>
#include <DMessageBox>
#include <DMessageManager>
#include <DTextBrowser>
#include <DTitlebar>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMimeDatabase>
#include <QProcess>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QVector>

CenterWindow::CenterWindow(DMainWindow *parent)
    : DMainWindow(parent), manager(AppManager::instance()) {
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
  auto vlayout = new QVBoxLayout(w);
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
    scinfos[row].enabled = b;
    manager->enableHotKey(row, b);
  });
  connect(tbhotkeys, &DTableWidget::cellDoubleClicked, this,
          [=](int row, int) { this->editTask(row); });

  vlayout->addWidget(tbhotkeys);
  tabs->addTab(w, tr("Hotkeys"));

  // ToolBox
  w = new QWidget(this);

  tabs->addTab(w, tr("ToolBox"));

  // Plugins
  w = new QWidget(this);
  auto playout = new QHBoxLayout(w);
  lwplgs = new DListWidget(w);
  playout->addWidget(lwplgs);
  tbplginfo = new DTextBrowser(w);
  playout->addWidget(tbplginfo);
  tabs->addTab(w, tr("Plugins"));

  // AboutAuthor
  w = new QWidget(this);
  auto alayout = new QVBoxLayout(w);
  auto l = new DLabel(this);
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

  //初始化热键事件处理函数
  QObject::connect(manager, &AppManager::hotkeyTirggered, this,
                   [=](const QHotkey *, int index) {
                     auto &task = scinfos[index];
                     this->runTask(task.process, task.params);
                   });
  QObject::connect(manager, &AppManager::hotkeyReleased, this,
                   [=](const QHotkey *, int) {

                   });
  QObject::connect(manager, &AppManager::hotkeyEnableChanged, this,
                   [=](bool value, const QHotkey *, int index) {
                     tbhotkeys->item(index, 0)->setCheckState(
                         value ? Qt::Checked : Qt::Unchecked);
                   });
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

bool CenterWindow::runTask(QString program, QString param) {
  QMimeDatabase db;

  QFileInfo info(program);
  auto absp = info.absoluteFilePath();

  auto mt = db.mimeTypeForFile(absp);
  auto n = mt.name();
  if (n == "application/x-executable") {
    if (!pstart.startDetached(absp, parseCmdParams(param))) {
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
  auto &task = scinfos[index];
  ShortCutEditDialog d(task.enabled, task.seq, task.process, task.params);
  if (d.exec()) {
    auto res = d.getResult();
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
      scinfos.removeAt(item);
      manager->unregisterHotkey(item);
      tbhotkeys->removeRow(item);
    }
  } else {
    auto row = tbhotkeys->currentRow();
    scinfos.removeAt(row);
    manager->unregisterHotkey(row);
    tbhotkeys->removeRow(row);
  }
}

void CenterWindow::on_clearHotkey() {
  scinfos.clear();
  manager->clearHotkey();
  tbhotkeys->setRowCount(0);
  DMessageManager::instance()->sendMessage(this, ProgramIcon,
                                           tr("ClearSuccess"));
}

void CenterWindow::on_addHotkey() {
  ShortCutEditDialog d;
  if (d.exec()) {
    auto res = d.getResult();
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

    scinfos.append(res);
    manager->registerHotkey(res.seq);
  }
}

void CenterWindow::enableSelectedHotkeys(bool enable) {
  auto selrows = tbhotkeys->selectionModel()->selectedRows();
  for (auto &item : selrows) {
    manager->enableHotKey(item.row(), enable);
  }
}
