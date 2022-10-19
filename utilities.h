#ifndef UTILIES_H
#define UTILIES_H

#include "plugin/iwingtoolplg.h"
#include <QComboBox>
#include <QIcon>
#include <QImageReader>
#include <QKeySequence>
#include <QMimeDatabase>
#include <QString>
#include <QtDBus>
#define ProgramIcon QIcon(":/images/logo.svg")
#define ICONRES(name) QIcon(":/images/" name ".png")
#define ICONRES2(name) QIcon(":/images/" name ".svg")

// 该结构体在不同使用场合下，每个成员含义可能有所变化
// 也并不是所有的成员在同一个场合用到的
struct ToolStructInfo {
  bool enabled = false;
  QKeySequence seq = QKeySequence(); // 不是热键，就不会使用
  QString process = QString(); // 如果是文件是路径，如果是插件是插件名
  QString params = QString();   // 参数传参
  QString fakename = QString(); // 别名，用于代替默认显示

  // 接下来这个部分非热键部分使用
  // 作用是自定义 icon

  QString iconpath = QString(); // 图标路径
  QIcon icon = QIcon();         // 缓存

  // 以下仅供插件使用
  int serviceID = -1; // 这个 ID 比服务 CALLID 小 1
  int pluginIndex = -1;
  QString provider = QString();
  QString serviceName = QString(); // 函数服务名缓存

  bool isPlugin = false;
};

Q_DECLARE_METATYPE(ToolStructInfo)

class Utilities {
public:
  static QIcon processPluginIcon(IWingToolPlg *plg) {
    if (plg->pluginIcon().isNull()) {
      return ICONRES("plugin");
    }
    return plg->pluginIcon();
  }

  static bool isIconExist(QString name) {
    QMimeDatabase db;
    auto t = db.mimeTypeForFile(name);
    if (QIcon::hasThemeIcon(name)) {
      return true;
    }

    if (QFile::exists(name)) {
      QImageReader r(name);
      return r.canRead();
    }
    return false;
  }

  static QIcon trimIconFromFile(QString filename) {
    if (filename.isEmpty())
      return QIcon();

    if (QFile::exists(filename)) {
      QPixmap img;
      if (img.load(filename))
        return QIcon(img.width() > 64 || img.height() > 64
                         ? img.scaled(64, 64, Qt::KeepAspectRatio)
                         : img);
    }
    // 我不信你如果使用 theme 图标的路径含有 / 字符
    // 据我多次观察
    if (filename.indexOf('/') < 0) {
      if (QIcon::hasThemeIcon(filename))
        return QIcon::fromTheme(filename);
    } else {
      QMimeDatabase db;
      auto t = db.mimeTypeForFile(filename);
      if (QIcon::hasThemeIcon(t.iconName()))
        return QIcon::fromTheme(t.iconName());
    }
    return QIcon();
  }

  static QIcon trimIconFromInfo(IWingToolPlg *plg, ToolStructInfo &info) {
    if (info.icon.isNull()) {
      if (info.isPlugin) {
        if (plg == nullptr)
          return QIcon();
        return plg->pluginIcon();
      } else {
        return trimIconFromFile(info.process);
      }
    } else {
      return info.icon;
    }
  }

  static QString getProgramName(const QStringList &services,
                                ToolStructInfo &info) {
    if (info.fakename.length())
      return info.fakename;
    return info.isPlugin ? info.process + " | " + services[info.serviceID]
                         : QFileInfo(info.process).fileName();
  }

  static QString getToolTipContent(const QStringList &services,
                                   ToolStructInfo &info) {
    if (info.isPlugin) {
      return QObject::tr("FakeName:%1\nProcess:%2\nService:%3\nParams:%4")
          .arg(info.fakename)
          .arg(info.process)
          .arg(services[info.serviceID])
          .arg(info.params);
    } else {
      return QObject::tr("FakeName:%1\nProcess:%2\nParams:%3")
          .arg(info.fakename)
          .arg(info.process)
          .arg(info.params);
    }
  }

  static QStringList parseCmdParams(QString str) {
    static QRegularExpression regex("(\"[^\"]+\"|[^\\s\"]+)");
    QStringList args;
    int off = 0;
    while (true) {
      auto match = regex.match(str, off);
      if (!match.hasMatch()) {
        break;
      }
      auto res = match.captured();
      if (res[0] == '\"')
        res = res.replace("\"", "");
      if (res[0] == '\'')
        res = res.replace("'", "");
      args << res;
      off = match.capturedEnd();
    }
    return args;
  }

  static bool isVaildString(QByteArray &arr, QString &output) {
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::ConverterState state;

    output = codec->toUnicode(arr.constData(), arr.size(), &state);
    return state.invalidChars == 0;
  }
};

#endif // UTILIES_H
