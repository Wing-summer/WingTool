#ifndef IWINGTOOLPLG_H
#define IWINGTOOLPLG_H

#include <QApplication>
#include <QIcon>
#include <QKeySequence>
#include <QMenu>
#include <QObject>
#include <QPointer>
#include <QUuid>
#include <QVariant>

#define SDKVERSION 1
#define GETPLUGINQM(name)                                                      \
  (QCoreApplication::applicationDirPath() + "/plglang/" + name)
#define PLUGINDIR (QCoreApplication::applicationDirPath() + "/plugin")

#define WINGSUMMER "wingsummer"

#ifndef Q_MOC_RUN
#define PLUGINSRV
#define PLUGININT
#endif

#define PLUGINSRVTAG "PLUGINSRV"
#define PLUGININTTAG "PLUGININT"

/*=================================*/

// 插件系统预定义服务号，全部为负数
// 如果服务号为非负数，则表示为插件服务

#define PLUGINLOADING -1 // 插件加载中消息
#define PLUGINLOADED -2  // 插件加载完消息

/*=================================*/

enum class MouseWheelEvent { None, Up, Down, Left, Right };
Q_DECLARE_METATYPE(MouseWheelEvent)

#undef Success

enum class RemoteCallError {
  Success,         // 回调成功
  Unkown,          // 回调未知错误，通常由于未处理异常导致
  PluginNotFound,  // 找不到的插件
  ServiceNotFound, // 找到插件，但没有找到对应的服务
  ArgError,        // 调用的参数出现问题
  MessageIDError // 发送消息的 ID 错误，基本发送了小于 0 的消息
};
Q_DECLARE_METATYPE(RemoteCallError)

#undef ButtonPress
#undef ButtonRelease
enum HookIndex {
  None = 0,
  ButtonPress = 1,
  ButtonRelease = 2,
  Clicked = 4,
  DoubleClicked = 8,
  MouseWheel = 16,
  MouseMove = 32,
  MouseDrag = 64
};
Q_DECLARE_METATYPE(HookIndex)

struct WingPluginInfo;

class IWingToolPlg : public QObject {
  Q_OBJECT
public:
  enum class Catagorys {
    Explor,       // 探索
    Productivity, // 生产力
    Searcher,     // 搜索工具
    Picture,      // 图像处理
    Develop,      // 开发
    Creative      // 创造
  };

  Q_ENUM(Catagorys)

public:
  // 指示 SDK 版本，目前采用不兼容模式
  virtual int sdkVersion() = 0;
  // 签名，目前固定为 WINGSUMMER 这个宏
  virtual QString signature() = 0;
  // 析构函数
  virtual ~IWingToolPlg() {}
  // 插件预初始化，主要初始化服务以备正式初始化使用，可选
  // 如果预初始化失败，则插件会被卸载
  // 注：如果提供翻译文件名，此时就已经被加载
  virtual bool preInit() { return true; }
  // 插件初始化函数，如果初始化失败，则插件会被卸载
  virtual bool init(QList<WingPluginInfo> loadedplugin) = 0;
  // 插件卸载函数
  virtual void unload() = 0;
  // 插件的名称，可以使用 tr 实现多语言
  virtual QString pluginName() = 0;
  // 插件作者
  virtual QString pluginAuthor() = 0;
  // 插件类别，目前还没啥特定的作用，以后发掘
  virtual Catagorys pluginCatagory() = 0;
  // 插件版本号，作者定
  virtual uint pluginVersion() = 0;
  // 插件网站，供插件作者宣传用
  virtual QString pluginWebsite() = 0;
  // 插件说明
  virtual QString pluginComment() = 0;
  // 插件图标，我建议必须有一个，否则后面不好识别
  virtual QIcon pluginIcon() = 0;
  // 插件服务类，必须继承 QObject ，自定义
  virtual const QMetaObject *serviceMeta() = 0;
  // 插件指针
  virtual const QPointer<QObject> serviceHandler() = 0;
  // 插件订阅，如果需要跟踪鼠标就需要订阅
  virtual HookIndex getHookSubscribe() { return HookIndex::None; }
  // 注册在程序右键托盘菜单，这个对于某些功能会十分方便
  // 但非必要不要弄，因为这样的插件多了，反而麻烦了，一个插件仅有一项
  // 类型仅支持 QMenu* 或者 QAction* 否则不载入
  virtual QObject *trayRegisteredMenu() { return nullptr; }
  // 插件的语言包文件名，如果空插件系统默认不加载
  // 如果有需要还请手动加载
  virtual QString translatorFile() { return QString(); }

signals:
  // 注册热键，如果被占用则返回空表示失败（通常是重复），
  // 大于等于 0 则表示成功，返回句柄
  QUuid registerHotkey(QKeySequence keyseq);

  // 修改热键状态，其中 id 为注册热键句柄，enable 为热键的新状态
  bool enableHotKey(const QUuid id, bool enabled = true);

  // 修改热键
  bool editHotkey(const QUuid id, QKeySequence seq);

