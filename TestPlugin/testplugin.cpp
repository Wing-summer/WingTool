#include "testplugin.h"
#include <QMessageBox>

TestPlugin::TestPlugin(QObject *parent) {
  Q_UNUSED(parent);
  qRegisterMetaType<TestService>("TestService");

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
}

int TestPlugin::sdkVersion() { return SDKVERSION; }

QString TestPlugin::signature() { return WINGSUMMER; }

TestPlugin::~TestPlugin() {}

bool TestPlugin::init(QList<WingPluginInfo> loadedplugin) {
  Q_UNUSED(loadedplugin);
  dialog->show();
  auto s = GETPLUGINQM("TestPlugin.qm");
  if (!translator.load(s) || !QApplication::installTranslator(&translator)) {
    QMessageBox::critical(nullptr, "Error", "Error Loading File!",
                          QMessageBox::Ok);
    return false;
  }
  return true;
}

void TestPlugin::unload() {
  dialog->close();
  delete dialog;
}

QString TestPlugin::pluginName() { return "TestPlugin"; }

QString TestPlugin::pluginAuthor() { return WINGSUMMER; }

IWingToolPlg::Catagorys TestPlugin::pluginCatagory() {
  return IWingToolPlg::Catagorys::Explor;
}

uint TestPlugin::pluginVersion() { return 1; }

QString TestPlugin::pluginComment() { return "This is a test plugin !"; }

QIcon TestPlugin::pluginIcon() { return QIcon(":/TestPlugin/logo.svg"); }

const QPointer<QObject> TestPlugin::serviceHandler() {
  return QPointer<QObject>(services);
}

const QMetaObject *TestPlugin::serviceMeta() { return services->metaObject(); }

HookIndex TestPlugin::getHookSubscribe() { return HookIndex::None; }

QVariant TestPlugin::pluginServicePipe(int serviceID, QList<QVariant> params) {
  switch (serviceID) {
  case HostService:
    if (params.first() == LoadedPluginMsg) {
      testhotkey = registerHotkey(
          QKeySequence(Qt::KeyboardModifier::ControlModifier |
                       Qt::KeyboardModifier::AltModifier | Qt::Key_Q));
      if (testhotkey.isNull()) {
        tbinfo->append(QString("registerHotkey Error!"));
      }
    }
    break;
  case RemoteCallRes:
    break;
  case HotKeyTriggered:
    tbinfo->append(QString("HotKeyTriggered : %1")
                       .arg(params.first().value<QUuid>().toString()));
    break;
  case 0:
    if (params.count()) {
      auto param = params.first();
      if (param.canConvert(QMetaType::Int)) {
        tbinfo->append(QString("[func1 call] : %1").arg(param.value<int>()));
      }
    }
    break;
  case 1:
    if (params.count()) {
      QStringList res;
      for (auto &item : params) {
        if (item.canConvert(QMetaType::QString)) {
          res.append(item.value<QString>());
        } else if (item.canConvert(QMetaType::QStringList)) {
          res += item.value<QStringList>();
        }
      }
      tbinfo->append(QString("[func2 call] : ") + res.join(';'));
    }
    break;
  case 2:
    dialog->setVisible(!dialog->isVisible());
    break;
  }
  return QVariant();
}

void TestPlugin::onSetting() {
  QMessageBox::information(nullptr, "Settings", "You Clicked Settings!");
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(TestPlugin, IWingToolPlg)
#endif // QT_VERSION < 0x050000
