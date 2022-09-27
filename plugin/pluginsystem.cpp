#include "pluginsystem.h"
#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>

PluginSystem *PluginSystem::m_instance = nullptr;

PluginSystem::PluginSystem(QObject *parent)
    : QObject(parent), manager(AppManager::instance()) {

  // init plugin dispathcer
#define InitDispathcer(hookindex)                                              \
  dispatcher.insert(hookindex, QList<IWingToolPlg *>());

  InitDispathcer(HookIndex::Clicked);
  InitDispathcer(HookIndex::MouseMove);
  InitDispathcer(HookIndex::MouseWheel);
  InitDispathcer(HookIndex::DoubleClicked);
  InitDispathcer(HookIndex::MouseDrag);

  // 初始化类别插件容器
#define InitCatagory(catagory)                                                 \
  m_catplgs.insert(catagory, QList<IWingToolPlg *>());

  InitCatagory(IWingToolPlg::Catagorys::Explor);
  InitCatagory(IWingToolPlg::Catagorys::Develop);
  InitCatagory(IWingToolPlg::Catagorys::Picture);
  InitCatagory(IWingToolPlg::Catagorys::Creative);
  InitCatagory(IWingToolPlg::Catagorys::Searcher);
  InitCatagory(IWingToolPlg::Catagorys::Productivity);

  // 初始化插件基础服务
  connect(manager, &AppManager::buttonPress, this,
          [=](EventMonitor::MouseButton btn, int x, int y) {
            for (auto item : dispatcher[HookIndex::ButtonPress]) {
              item->buttonPress(MouseButtonEvent(btn), x, y);
            }
          });
  connect(manager, &AppManager::buttonRelease, this,
          [=](EventMonitor::MouseButton btn, int x, int y) {
            for (auto item : dispatcher[HookIndex::ButtonRelease]) {
              item->buttonRelease(MouseButtonEvent(btn), x, y);
            }
          });
  connect(manager, &AppManager::mouseMove, this, [=](int x, int y) {
    for (auto item : dispatcher[HookIndex::MouseMove]) {
      item->mouseMove(x, y);
    }
  });
  connect(manager, &AppManager::mouseDrag, this, [=](int x, int y) {
    for (auto item : dispatcher[HookIndex::MouseDrag]) {
      item->mouseDrag(x, y);
    }
  });
  connect(manager, &AppManager::mouseWheel, this,
          [=](EventMonitor::MouseWheel direction) {
            for (auto item : dispatcher[HookIndex::MouseWheel]) {
              item->mouseWheel(MouseWheelEvent(direction));
            }
          });
  connect(manager, &AppManager::clicked, this, [=](int x, int y) {
    for (auto item : dispatcher[HookIndex::Clicked]) {
      item->clicked(x, y);
    }
  });
  connect(manager, &AppManager::doubleClicked, this, [=](int x, int y) {
    for (auto item : dispatcher[HookIndex::DoubleClicked]) {
      item->doubleClicked(x, y);
    }
  });
  connect(manager, &AppManager::hotkeyTirggered, this,
          [=](const QHotkey *hotkey) {

          });

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

        lp = LP::provider;
        if (p->provider().isEmpty() || loadedProvider.contains(p->provider())) {
          dError(tr("ErLoadPluginProvider"));
          loader.unload();
          return;
        }

        lp = LP::service;
        // 插件至少含有一种服务
        auto service = p->pluginServices();
        if (service.isEmpty()) {
          dError(tr("ErLoadPluginService"));
          loader.unload();
          return;
        }
        // 禁止含有相同标识的服务名
        for (auto &s : service) {
          auto c = service.count(s);
          if (c != 1) {
            dError(tr("ErLoadPluginService"));
            loader.unload();
            return;
          }
        }

        // 检查完毕后，就可以进入真正的加载环节

        lp = LP::plugin2MessagePipe;
        emit p->pluginServicePipe(HostService, {LoadingPluginMsg});

        if (!p->init(loadedplginfos)) {
          dError(tr("ErrLoadInitPlugin"));
          loader.unload();
          return;
        }

        WingPluginInfo info;
        info.provider = p->provider();
        info.pluginName = p->pluginName();
        info.pluginAuthor = p->pluginAuthor();
        info.pluginComment = p->pluginComment();
        info.pluginVersion = p->pluginVersion();

        loadedplginfos.push_back(info);
        m_plgs.push_back(p);
        loadedProvider << p->provider();

        dWarning(tr("PluginInitRegister"));

        // 初始化插件容器
        m_plghk.insert(p, QList<QUuid>());

        // 查询订阅
        auto sub = p->getHookSubscribe();

#define INSERTSUBSCRIBE(HOOK)                                                  \
  if (sub & HOOK)                                                              \
    dispatcher[HOOK].push_back(p);

        INSERTSUBSCRIBE(HookIndex::Clicked);
        INSERTSUBSCRIBE(HookIndex::MouseMove);
        INSERTSUBSCRIBE(HookIndex::MouseWheel);
        INSERTSUBSCRIBE(HookIndex::DoubleClicked);
        INSERTSUBSCRIBE(HookIndex::MouseDrag);

        // 连接信号
        connect(p, &IWingToolPlg::registerHotkey, this,
                [=](QKeySequence &keyseq) {
                  auto sender = qobject_cast<IWingToolPlg *>(QObject::sender());
                  if (sender == nullptr)
                    return QUuid();
                  auto hk = this->manager->registerHotkey(keyseq);
                  if (hk) {
                    auto uuid = QUuid::createUuid();
                    m_plghk[sender].append(uuid);
                    uhmap.insert(uuid, hk);
                    return uuid;
                  } else {
                    return QUuid();
                  }
                });
        connect(p, &IWingToolPlg::unregisterHotkey, this, [=](const QUuid id) {
          auto sender = qobject_cast<IWingToolPlg *>(QObject::sender());
          if (sender == nullptr)
            return false;
          auto &plist = m_plghk[sender];
          auto i = plist.indexOf(id);
          if (i >= 0) {
            plist.removeAt(i);
            uhmap.remove(id);
            return this->manager->unregisterHotkey(uhmap[id]);
          } else {
            return false;
          }
        });
        connect(p, &IWingToolPlg::enableHotKey, this,
                [=](const QUuid id, bool enabled) {
                  auto sender = qobject_cast<IWingToolPlg *>(QObject::sender());
                  if (sender == nullptr)
                    return false;
                  auto &plist = m_plghk[sender];
                  if (plist.contains(id)) {
                    return this->manager->enableHotKey(uhmap[id], enabled);
                  }
                  return false;
                });
        connect(p, &IWingToolPlg::editHotkey, this,
                [=](const QUuid id, QKeySequence &seq) {
                  auto sender = qobject_cast<IWingToolPlg *>(QObject::sender());
                  if (sender == nullptr)
                    return false;
                  auto &plist = m_plghk[sender];
                  if (plist.contains(id)) {
                    return this->manager->editHotkey(uhmap[id], seq);
                  }
                  return false;
                });
        connect(p, &IWingToolPlg::remoteCall, this,
                [=](const QByteArray provider, const QByteArray callback,
                    QList<QVariant> params) {
                  auto sender = qobject_cast<IWingToolPlg *>(QObject::sender());
                  if (sender == nullptr)
                    return RemoteCallError::Unkown;

                  auto index = loadedProvider.indexOf(provider);
                  if (index < 0)
                    return RemoteCallError::PluginNotFound;

                  auto plg = m_plgs[index];
                  auto id = plg->pluginServices().indexOf(callback);
                  if (id < 0)
                    return RemoteCallError::ServiceNotFound;

                  try {
                    // 调用获取返回值
                    auto res = emit plg->pluginServicePipe(id, params);
                    // 将返回值以相同方式传送回去
                    emit sender->pluginServicePipe(RemoteCallRes, {res});
                    return RemoteCallError::Success;
                  } catch (...) {
                    return RemoteCallError::Unkown;
                  }
                });

        emit p->pluginServicePipe(HostService, {LoadedPluginMsg});

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
  for (auto &item : plist) {
    auto hk = uhmap[item];
    keys.append(hk->shortcut());
  }
  return keys;
}
