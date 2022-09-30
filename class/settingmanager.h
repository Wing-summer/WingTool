#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include "class/hotkey.h"
#include "utilities.h"
#include <QKeySequence>
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
  int toolGridSize() const;
  void setToolGridSize(const int v);

  QKeySequence toolBoxHotkey() const;
  void setToolBoxHotkey(const QKeySequence seq);

  Qt::KeyboardModifier toolwinMod() const;
  void setToolwinMod(const Qt::KeyboardModifier &toolwinMod);

  Qt::MouseButton toolwinMouseBtn() const;
  void setToolMouseBtn(const Qt::MouseButton &toolwinMouseBtn);

signals:
  void sigToolGridSizeChanged(int v);
  void loadedGeneral();
  void sigToolBoxHotkeyChanged(const QKeySequence seq);
  void sigToolWinShortCutChanged(const Qt::KeyboardModifier mod,
                                 const Qt::MouseButton btn);

  void sigSaveConfig(QDataStream &stream);

  // 配置添加信号
  void addHotKeyInfo(ToolStructInfo &info);
  void setToolWinInfo(int index, ToolStructInfo &info);
  void addWinToolInfo(ToolStructInfo &info);

private:
  static SettingManager *m_instance;

  int m_toolGridSize;

  QKeySequence m_toolBox;

  Qt::KeyboardModifier m_toolwinMod;
  Qt::MouseButton m_toolMouse;
};

#endif // SETTINGMANAGER_H
