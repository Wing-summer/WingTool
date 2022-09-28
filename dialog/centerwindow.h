#ifndef CENTERWINDOW_H
#define CENTERWINDOW_H

#include "class/appmanager.h"
#include "class/settingmanager.h"
#include "plugin/pluginsystem.h"
#include "utilities.h"
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
  enum class TabPage {
    General,
    Hotkeys,
    ToolBox,
    Plugins,
    AboutAuthor,
    Sponsor
  };

  Q_ENUM(TabPage)

public:
  CenterWindow(DMainWindow *parent = nullptr);

  void show(TabPage index);

private:
  QStringList parseCmdParams(QString str);
  bool runTask(ToolStructInfo record);
  void editTask(int index);

  void on_editHotkey();
  void on_removeHotkey();
  void on_clearHotkey();
  void on_addHotkey();
  void enableSelectedHotkeys(bool enable);

  void on_editToolWin();
  void on_removeToolWin();
  void on_clearToolWin();
  void on_addToolWin();
  void on_upToolWin();
  void on_downToolWin();

public slots:
  void getHokeysBuffer(QList<QHotkey *> &hotkeysBuf,
                       QMap<QHotkey *, ToolStructInfo> &buffer);
  void getToolLeftBuffer(ToolStructInfo buffer[]);
  void getToolRightBuffer(QList<ToolStructInfo> &buffer);
  void loadingFinish();

  void initPluginSys();
  void initAppManger();

protected:
  void closeEvent(QCloseEvent *event) override;

private:
  QList<QAction *> hkcmenu;

private:
  AppManager *manager;
  SettingManager *sm;
  PluginSystem *plgsys;

  DTabWidget *tabs;

  DTableWidget *tbhotkeys;
  DMenu *tbmenu;

  DListWidget *lwplgs, *lstoolwin;
  DTextBrowser *tbplginfo, *tbtoolinfo;

  DCheckBox *cbauto; // 开机自启动

  QProcess pstart;

  DIconButton *lbls[9] = {nullptr};

private:
  QMap<QHotkey *, ToolStructInfo> scinfos; // 用于 Hotkeys
  ToolStructInfo toolinfos[9];             // 用于 Tool 左侧
  QList<ToolStructInfo> wintoolinfos;      // 用于 WinTool（ Tool 右侧 ）
  QList<QHotkey *> hotkeys;
};

#endif // CENTERWINDOW_H
