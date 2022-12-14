#ifndef CENTERWINDOW_H
#define CENTERWINDOW_H

#include "class/appmanager.h"
#include "class/settingmanager.h"
#include "plugin/pluginsystem.h"
#include "toolboxwindow.h"
#include "utilities.h"
#include <DCheckBox>
#include <DComboBox>
#include <DDialog>
#include <DKeySequenceEdit>
#include <DListWidget>
#include <DMainWindow>
#include <DMenu>
#include <DSpinBox>
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
    None = -1,
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

  void show(TabPage index = TabPage::None);

private:
  bool runTask(ToolStructInfo record);
  void editTask(int index);

  void on_editHotkey();
  void on_removeHotkey();
  void on_clearHotkey();
  void on_addHotkey();
  void enableSelectedHotkeys(bool enable);

  void on_editWinTool();
  void on_removeWinTool();
  void on_clearWinTool();
  void on_addWinTool();
  void on_upWinTool();
  void on_downWinTool();

  void on_exportSettings();
  void on_importSettings();
  void on_resetSettings();
  void on_runplg();

public slots:
  void addHotKeyInfo(ToolStructInfo &info);
  void setToolWinInfo(int index, ToolStructInfo &info);
  void setToolFinished();
  void addWinToolInfo(ToolStructInfo &info);

  void initGeneralSettings();
  void initPluginSys();
  void initAppManger();
  void getConfig(QDataStream &f);
  void resetConfig();

protected:
  void closeEvent(QCloseEvent *event) override;
  void showEvent(QShowEvent *event) override;

private:
  QList<QAction *> hkcmenu, lscmenu;

private:
  AppManager *manager;
  SettingManager *sm;
  PluginSystem *plgsys;

  DTabWidget *tabs;

  DTableWidget *tbhotkeys;
  DMenu *tbmenu;

  DListWidget *lwplgs, *lstoolwin;
  DTextBrowser *tbplginfo, *tbtoolinfo;

  DCheckBox *cbToolWin, *cbWinTool;

  QProcess pstart;

  DIconButton *lbls[9] = {nullptr};
  int sellbl = 0;

  DKeySequenceEdit *kseqTool;
  DKeySequenceEdit *kseqRun;
  DComboBox *cbMod, *cbMouseBtn;

  DSpinBox *sbGridsize;

  Hotkey *hkwintool;
  Hotkey *hkrunwin;
  ToolBoxWindow wintool;

private:
  QMap<Hotkey *, ToolStructInfo> scinfos; // ?????? Hotkeys
  QList<Hotkey *> hotkeys;                // Hotkeys ??????????????????
  ToolStructInfo toolinfos[9]; // ?????? Tool ??????????????? 4 ??????????????????
  QList<ToolStructInfo> wintoolinfos; // ?????? WinTool??? Tool ?????? ???
};

#endif // CENTERWINDOW_H
