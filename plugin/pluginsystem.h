#ifndef PLUGINSYSTEM_H
#define PLUGINSYSTEM_H

#include "class/appmanager.h"
#include "iwingtoolplg.h"
#include <DLog>
#include <QFileInfo>
#include <QMap>
#include <QObject>
#include <QUuid>

DCORE_USE_NAMESPACE

class PluginSystem : public QObject {
  Q_OBJECT
public:
  enum class LP {
    begin,
    signature,
    sdkVersion,
    pluginName,
    provider,
    service,
    plugin2MessagePipe,
    init,
  };
  Q_ENUM(LP)

public:
  explicit PluginSystem(QObject *parent = nullptr);
  ~PluginSystem();

  static PluginSystem *instance();

  bool LoadPlugin();
  void UnloadPlugin();

  void loadPlugin(QFileInfo filename);

  QList<IWingToolPlg *> plugins();
  IWingToolPlg *plugin(int index);

  int pluginCounts();

  QList<QKeySequence> pluginRegisteredHotkey(IWingToolPlg *plg);

  bool pluginCall(QString provider, int serviceID, QList<QVariant> params);

private:
  IWingToolPlg *loopUpHotkey(QUuid uuid, int &index);

private:
  static PluginSystem *m_instance;
  AppManager *manager;

  QStringList loadedProvider;                 // 已加载的插件 PUID
  QList<IWingToolPlg *> m_plgs;               // 已加载的插件集合
  QMap<IWingToolPlg *, QList<QUuid>> m_plghk; // 注册的热键句柄集合
  QMap<QUuid, Hotkey *> uhmap;                // UUID 和 QHotkey 的对应图
  QMap<IWingToolPlg::Catagorys, QList<IWingToolPlg *>>
      m_catplgs; // 对应类别的插件集合

  QMap<HookIndex, QList<IWingToolPlg *>> dispatcher;
};

#endif // PLUGINSYSTEM_H
