#include "testplugin.h"

TestPlugin::TestPlugin(QObject *parent) { Q_UNUSED(parent) }

int TestPlugin::sdkVersion() { return SDKVERSION; }

QString TestPlugin::signature() { return WINGSUMMER; }

TestPlugin::~TestPlugin() {}

bool TestPlugin::init(QList<WingPluginInfo> loadedplugin) {
  Q_UNUSED(loadedplugin);
  dialog = new QDialog;
  dialog->setFixedSize(400, 400);
  dialog->setWindowTitle("TestPluginConsole");
  dialog->setWindowFlags(Qt::CustomizeWindowHint |
                         Qt::WindowMinimizeButtonHint |
                         Qt::WindowMaximizeButtonHint);
  tbinfo = new QTextBrowser(dialog);
  tbinfo->setFixedSize(dialog->size());
  tbinfo->setUndoRedoEnabled(false);
  dialog->show();
  return true;
}

void TestPlugin::unload() {
  dialog->close();
  delete dialog;
}

QString TestPlugin::pluginName() { return "TestPlugin"; }

QString TestPlugin::provider() { return "testpro"; }

QString TestPlugin::pluginAuthor() { return WINGSUMMER; }

IWingToolPlg::Catagorys TestPlugin::pluginCatagory() {
  return IWingToolPlg::Catagorys::Explor;
}

uint TestPlugin::pluginVersion() { return 1; }

QString TestPlugin::pluginComment() { return "This is a test plugin !"; }

QIcon TestPlugin::pluginIcon() { return QIcon(":/TestPlugin/logo.svg"); }

QStringList TestPlugin::pluginServices() { return {"func1", "func2", "func3"}; }

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
    break;
  }
  return QVariant();
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(TestPlugin, IWingToolPlg)
#endif // QT_VERSION < 0x050000
