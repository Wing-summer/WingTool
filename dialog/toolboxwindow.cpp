#include "toolboxwindow.h"
#include "plugin/pluginsystem.h"
#include <DLabel>
#include <DThemeManager>
#include <DTitlebar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

ToolBoxWindow::ToolBoxWindow(DMainWindow *parent) : DDialog(parent) {
  setWindowTitle(tr("PleaseChoose"));
  setWindowFlag(Qt::Tool);
  setWindowFlag(Qt::WindowStaysOnTopHint);
  setCloseButtonVisible(false);
  lstool = new DListWidget(this);
  lstool->setMouseTracking(true);
  auto pe = lstool->palette();
  pe.setColor(QPalette::AlternateBase,
              DGuiApplicationHelper::instance()->themeType() ==
                      DGuiApplicationHelper::LightType
                  ? pe.base().color().darker(125)
                  : pe.base().color().lighter(125));
  lstool->setPalette(pe);
  lstool->setAlternatingRowColors(true);
  itemheight = lstool->fontMetrics().lineSpacing() * 4;
  lstool->setMinimumHeight(itemheight * 5);
  addContent(lstool);

  connect(DGuiApplicationHelper::instance(),
          &DGuiApplicationHelper::themeTypeChanged, this,
          &ToolBoxWindow::setTheme);

  connect(lstool, &DListWidget::clicked, this, [=](const QModelIndex &index) {
    emit this->sigRun(index.row());
    lstool->clearSelection();
    this->hide();
  });

  connect(lstool, &DListWidget::itemEntered, this,
          [=](QListWidgetItem *item) { item->setSelected(true); });
}

void ToolBoxWindow::addItem(ToolStructInfo &info, QString service, int index) {
  auto plgsys = PluginSystem::instance();
  auto plg = plgsys->plugin(info.pluginIndex);
  auto icon = Utilities::trimIconFromInfo(plg, info);
  QString content =
      tr("Process:") +
      (info.isPlugin ? info.process : QFileInfo(info.process).fileName()) +
      '\n';

  if (info.isPlugin)
    content += (tr("Service:") + service);
  if (info.params.length())
    content += ('\n' + tr("Params:") + info.params);

  auto pitem = new QListWidgetItem(icon, content);
  pitem->setTextAlignment(Qt::AlignCenter);
  pitem->setSizeHint(QSize(0, itemheight));

  if (index < 0)
    lstool->addItem(pitem);
  else
    lstool->insertItem(index, pitem);
}

void ToolBoxWindow::rmItem(int index) {
  if (index < 0) {
    lstool->clear();
    return;
  }

  if (index >= lstool->count()) {
    return;
  }

  auto k = lstool->takeItem(index);
  delete k;
}

void ToolBoxWindow::mvItem(int from, int to) {
  auto len = lstool->count();
  if (from < 0 || from >= len || to < 0 || to >= len)
    return;

  auto item = lstool->takeItem(from);
  lstool->insertItem(to, item);
}

void ToolBoxWindow::setTheme(DGuiApplicationHelper::ColorType theme) {
  auto pe = lstool->palette();
  pe.setColor(QPalette::AlternateBase, theme == DGuiApplicationHelper::LightType
                                           ? pe.base().color().darker(125)
                                           : pe.base().color().lighter(125));
  lstool->setPalette(pe);
}

void ToolBoxWindow::leaveEvent(QEvent *e) {
  Q_UNUSED(e);
  hide();
}

void ToolBoxWindow::focusOutEvent(QFocusEvent *event) {
  Q_UNUSED(event);
  hide();
}
