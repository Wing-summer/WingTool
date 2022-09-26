#ifndef PLUGINSYSTEM_H
#define PLUGINSYSTEM_H

#include "iwingtoolplg.h"
#include <QFileInfo>
#include <QObject>

class PluginSystem : public QObject {
  Q_OBJECT
public:
  enum class LP {
    begin,
    signature,
    sdkVersion,
    pluginName,
    puid,
    plugin2MessagePipe,
    init,
  };
  Q_ENUM(LP)

public:
  explicit PluginSystem(QObject *parent = nullptr);

  ~PluginSystem();
  bool LoadPlugin();
  void UnloadPlugin();

  QList<IWingToolPlg *> plugins();

  void loadPlugin(QFileInfo filename);
  IWingToolPlg *currentControlPlugin();
};

#endif // PLUGINSYSTEM_H
