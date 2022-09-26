#ifndef IWINGTOOLPLG_H
#define IWINGTOOLPLG_H

#include <QCryptographicHash>
#include <QObject>

#define SDKVERSION 0
#define GETPLUGINQM(name)                                                      \
  (QCoreApplication::applicationDirPath() + "/plglang/" + name)
#define PLUGINDIR (QCoreApplication::applicationDirPath() + "/plugin")

#define WINGSUMMER "wingsummer"

struct WingPluginInfo {
  QString pluginName;
  QString pluginAuthor;
  uint pluginVersion;
  QString puid;
  QString pluginComment;
};

enum class MouseButton {
  NoneButton,
  LeftButton,
  RightButton,
  MiddleButton,
  XButton_1,
  XButton_2
};
Q_DECLARE_METATYPE(MouseButton)

enum class MouseWheel { None, Up, Down, Left, Right };
Q_DECLARE_METATYPE(MouseWheel)

class IWingToolPlg : public QObject {
  Q_OBJECT
public:
  virtual int sdkVersion() = 0;
  virtual QString signature() = 0;
  QByteArray puid() { return GetPUID(this); }
  virtual ~IWingToolPlg() {}

  virtual bool init(QList<WingPluginInfo> loadedplugin) = 0;
  virtual void unload() = 0;
  virtual QString pluginName() = 0;
  virtual QString pluginAuthor() = 0;
  virtual uint pluginVersion() = 0;
  virtual QString pluginComment() = 0;

  static QByteArray GetPUID(IWingToolPlg *plugin) {
    auto str = QString("%1%2%3%4")
                   .arg(WINGSUMMER)
                   .arg(plugin->pluginName())
                   .arg(plugin->pluginAuthor())
                   .arg(plugin->pluginVersion());
    return QCryptographicHash::hash(str.toLatin1(), QCryptographicHash::Md5);
  }

  virtual QIcon pluginIcon() = 0;

signals:
  bool registerHotkey(QKeySequence &keyseq);
  bool enableHotKey(int index, bool enabled = true);
  bool unregisterHotkey(int index);

public slots:
  virtual void buttonPress(MouseButton btn, int x, int y) {
    Q_UNUSED(btn);
    Q_UNUSED(x);
    Q_UNUSED(y);
  }
  virtual void buttonRelease(MouseButton btn, int x, int y) {
    Q_UNUSED(btn);
    Q_UNUSED(x);
    Q_UNUSED(y);
  }
  virtual void clicked(int x, int y) {
    Q_UNUSED(x);
    Q_UNUSED(y);
  }
  virtual void doubleClicked(int x, int y) {
    Q_UNUSED(x);
    Q_UNUSED(y);
  }
  virtual void mouseWheel(MouseWheel direction) { Q_UNUSED(direction); }
  virtual void mouseMove(int x, int y) {
    Q_UNUSED(x);
    Q_UNUSED(y);
  }

  virtual void hotkeyTirggered(int index) { Q_UNUSED(index); }
  virtual void hotkeyReleased(int index) { Q_UNUSED(index); }
  virtual void hotkeyEnableChanged(bool value, int index) {
    Q_UNUSED(value);
    Q_UNUSED(index);
  }

  virtual void selectionTextChanged(const QString &selectedText) {
    Q_UNUSED(selectedText);
  }
};

#define IWINGPLUGIN_INTERFACE_IID "com.wingsummer.iwingplugin"
Q_DECLARE_INTERFACE(WingPluginInfo, IWINGPLUGIN_INTERFACE_IID)

#endif // IWINGTOOLPLG_H
