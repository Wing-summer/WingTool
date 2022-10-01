#include "class/appmanager.h"
#include "class/settingmanager.h"
#include "class/wingapplication.h"
#include "dialog/centerwindow.h"
#include "plugin/pluginsystem.h"

#include <DApplication>
#include <DApplicationSettings>
#include <DGuiApplicationHelper>
#include <DWidgetUtil>
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

  WingApplication a(fakeArgc, fakeArgs.data());
  QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

  //  auto s = a.applicationDirPath() + "/lang/default.qm";
  //  QTranslator translator;
  //  if (!translator.load(s)) {
  //    DMessageBox::critical(nullptr, "Error", "Error Loading Translation
  //    File!",
  //                          DMessageBox::Ok);
  //    return -1;
  //  }
  //  a.installTranslator(&translator);

  a.setOrganizationName("WingCloud");
  a.setApplicationName(QObject::tr("WingTool"));
  a.setApplicationVersion("1.0.0");
  a.setApplicationLicense("AGPL-3.0");

  QIcon picon = ProgramIcon;
  a.setProductIcon(picon);
  a.setProductName(QObject::tr("WingTool"));
  a.setApplicationDescription("This is a dtk template application.");

  a.loadTranslator();
  a.setApplicationDisplayName("WingTool");

  if (!a.setSingleInstance("com.Wingsummer.WingTool")) {
    return -1;
  }

  // 单例传参
  auto instance = DGuiApplicationHelper::instance();
  QObject::connect(instance, &DGuiApplicationHelper::newProcessInstance,
                   [=](qint64 pid, const QStringList &arguments) {
                     Q_UNUSED(pid);
                     Q_UNUSED(arguments);
                   });

  // 保存程序的窗口主题设置
  DApplicationSettings as;
  Q_UNUSED(as)

  CenterWindow w;

  /*== 以下在主函数初始化确保单例 ==*/
  /* 之后不得使用构造函数的方式使用 */

  // 初始化程序基础驱动
  AppManager manager;
  w.initAppManger();

  // 初始化插件系统
  PluginSystem plgsys;
  w.initPluginSys();

  // 初始化软件配置
  SettingManager sm;

  QObject::connect(&sm, &SettingManager::loadedGeneral, &w,
                   &CenterWindow::initGeneralSettings);
  QObject::connect(&sm, &SettingManager::addHotKeyInfo, &w,
                   &CenterWindow::addHotKeyInfo);
  QObject::connect(&sm, &SettingManager::setToolWinInfo, &w,
                   &CenterWindow::setoolWinInfo);
  QObject::connect(&sm, &SettingManager::addWinToolInfo, &w,
                   &CenterWindow::addWinToolInfo);
  QObject::connect(&sm, &SettingManager::sigSaveConfig, &w,
                   &CenterWindow::getConfig);
  sm.loadSettings();

  QObject::connect(&manager, &AppManager::checkToolShow,
                   [&sm, &manager](Qt::MouseButton btn) {
                     auto mod = manager.getKeyModifier();
                     return sm.toolwinEnabled() && mod == sm.toolwinMod() &&
                            sm.toolwinMouseBtn() == btn;
                   });

  /*===========================*/

  // 初始化托盘
  QSystemTrayIcon systray;
  QMenu sysmenu;
  auto menu = &sysmenu;
  auto ac = new QAction(QObject::tr("ShowMain"), menu);
  QObject::connect(ac, &QAction::triggered,
                   [&w] { w.show(CenterWindow::TabPage::General); });
  sysmenu.addAction(ac);
  sysmenu.addSeparator();
  ac = new QAction(QObject::tr("About"), menu);
  QObject::connect(ac, &QAction::triggered,
                   [&w] { w.show(CenterWindow::TabPage::AboutAuthor); });
  sysmenu.addAction(ac);
  ac = new QAction(QObject::tr("Sponsor"), menu);
  QObject::connect(ac, &QAction::triggered,
                   [&w] { w.show(CenterWindow::TabPage::Sponsor); });
  sysmenu.addAction(ac);
  ac = new QAction(QObject::tr("Exit"), menu);
  QObject::connect(ac, &QAction::triggered, [] { QApplication::exit(0); });
  sysmenu.addAction(ac);
  systray.setContextMenu(menu);
  systray.setIcon(picon);
  systray.show();

  QObject::connect(&systray, &QSystemTrayIcon::activated,
                   [&w](QSystemTrayIcon::ActivationReason reason) {
                     if (reason == QSystemTrayIcon::ActivationReason::Trigger) {
                       w.show(CenterWindow::TabPage::General);
                       Utilities::activeWindowFromDock(w.winId());
                     }
                   });

  Dtk::Widget::moveToCenter(&w);

  return a.exec();
}
