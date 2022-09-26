#ifndef CENTERWINDOW_H
#define CENTERWINDOW_H

#include "class/appmanager.h"
#include "utilies.h"
#include <DCheckBox>
#include <DDialog>
#include <DListWidget>
#include <DMainWindow>
#include <DMenu>
#include <DTabWidget>
#include <DTableWidget>
#include <DTextBrowser>
#include <QObject>
#include <QProcess>

DWIDGET_USE_NAMESPACE

class CenterWindow : public DMainWindow {
  Q_OBJECT
public:
  CenterWindow(DMainWindow *parent = nullptr);

private:
  QStringList parseCmdParams(QString str);
  bool runTask(QString program, QString param);
  void editTask(int index);

  void on_editHotkey();
  void on_removeHotkey();
  void on_clearHotkey();
  void on_addHotkey();
  void enableSelectedHotkeys(bool enable);

private:
  QList<QAction *> hkcmenu;

private:
  AppManager *manager;

  DTabWidget *tabs;

  DTableWidget *tbhotkeys;
  DMenu *tbmenu;

  DListWidget *lwplgs;
  DTextBrowser *tbplginfo;

  DCheckBox *cbauto; // 开机自启动

  QProcess pstart;

private:
  QList<ShortCutEditRes> scinfos;
};

#endif // CENTERWINDOW_H
