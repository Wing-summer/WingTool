#include "testplugin.h"
#include <QMessageBox>

TestPlugin::TestPlugin(QObject *parent) {
  Q_UNUSED(parent);
  qRegisterMetaType<TestService>("TestService");
}

int TestPlugin::sdkVersion() { return SDKVERSION; }

QString TestPlugin::signature() { return WINGSUMMER; }

TestPlugin::~TestPlugin() { testmenu->deleteLater(); }

bool TestPlugin::preInit() {
  dialog = new QDialog;
  dialog->setFixedSize(400, 400);
  dialog->setWindowTitle("TestPluginConsole");
  dialog->setWindowFlag(Qt::Tool);
  dialog->setWindowFlags(Qt::CustomizeWindowHint |
                         Qt::WindowMinimizeButtonHint |
                         Qt::WindowMaximizeButtonHint);
  tbinfo = new QTextBrowser(dialog);
  tbinfo->setFixedSize(dialog->size());
  tbinfo->setUndoRedoEnabled(false);
  services = new TestService(tbinfo, dialog);

  testmenu = new QAction;
  testmenu->setIcon(QIcon(":/TestPlugin/logo.svg"));
  testmenu->setText("TestMenu");
  return true;
}

bool TestPlugin::init(QList<WingPluginInfo> loadedplugin) {
  Q_UNUSED(loadedplugin);
  dialog->show();
  return true;
}

void TestPlugin::unload() {
  dialog->close();
  delete dialog;
  testmenu->deleteLater();
}

QString TestPlugin::pluginName() { return "TestPlugin"; }

QString TestPlugin::pluginAuthor() { return WINGSUMMER; }

IWingToolPlg::Catagorys TestPlugin::pluginCatagory() {
  return IWingToolPlg::Catagorys::Explor;
}

uint TestPlugin::pluginVersion() { return 1; }

QString TestPlugin::pluginComment() { return "This is a test plugin !"; }

QString TestPlugin::pluginWebsite() {
  return "https://www.cnblogs.com/wingsummer/";
}

QIcon TestPlugin::pluginIcon() { return QIcon(":/TestPlugin/logo.svg"); }

const QPointer<QObject> TestPlugin::serviceHandler() {
  return QPointer<QObject>(services);
}

const QMetaObject *TestPlugin::serviceMeta() { return services->metaObject(); }

HookIndex TestPlugin::getHookSubscribe() { return HookIndex::None; }

QObject *TestPlugin::trayRegisteredMenu() { return testmenu; }

QString TestPlugin::translatorFile() { return "TestPlugin.qm"; }

QVariant TestPlugin::pluginServicePipe(int serviceID, QList<QVariant> params) {
  Q_UNUSED(params);
  switch (serviceID) {
  case PLUGINLOADING:
    break;
  case PLUGINLOADED: {
    testhotkey = registerHotkey(
        QKeySequence(Qt::KeyboardModifier::ControlModifier |
                     Qt::KeyboardModifier::AltModifier | Qt::Key_Q));
    if (testhotkey.isNull()) {
      tbinfo->append(QString("registerHotkey Error!"));
    }
  } break;
  default:
    tbinfo->append(QString("GetMessage : %1").arg(serviceID));
    break;
  }
  return QVariant();
}

void TestPlugin::onPluginCenter() {
  QMessageBox::information(nullptr, "Settings", "You Clicked Settings!");
}

void TestPlugin::hotkeyTirggered(QUuid id) {
  tbinfo->append(QString("HotKeyTriggered : %1").arg(id.toString()));
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(TestPlugin, IWingToolPlg)
#endif // QT_VERSION < 0x050000
