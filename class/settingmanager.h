#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include "class/hotkey.h"
#include "utilities.h"
#include <QObject>

#define TOOLGRIDSIZE 40

class SettingManager : public QObject {
  Q_OBJECT
public:
  explicit SettingManager(QObject *parent = nullptr);

  static SettingManager *instance();

public:
  bool loadSettings();
  bool saveSettings();
  bool exportSettings(QString filename);
  void resetSettings();

public:
  int toolGridSize();
  void setToolGridSize(int v);

signals:
  void sigToolGridSizeChanged(int v);
  void loadingFinish();

public slots:
  void getHokeysBuffer(QList<Hotkey *> &hotkeysBuf,
                       QMap<Hotkey *, ToolStructInfo> &buffer);
  void getToolLeftBuffer(ToolStructInfo buffer[]);
  void getToolRightBuffer(QList<ToolStructInfo> &buffer);

private:
  static SettingManager *m_instance;

  int m_toolGridSize = TOOLGRIDSIZE;
};

#endif // SETTINGMANAGER_H
