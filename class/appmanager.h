#ifndef APPMANAGER_H
#define APPMANAGER_H

#include "class/eventmonitor.h"
#include "class/hotkey.h"
#include "dialog/toolwindow.h"
#undef Bool
#undef Unsorted
#include <QList>
#include <QObject>
#include <QString>

class AppManager : public QObject {
  Q_OBJECT
public:
  explicit AppManager(QObject *parent = nullptr);
  ~AppManager();

  static AppManager *instance();

public:
  Hotkey *registerHotkey(QKeySequence &keyseq, bool isHostHotkey);
  bool enableHotKey(int index, bool enabled = true);
  bool enableHotKey(Hotkey *hotkey, bool enabled = true);
  bool unregisterHotkey(Hotkey *hotkey);
  bool unregisterHotkey(int index);
  bool editHotkey(int index, QKeySequence &keyseq);
  bool editHotkey(Hotkey *hotkey, QKeySequence &keyseq);
  Hotkey *hotkey(int index);
  void clearHotkey();

signals:
  void buttonPress(EventMonitor::MouseButton btn, int x, int y);
  void buttonRelease(EventMonitor::MouseButton btn, int x, int y);
  void clicked(int x, int y);
  void doubleClicked(int x, int y);
  void mouseWheel(EventMonitor::MouseWheel direction);
  void mouseMove(int x, int y);
  void mouseDrag(int x, int y);

  void hotkeyTirggered(const Hotkey *hotkey);
  void hotkeyReleased(const Hotkey *hotkey);
  void hotkeyEnableChanged(bool value, const Hotkey *hotkey);

  void selectionTextChanged(const QString &selectedText);

private:
  EventMonitor monitor;
  QList<Hotkey *> hotkeys;
  QStringList execs;

  bool ignoremsg = false;

  static AppManager *m_instance;

  ToolWindow toolwin;

  QList<QKeySequence> registeredSeq; // 已注册的键盘序列
};

#endif // APPMANAGER_H
