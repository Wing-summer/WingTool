#include "class/appmanager.h"
#include "class/wingapplication.h"
#include "dialog/centerwindow.h"

#include <DApplication>
#include <DApplicationSettings>
#include <DGuiApplicationHelper>
#include <DWidgetUtil>
#include <QMenu>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QTranslator>

DWIDGET_USE_NAMESPACE

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

  QIcon picon(":/images/logo.svg");
  a.setProductIcon(picon);
  a.setProductName(QObject::tr("WingHexExplorer"));
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

  // 初始化程序基础驱动
  AppManager manger;

  CenterWindow w;

  // 初始化托盘
  QSystemTrayIcon systray;
  QMenu sysmenu;
  systray.setContextMenu(&sysmenu);
  systray.setIcon(picon);
  systray.show();

  QObject::connect(&systray, &QSystemTrayIcon::activated,
                   [&w](QSystemTrayIcon::ActivationReason reason) {
                     if (reason == QSystemTrayIcon::ActivationReason::Trigger)
                       w.show();
                   });

  Dtk::Widget::moveToCenter(&w);

  return a.exec();
}
