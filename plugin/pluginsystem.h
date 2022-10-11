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

#define CALL_SUCCESS 1
#define CALL_INVALID 0
#define CALL_EXCEPTION -1
#define CALL_ARG_ERROR -2

class PluginSystem : public QObject {
  Q_OBJECT
public:
  explicit PluginSystem(QMenu *systray, QObject *parent = nullptr);
  ~PluginSystem();

  static PluginSystem *instance();

  bool LoadPlugin();
  void UnloadPlugin();

  void loadPlugin(QFileInfo filename);

  QList<IWingToolPlg *> plugins();
  IWingToolPlg *plugin(int index);

  int pluginCounts();

  QList<QKeySequence> pluginRegisteredHotkey(IWingToolPlg *plg);

  bool pluginCall(QString provider, int serviceID, QVector<QVariant> params);

  QByteArray pluginHash(int index);

  int pluginIndexByProvider(QString provider);

  const QStringList &pluginServiceNames(IWingToolPlg *plg);
  const QStringList &pluginServicetrNames(IWingToolPlg *plg);
  QString pluginProvider(IWingToolPlg *plg);

  bool hasRegisteredMenu();

private:
  IWingToolPlg *loopUpHotkey(QUuid uuid, int &index);
  int remoteCall(IWingToolPlg *plg, QString &callback, QVector<QVariant> params,
                 QVariant &ret);
  int remoteCall(IWingToolPlg *plg, int callID, QVector<QVariant> params,
                 QVariant &ret);

private:
  struct PluginRecord {
    QString provider;
    QList<QUuid> hotkeyuid;
    QList<QMetaMethod> services;

    QStringList serviceNames;   // 插件服务名缓存
    QStringList servicetrNames; // 插件服务本地化名称缓存
  };

private:
  const QStringList emptystrlist; // 留作空的字符串数组作返回值用

  static PluginSystem *m_instance;
  AppManager *manager;

  QStringList loadedProvider;   // 已加载的插件 provider ，需要唯一
  QList<IWingToolPlg *> m_plgs; // 已加载的插件集合
  QMap<IWingToolPlg *, PluginRecord> m_plgrec; // 已加载的插件记录

  QMap<QUuid, Hotkey *> uhmap; // UUID 和 QHotkey 的对应图
  QMap<IWingToolPlg::Catagorys, QList<IWingToolPlg *>>
      m_catplgs; // 对应类别的插件集合
  QMap<HookIndex, QList<IWingToolPlg *>> dispatcher; // Hook 消息订阅

  QList<QByteArray> m_plgsMD5s; // 已加载的插件 HASH

  QMenu *traymenu;

  uint plgmenuCount = 0;
};

#endif // PLUGINSYSTEM_H
