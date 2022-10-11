#include "pluginsystem.h"
#include "utilities.h"
#include <QCoreApplication>
#include <QDir>
#include <QMetaClassInfo>
#include <QPluginLoader>

PluginSystem *PluginSystem::m_instance = nullptr;

PluginSystem::PluginSystem(QMenu *systray, QObject *parent)
    : QObject(parent), manager(AppManager::instance()), traymenu(systray) {

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
          [=](Qt::MouseButton btn, int x, int y) {
            for (auto item : dispatcher[HookIndex::ButtonPress]) {
              item->buttonPress(btn, x, y);
            }
          });
  connect(manager, &AppManager::buttonRelease, this,
          [=](Qt::MouseButton btn, int x, int y) {
            for (auto item : dispatcher[HookIndex::ButtonRelease]) {
              item->buttonRelease(btn, x, y);
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
          [=](const Hotkey *hotkey) {
            if (hotkey->isHostHotkey())
              return;

            auto uuid = uhmap.key(const_cast<Hotkey *>(hotkey), QUuid());
            if (uuid.isNull())
              return;
            int id;
            auto plg = this->loopUpHotkey(uuid, id);
            if (plg)
              plg->pluginServicePipe(HotKeyTriggered, {uuid});
          });
  connect(manager, &AppManager::hotkeyReleased, this,
          [=](const Hotkey *hotkey) {
            if (hotkey->isHostHotkey())
              return;

            auto uuid = uhmap.key(const_cast<Hotkey *>(hotkey), QUuid());
            if (uuid.isNull())
              return;
            int id;
            auto plg = this->loopUpHotkey(uuid, id);
            if (plg)
              plg->pluginServicePipe(HotKeyReleased, {uuid});
          });
  connect(manager, &AppManager::hotkeyEnableChanged, this,
          [=](bool value, const Hotkey *hotkey) {
            if (hotkey->isHostHotkey())
              return;

            auto uuid = uhmap.key(const_cast<Hotkey *>(hotkey), QUuid());
            if (uuid.isNull())
              return;
            int id;
            auto plg = this->loopUpHotkey(uuid, id);
            if (plg)
              plg->pluginServicePipe(HotkeyEnableChanged, {value, uuid});
          });

  LoadPlugin();
  m_instance = this;
}

PluginSystem::~PluginSystem() { UnloadPlugin(); }

bool PluginSystem::LoadPlugin() {
  QDir plugindir(QCoreApplication::applicationDirPath() + "/plugin");
#ifdef QT_DEBUG
  plugindir.setNameFilters(QStringList("*.so"));
#else
  plugindir.setNameFilters(QStringList("*.wingplg"));
#endif
  auto plgs = plugindir.entryInfoList(QDir::Files);
  for (auto item : plgs) {
    loadPlugin(item);
  }
  return true;
}

void PluginSystem::UnloadPlugin() {}

QList<IWingToolPlg *> PluginSystem::plugins() { return m_plgs; }

void PluginSystem::loadPlugin(QFileInfo fileinfo) {
  if (fileinfo.exists()) {
    QPluginLoader loader(fileinfo.absoluteFilePath());
    QList<WingPluginInfo> loadedplginfos;
    QList<QVariant> emptyparam;

    try {
      auto p = qobject_cast<IWingToolPlg *>(loader.instance());
      dInfo(tr("PluginLoadingBegin : %1").arg(p->pluginName()));
      if (p) {
        if (p->signature() != WINGSUMMER) {
          dError(tr("ErrLoadPluginSign"));
          loader.unload();
          return;
        }
        if (p->sdkVersion() != SDKVERSION) {
          dError(tr("ErrLoadPluginSDKVersion"));
          loader.unload();
          return;
        }
        if (!p->pluginName().trimmed().length()) {
          dError(tr("ErrLoadPluginNoName"));
          loader.unload();
          return;
        }

        auto handler = p->serviceHandler();
        if (handler.isNull()) {
          dError(tr("ErrLoadPluginNoHandler"));
          loader.unload();
          return;
        }

        auto meta = p->serviceMeta();
        auto clsname = meta->className();
        if (clsname == nullptr) {
          dError(tr("ErrLoadPluginProvider"));
          loader.unload();
          return;
        }

        auto provider = QString::fromUtf8(clsname);
        if (provider.isEmpty() || loadedProvider.contains(provider)) {
          dError(tr("ErrLoadPluginProvider"));
          loader.unload();
          return;
        }

        // 筹备一个临时容器
        PluginRecord record;

        // 插件至少含有一种有效服务
        auto srvc = meta->methodCount();

        QVector<const char *> tmpfunc;
        // 暂时缓存一下原函数名称，供之后的函数名本地化之用

        for (auto i = 0; i < srvc; i++) {
          auto m = meta->method(i);
          if (strcmp(PLUGINSRVTAG, m.tag())) {
            continue;
          }

          if (m.parameterCount() > 10) {
            dError(tr("[InvaildPlgSrvArg]") +
                   QString("%1/10").arg(m.parameterCount()));
            continue;
          }

          // 记录有效服务函数
          record.services.append(m);
          tmpfunc.append(m.name());
          record.serviceNames.append(QString::fromUtf8(m.name()));
        }

        if (record.services.isEmpty()) {
          dError(tr("ErLoadPluginService"));
          loader.unload();
          return;
        }

        // 检查完毕后，就可以进入真正的加载环节
        emit p->pluginServicePipe(HostService, {LoadingPluginMsg});

        if (!p->init(loadedplginfos)) {
          dError(tr("ErrLoadInitPlugin"));
          loader.unload();
          return;
        }

        for (auto &item : tmpfunc) {
          record.servicetrNames.append(
              QCoreApplication::translate(clsname, item));
        }

        WingPluginInfo info;
        info.provider = provider;
        info.pluginName = p->pluginName();
        info.pluginAuthor = p->pluginAuthor();
        info.pluginComment = p->pluginComment();
        info.pluginVersion = p->pluginVersion();

        loadedplginfos << info;
        m_plgs << p;
        loadedProvider << provider;
        m_plgsMD5s << Utilities::getPUID(p);

        dWarning(tr("PluginInitRegister"));

        // 看看有没有要注册的托盘

        auto menuobj = p->trayRegisteredMenu();
        if (menuobj) {
          auto menu = qobject_cast<QMenu *>(menuobj);
          if (menu) {
            traymenu->addMenu(menu);
            plgmenuCount++;
          } else {
            auto amenu = qobject_cast<QAction *>(menuobj);
            if (amenu) {
              traymenu->addAction(amenu);
              plgmenuCount++;
            } else {
              dError(tr("InvaildPlgMenu in loading %1").arg(p->pluginName()));
            }
          }
        }

        // 初始化插件容器
        record.provider = provider;
        m_plgrec.insert(p, record);

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
                [=](QKeySequence keyseq) {
                  auto sender = qobject_cast<IWingToolPlg *>(QObject::sender());
                  if (sender == nullptr)
                    return QUuid();
                  auto hk = this->manager->registerHotkey(keyseq, false);
                  if (hk) {
                    auto uuid = QUuid::createUuid();
                    m_plgrec[sender].hotkeyuid << uuid;
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
          auto &plist = m_plgrec[sender].hotkeyuid;
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
                  auto &plist = m_plgrec[sender].hotkeyuid;
                  if (plist.contains(id)) {
                    return this->manager->enableHotKey(uhmap[id], enabled);
                  }
                  return false;
                });
        connect(p, &IWingToolPlg::editHotkey, this,
                [=](const QUuid id, QKeySequence seq) {
                  auto sender = qobject_cast<IWingToolPlg *>(QObject::sender());
                  if (sender == nullptr)
                    return false;
                  auto &plist = m_plgrec[sender].hotkeyuid;
                  if (plist.contains(id)) {
                    return this->manager->editHotkey(uhmap[id], seq);
                  }
                  return false;
                });
        connect(p, &IWingToolPlg::remoteCall, this,
                [=](const QString provider, const QString callback,
                    QVector<QVariant> params) {
                  auto sender = qobject_cast<IWingToolPlg *>(QObject::sender());
                  if (sender == nullptr)
                    return RemoteCallError::Unkown;

                  auto index = loadedProvider.indexOf(provider);
                  if (index < 0)
                    return RemoteCallError::PluginNotFound;

                  auto plg = m_plgs[index];

                  QVariant ret;
                  auto res = this->remoteCall(
                      plg, const_cast<QString &>(callback), params, ret);
                  if (res == 0) {
                    return RemoteCallError::PluginNotFound;
                  } else if (res < 0) {
                    return RemoteCallError::Unkown;
                  }

                  // 将返回值以相同方式传送回去
                  emit sender->pluginServicePipe(RemoteCallRes, {ret});

                  return RemoteCallError::Success;
                });

        emit p->pluginServicePipe(HostService, {LoadedPluginMsg});
        dInfo(tr("PluginLoaded : %1 %2").arg(p->pluginName()).arg(provider));
      } else {
        dError(loader.errorString());
        loader.unload();
      }
    } catch (...) {
      dError(tr("PluginLoadingEx"));
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

int PluginSystem::pluginCounts() { return m_plgs.count(); }

QList<QKeySequence> PluginSystem::pluginRegisteredHotkey(IWingToolPlg *plg) {
  if (plg == nullptr)
    return QList<QKeySequence>();

  QList<QKeySequence> keys;
  auto plist = m_plgrec[plg].hotkeyuid;
  for (auto &item : plist) {
    auto hk = uhmap[item];
    keys << hk->shortcut();
  }
  return keys;
}

bool PluginSystem::pluginCall(QString provider, int serviceID,
                              QVector<QVariant> params) {
  if (serviceID < 0)
    return false;

  auto i = loadedProvider.indexOf(provider);
  if (i < 0)
    return false;

  auto plg = m_plgs[i];
  QVariant ret;
  return remoteCall(plg, serviceID, params, ret) > 0;
}

QByteArray PluginSystem::pluginHash(int index) { return m_plgsMD5s[index]; }

int PluginSystem::pluginIndexByProvider(QString provider) {
  return loadedProvider.indexOf(provider);
}

const QStringList &PluginSystem::pluginServiceNames(IWingToolPlg *plg) {
  if (m_plgs.contains(plg))
    return m_plgrec[plg].serviceNames;
  return emptystrlist;
}

const QStringList &PluginSystem::pluginServicetrNames(IWingToolPlg *plg) {
  if (m_plgs.contains(plg))
    return m_plgrec[plg].servicetrNames;
  return emptystrlist;
}

QString PluginSystem::pluginProvider(IWingToolPlg *plg) {
  if (m_plgs.contains(plg))
    return m_plgrec[plg].provider;
  return QString();
}

bool PluginSystem::hasRegisteredMenu() { return plgmenuCount > 0; }

IWingToolPlg *PluginSystem::loopUpHotkey(QUuid uuid, int &index) {
  for (auto plg : m_plgs) {
    auto res = m_plgrec[plg].hotkeyuid.indexOf(uuid);
    if (res >= 0) {
      index = res;
      return plg;
    }
  }
  return nullptr;
}

int PluginSystem::remoteCall(IWingToolPlg *plg, QString &callback,
                             QVector<QVariant> params, QVariant &ret) {
  // 开始查询有没有对应的函数
  auto &srvn = m_plgrec[plg].serviceNames;
  auto id = 0;
  auto &srv = m_plgrec[plg].services;

  for (;; id++) {
    id = srvn.indexOf(callback, id);
    if (id < 0)
      break;

    // 先检查一下参数个数
    auto &m = srv[id];
    if (params.count() != m.parameterCount()) {
      continue;
    }

    // 检查类型是否合格
    auto len = params.count();
    bool invalid = false;
    for (auto i = 0; i < len; i++) {
      if (!params[i].canConvert(m.parameterType(i))) {
        invalid = true;
        break;
      }
    }
    if (invalid)
      continue;

    return remoteCall(plg, id, params, ret);
  }

  return CALL_INVALID;
}

int PluginSystem::remoteCall(IWingToolPlg *plg, int callID,
                             QVector<QVariant> params, QVariant &ret) {

  auto caller = m_plgrec[plg].services[callID];

  if (!caller.isValid()) {
    dError(tr("[remoteCallVaildErr]") +
           QString("%1 : %2")
               .arg(m_plgrec[plg].provider)
               .arg(QString::fromUtf8(caller.name())));
    return CALL_INVALID;
  }
  auto len = caller.parameterCount();
  if (params.count() < len) {
    dError(tr("[remoteCallVaildErr]") +
           QString("%1 : %2 [%3/%4]")
               .arg(m_plgrec[plg].provider)
               .arg(QString::fromUtf8(caller.name()))
               .arg(params.count())
               .arg(len));
    return CALL_ARG_ERROR;
  }

  try {

#define RETURN(type, value)                                                    \
  QGenericReturnArgument(QMetaType::typeName(type), static_cast<void *>(&value))
#define ARG(type, value)                                                       \
  QGenericArgument(QMetaType::typeName(type), static_cast<void *>(&value))

    params.resize(10);

    for (auto i = 0; i < len; i++) {
      if (!params[i].convert(caller.parameterType(i))) {
        dError(tr("[remoteCallArgErr]") +
               QString("%1 : %2 [%3:%4|%5]")
                   .arg(m_plgrec[plg].provider)
                   .arg(QString::fromUtf8(caller.name()))
                   .arg(i)
                   .arg(QMetaType::typeName(caller.parameterType(i)))
                   .arg(params[i].typeName()));
        return CALL_ARG_ERROR;
      }
    }

    // 开始调用
    caller.invoke(plg->serviceHandler(), Qt::ConnectionType::DirectConnection,
                  RETURN(caller.returnType(), ret),
                  ARG(caller.parameterType(0), params[0]),
                  ARG(caller.parameterType(1), params[1]),
                  ARG(caller.parameterType(2), params[2]),
                  ARG(caller.parameterType(3), params[3]),
                  ARG(caller.parameterType(4), params[4]),
                  ARG(caller.parameterType(5), params[5]),
                  ARG(caller.parameterType(6), params[6]),
                  ARG(caller.parameterType(7), params[7]),
                  ARG(caller.parameterType(8), params[8]),
                  ARG(caller.parameterType(9), params[9]));

    return CALL_SUCCESS;
  } catch (...) {
    dError(tr("[remoteCallEx]") + QString("%1 : %2")
                                      .arg(m_plgrec[plg].provider)
                                      .arg(QString::fromUtf8(caller.name())));
    return CALL_EXCEPTION;
  }
}
