#include "class/appmanager.h"
#include "class/settingmanager.h"
#include "class/wingapplication.h"
#include "dialog/centerwindow.h"
#include "plugin/pluginsystem.h"

#include <DApplication>
#include <DApplicationSettings>
#include <DGuiApplicationHelper>
#include <DWidgetUtil>
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QTranslator>

DWIDGET_USE_NAMESPACE

Q_DECLARE_METATYPE(Qt::MouseButton)

int main(int argc, char *argv[]) {
  //解决 root/ubuntu 主题样式走形
  qputenv("XDG_CURRENT_DESKTOP", "Deepin");
  QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  QApplication::setQuitOnLastWindowClosed(false);
  qRegisterMetaType<Qt::MouseButton>("MouseButton");

  // 程序内强制添加 -platformtheme
  // deepin 参数喂给 Qt 让 Qt 正确使用 Deepin 主题修复各种奇怪样式问题
  QVector<char *> fakeArgs(argc + 2);
  char fa1[] = "-platformtheme";
  char fa2[] = "deepin";
  fakeArgs[0] = argv[0];
  fakeArgs[1] = fa1;
  fakeArgs[2] = fa2;

  for (int i = 1; i < argc; i++)
    fakeArgs[i + 2] = argv[i];
  int fakeArgc = argc + 2;

  QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
  WingApplication a(fakeArgc, fakeArgs.data());

  auto s = a.applicationDirPath() + "/lang/default.qm";
  QTranslator translator;
  if (!translator.load(s)) {
    DMessageBox::critical(nullptr, "Error", "Error Loading Translation File !",
                          DMessageBox::Ok);
    return -1;
  }
  a.installTranslator(&translator);

  a.setOrganizationName("WingCloud");
  a.setApplicationName(QObject::tr("WingTool"));
  a.setApplicationVersion("1.0.0");
  a.setApplicationLicense("AGPL-3.0");

  QIcon picon = ProgramIcon;
  a.setProductIcon(picon);
  a.setProductName(QObject::tr("WingTool"));
  a.setApplicationDescription(
      QObject::tr("A powerful plugin toolbox for Deepin."));

  a.loadTranslator();
  a.setApplicationDisplayName("WingTool");

  if (!a.setSingleInstance("com.Wingsummer.WingTool")) {
    return -1;
  }

  // 保存程序的窗口主题设置
  DApplicationSettings as;
  Q_UNUSED(as)

  CenterWindow w;

  // 单例传参
  auto instance = DGuiApplicationHelper::instance();
  QObject::connect(instance, &DGuiApplicationHelper::newProcessInstance,
                   [&](qint64 pid, const QStringList &arguments) {
                     Q_UNUSED(pid);
                     Q_UNUSED(arguments);
                     w.show();
                     w.activateWindow();
                     w.raise();
                   });

  // 初始化托盘，前半部分
  QSystemTrayIcon systray;
  QMenu sysmenu;
  auto menu = &sysmenu;
  auto ac = new QAction(QObject::tr("ShowMain"), menu);
  QObject::connect(ac, &QAction::triggered, [&w] {
    w.show(CenterWindow::TabPage::General);
    w.activateWindow();
    w.raise();
  });
  sysmenu.addAction(ac);
  sysmenu.addSeparator();

  /*== 以下在主函数初始化确保单例 ==*/
  /* 之后不得使用构造函数的方式使用 */

  // 初始化程序基础驱动
  AppManager manager;
  w.initAppManger();

  // 初始化插件系统
  PluginSystem *plgsys = new PluginSystem(menu);
  w.initPluginSys();

  // 初始化软件配置
  SettingManager sm;

  QObject::connect(&sm, &SettingManager::loadedGeneral, &w,
                   &CenterWindow::initGeneralSettings);
  QObject::connect(&sm, &SettingManager::addHotKeyInfo, &w,
                   &CenterWindow::addHotKeyInfo);
  QObject::connect(&sm, &SettingManager::setToolWinInfo, &w,
                   &CenterWindow::setToolWinInfo);
  QObject::connect(&sm, &SettingManager::setToolFinished, &w,
                   &CenterWindow::setToolFinished);
  QObject::connect(&sm, &SettingManager::addWinToolInfo, &w,
                   &CenterWindow::addWinToolInfo);
  QObject::connect(&sm, &SettingManager::sigSaveConfig, &w,
                   &CenterWindow::getConfig);
  QObject::connect(&sm, &SettingManager::sigReset, &w,
                   &CenterWindow::resetConfig);

  // 载入配置，并进行检查，非法重置
  if (!sm.loadSettings()) {
    dError(QObject::tr("ErrorLoadingSettings"));
    auto path = sm.backupOrignSetting();
    sm.resetSettings();
    sm.saveSettings();
    DMessageBox::critical(nullptr, QObject::tr("Err"),
                          QObject::tr("ErrResetSettings") + '\n' + path);
  }

  QObject::connect(&manager, &AppManager::checkToolShow,
                   [&sm, &manager](Qt::MouseButton btn) {
                     auto mods = manager.getKeyModifiers();
                     return sm.toolwinEnabled() && mods == sm.toolwinMod() &&
                            sm.toolwinMouseBtn() == btn;
                   });

  /*===========================*/

  // 初始化托盘，后半部分
  if (plgsys->hasRegisteredMenu())
    sysmenu.addSeparator();
  ac = new QAction(QObject::tr("About"), menu);
  QObject::connect(ac, &QAction::triggered, [&w] {
    w.show(CenterWindow::TabPage::AboutAuthor);
    w.activateWindow();
    w.raise();
  });
  sysmenu.addAction(ac);
  ac = new QAction(QObject::tr("Wiki"), menu);
  QObject::connect(ac, &QAction::triggered, [=] {
    QDesktopServices::openUrl(QUrl("https://code.gitlink.org.cn/wingsummer/"
                                   "WingTool/wiki/%E7%AE%80%E4%BB%8B"));
  });
  sysmenu.addAction(ac);
  ac = new QAction(QObject::tr("Sponsor"), menu);
  QObject::connect(ac, &QAction::triggered, [&w] {
    w.show(CenterWindow::TabPage::Sponsor);
    w.activateWindow();
    w.raise();
  });
  sysmenu.addAction(ac);
  ac = new QAction(QObject::tr("Exit"), menu);

  sysmenu.addAction(ac);
  systray.setContextMenu(menu);
  systray.setToolTip(QObject::tr("WingTool"));
  systray.setIcon(picon);
  QObject::connect(ac, &QAction::triggered, [&w, &sm, plgsys] {
    if (DMessageBox::question(&w, QObject::tr("Exit"),
                              QObject::tr("ConfirmExit")) == DMessageBox::Yes) {
      sm.saveSettings();
      plgsys->deleteLater();
      QApplication::exit(0);
    }
  });
  QObject::connect(&systray, &QSystemTrayIcon::activated,
                   [&w](QSystemTrayIcon::ActivationReason reason) {
                     if (reason == QSystemTrayIcon::ActivationReason::Trigger) {
                       w.show();
                       w.activateWindow();
                       w.raise();
                     }
                   });
  systray.show();

  Dtk::Widget::moveToCenter(&w);

  return a.exec();
}
