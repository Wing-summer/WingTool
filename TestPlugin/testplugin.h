#ifndef GENERICPLUGIN_H
#define GENERICPLUGIN_H

#include "../plugin/iwingtoolplg.h"
#include <QDialog>
#include <QTextBrowser>
#include <QTranslator>

class TestService : public QObject {
  Q_OBJECT
public:
  explicit TestService() {}
  explicit TestService(const TestService &) : QObject(nullptr) {}
  explicit TestService(QTextBrowser *browser, QDialog *d)
      : b(browser), dialog(d) {}
  virtual ~TestService() {}

public slots:
  PLUGINSRV void func1(int v) {
    b->append(QString("[func1 call] : %1").arg(v));
  }
  PLUGINSRV void func2(QString v) { b->append(QString("[func2 call] : ") + v); }
  PLUGINSRV void func3() { dialog->setVisible(!dialog->isVisible()); }

private:
  void trans() {
    tr("func1");
    tr("func2");
    tr("func3");
  }

private:
  QTextBrowser *b;
  QDialog *dialog;
};

Q_DECLARE_METATYPE(TestService)

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
  QString pluginAuthor() override;
  Catagorys pluginCatagory() override;
  uint pluginVersion() override;
  QString pluginComment() override;
  QString pluginWebsite() override;
  QIcon pluginIcon() override;
  const QMetaObject *serviceMeta() override;
  const QPointer<QObject> serviceHandler() override;
  HookIndex getHookSubscribe() override;

  QObject *trayRegisteredMenu() override;

public slots:
  QVariant pluginServicePipe(int serviceID, QList<QVariant> params) override;
  virtual void onPluginCenter() override;

private:
  QUuid testhotkey;
  QDialog *dialog;
  QTextBrowser *tbinfo;

  TestService *services;
  QTranslator translator;

  QAction *testmenu;
};

#endif // GENERICPLUGIN_H
