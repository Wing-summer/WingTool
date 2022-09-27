#ifndef GENERICPLUGIN_H
#define GENERICPLUGIN_H

#include "../plugin/iwingtoolplg.h"
#include <QDialog>
#include <QTextBrowser>

class TestPlugin : public IWingToolPlg {
  Q_OBJECT
#if QT_VERSION >= 0x050000
  Q_PLUGIN_METADATA(IID IWINGPLUGIN_INTERFACE_IID FILE "TestPlugin.json")
#endif // QT_VERSION >= 0x050000

  Q_INTERFACES(IWingToolPlg)

public:
  TestPlugin(QObject *parent = nullptr);
  int sdkVersion() override;
  QString signature() override;
  ~TestPlugin() override;

  bool init(QList<WingPluginInfo> loadedplugin) override;
  void unload() override;
  QString pluginName() override;
  QString provider() override;
  QString pluginAuthor() override;
  Catagorys pluginCatagory() override;
  uint pluginVersion() override;
  QString pluginComment() override;
  QIcon pluginIcon() override;
  QStringList pluginServices() override;
  HookIndex getHookSubscribe() override;

public slots:
  QVariant pluginServicePipe(int serviceID, QList<QVariant> params) override;

private:
  QUuid testhotkey;
  QDialog *dialog;
  QTextBrowser *tbinfo;
};

#endif // GENERICPLUGIN_H