  // 注销热键，其中 id 为注册热键句柄
  bool unregisterHotkey(const QUuid id);

  // 跨插件函数远程调用，其中 puid 为插件的唯一标识，
  // callback 为回调函数名称， params 为远程调用的参数
  QVariant remoteCall(const QString provider, const QString callback,
                      QVector<QVariant> params, RemoteCallError &err);

  // 向某个插件发送一个消息
  // 注：不要用它发送数值小于 0 的消息，会发送失败滴，别瞎搞
  QVariant sendRemoteMessage(const QString provider, int id,
                             QList<QVariant> params, RemoteCallError &err);

  // 查询某个插件是否存在
  bool isProviderExists(const QString provider);

  // 查询某个插件服务是否含有所述服务
  bool isServiceExists(const QString provider, const QString callback);

  // 查询某个插件服务是否接口
  bool isInterfaceExists(const QString provider, const QString callback);

  // 获取服务的参数类型
  QList<int> getServiceParamTypes(const QString provider,
                                  const QString callback);

  // 获取接口的参数类型
  QVector<QList<int>> getInterfaceParamTypes(const QString provider,
                                             const QString callback);

  // 获取全局按下的修饰键序列
  Qt::KeyboardModifiers getPressedKeyModifiers();

  // 获取全局按下的鼠标按键序列
  Qt::MouseButtons getPressedMouseButtons();

  // 获取所有插件提供者名称
  QStringList getPluginProviders();

  // 获取插件信息
  WingPluginInfo getPluginInfo(const QString provider);

  // 获取插件的所有服务名，isTr 指示是否使用本地化的
  QStringList getPluginServices(const QString provider, bool isTr = false);

  // 获取所有插件的所有接口名（注：无去重，可能有重复项）
  QStringList getPluginInterfaces(const QString provider);

public slots:
  // 宿主开始回调函数时候使用，第一个参数是函数服务索引，第二个是参数集合
  virtual QVariant pluginServicePipe(int serviceID, QList<QVariant> params) = 0;

  // 当插件窗口选中该插件点击插件中心按钮时触发，以供插件调用自身设置对话框
  virtual void onPluginCenter() = 0;

  // 当鼠标任何一个键被按下就会触发该函数，如果想处理重载
  virtual void buttonPress(Qt::MouseButton btn, int x, int y) {
    Q_UNUSED(btn);
    Q_UNUSED(x);
    Q_UNUSED(y);
  }

  // 当鼠标任何一个键从被按下的状态释放就会触发该函数，如果想处理重载
  virtual void buttonRelease(Qt::MouseButton btn, int x, int y) {
    Q_UNUSED(btn);
    Q_UNUSED(x);
    Q_UNUSED(y);
  }

  // 当鼠标进行左键单击时会触发该函数，如果想处理重载
  // 该函数也就是 buttonPress 的一种特殊情况
  virtual void clicked(int x, int y) {
    Q_UNUSED(x);
    Q_UNUSED(y);
  }

  // 当鼠标双击时会触发该函数，如果想处理重载
  // 注：当鼠标双击时，系统无法识别好第一个点击，会被识别
  // 为单击，但第二个紧接的单击会被识别为双击
  virtual void doubleClicked(int x, int y) {
    Q_UNUSED(x);
    Q_UNUSED(y);
  }

  // 当鼠标滚轮滚动时会触发该函数，如果想处理重载
  virtual void mouseWheel(MouseWheelEvent direction) { Q_UNUSED(direction); }

  // 当鼠标移动时会触发该函数，如果想处理重载
  virtual void mouseMove(int x, int y) {
    Q_UNUSED(x);
    Q_UNUSED(y);
  }

  // 当鼠标进行拖拽操作时触发该函数，如果想处理重载
  virtual void mouseDrag(int x, int y) {
    Q_UNUSED(x);
    Q_UNUSED(y);
  }

  // 当插件注册的热键触发时会触发该函数
  virtual void hotkeyTirggered(QUuid id) { Q_UNUSED(id); }
  // 如果插件注册的热键被释放时会触发该函数
  virtual void hotkeyReleased(QUuid id) { Q_UNUSED(id); }
  // 如果插件注册的热键启用状态改变时会触发该函数
  virtual void hotkeyEnableChanged(bool value, QUuid id) {
    Q_UNUSED(value);
    Q_UNUSED(id);
  }
};

#define IWINGPLUGIN_INTERFACE_IID "com.wingsummer.iwingtoolplg"
Q_DECLARE_INTERFACE(IWingToolPlg, IWINGPLUGIN_INTERFACE_IID)

struct WingPluginInfo {
  QString pluginName;
  QString pluginAuthor;
  IWingToolPlg::Catagorys pluginCatagory;
  uint pluginVersion;
  QString provider;
  QString pluginComment;
  QString pluginWebsite;
  HookIndex HookSubscribe;
  QString translatorFile;
};

#endif // IWINGTOOLPLG_H
