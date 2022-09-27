#include "pluginsystem.h"
#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>

PluginSystem *PluginSystem::m_instance = nullptr;

PluginSystem::PluginSystem(QObject *parent) : QObject(parent) {

  // 初始化类别插件容器

#define InitCatagory(catagory)                                                 \
  m_catplgs.insert(catagory, QList<IWingToolPlg *>());

  InitCatagory(IWingToolPlg::Catagorys::Explor);
  InitCatagory(IWingToolPlg::Catagorys::Develop);
  InitCatagory(IWingToolPlg::Catagorys::Picture);
  InitCatagory(IWingToolPlg::Catagorys::Creative);
  InitCatagory(IWingToolPlg::Catagorys::Searcher);
  InitCatagory(IWingToolPlg::Catagorys::Productivity);

  LoadPlugin();
  m_instance = this;
}

PluginSystem::~PluginSystem() {
  UnloadPlugin();
  DLogManager::registerFileAppender();
  DLogManager::registerConsoleAppender();
}

bool PluginSystem::LoadPlugin() {
  QDir plugindir(QCoreApplication::applicationDirPath() + "/plugin");
  plugindir.setNameFilters(QStringList("*.wingplg"));
  auto plgs = plugindir.entryInfoList();
  for (auto item : plgs) {
    loadPlugin(item);
  }
  return true;
}

void PluginSystem::UnloadPlugin() {}

QList<IWingToolPlg *> PluginSystem::plugins() { return m_plgs; }

void PluginSystem::loadPlugin(QFileInfo fileinfo) {
  LP lp(LP::begin);

  if (fileinfo.exists()) {
    QPluginLoader loader(fileinfo.absoluteFilePath());
    QList<WingPluginInfo> loadedplginfos;
    QList<QVariant> emptyparam;

    try {
      auto p = qobject_cast<IWingToolPlg *>(loader.instance());
      if (p) {
        lp = LP::signature;
        if (p->signature() != WINGSUMMER) {
          dError(tr("ErrLoadPluginSign"));
          loader.unload();
          return;
        }
        lp = LP::sdkVersion;
        if (p->sdkVersion() != SDKVERSION) {
          dError(tr("ErrLoadPluginSDKVersion"));
          loader.unload();
          return;
        }
        lp = LP::pluginName;
        if (!p->pluginName().trimmed().length()) {
          dError(tr("ErrLoadPluginNoName"));
          loader.unload();
          return;
        }

        lp = LP::plugin2MessagePipe;
        emit p->plugin2MessagePipe(HostService,
                                   {LoadingPluginMsg}); // PluginLoading

        if (!p->init(loadedplginfos)) {
          dError(tr("ErrLoadInitPlugin"));
          loader.unload();
          return;
        }

        WingPluginInfo info;
        // info.puid = p->puid();
        info.pluginName = p->pluginName();
        info.pluginAuthor = p->pluginAuthor();
        info.pluginComment = p->pluginComment();
        info.pluginVersion = p->pluginVersion();

        loadedplginfos.push_back(info);
        m_plgs.push_back(p);
        // loadedpuid << puid;

        dWarning(tr("PluginWidgetRegister"));

        // 初始化插件容器
        m_plghk.insert(p, QList<QUuid>());

        connect(p, &IWingToolPlg::registerHotkey, this,
                [=](QKeySequence &keyseq) {
                  auto hk = this->manager->registerHotkey(keyseq);
                  if (hk) {
                    auto uuid = QUuid::createUuid();
                    uhmap.insert(uuid, hk);
                    return uuid;
                  } else {
                    return QUuid();
                  }
                });

        emit p->plugin2MessagePipe(HostService, {LoadedPluginMsg});

      } else {
        dError(loader.errorString());
        loader.unload();
      }
    } catch (...) {
      auto m = QMetaEnum::fromType<LP>();
      dError(QString(tr("ErrLoadPluginLoc") + m.valueToKey(int(lp))));
      loader.unload();
    }
  }
}

PluginSystem *PluginSystem::instance() { return m_instance; }

IWingToolPlg *PluginSystem::plugin(int index) {
  if (index < 0 || index >= m_plgs.count())
    return nullptr;
  return m_plgs[index];
}

QList<QKeySequence> PluginSystem::pluginRegisteredHotkey(IWingToolPlg *plg) {
  if (plg == nullptr)
    return QList<QKeySequence>();

  QList<QKeySequence> keys;
  auto plist = m_plghk[plg];
}
