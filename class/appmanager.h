#ifndef APPMANAGER_H
#define APPMANAGER_H

#include "QHotkey/qhotkey.h"
#include "class/eventmonitor.h"
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
  QHotkey *registerHotkey(QKeySequence &keyseq);
  bool enableHotKey(int index, bool enabled = true);
  bool unregisterHotkey(QHotkey *hotkey);
  bool unregisterHotkey(int index);
  bool editHotkey(int index, QKeySequence &keyseq);
  QHotkey *hotkey(int index);
  void clearHotkey();

signals:
  void buttonPress(EventMonitor::MouseButton btn, int x, int y);
  void buttonRelease(EventMonitor::MouseButton btn, int x, int y);
  void clicked(int x, int y);
  void doubleClicked(int x, int y);
  void mouseWheel(EventMonitor::MouseWheel direction);
  void mouseMove(int x, int y);
  void mouseDrag(int x, int y);

  void hotkeyTirggered(const QHotkey *hotkey, int index);
  void hotkeyReleased(const QHotkey *hotkey, int index);
  void hotkeyEnableChanged(bool value, const QHotkey *hotkey, int index);

  void selectionTextChanged(const QString &selectedText);

private:
  EventMonitor monitor;
  QList<QHotkey *> hotkeys;
  QStringList execs;

  bool ignoremsg = false;

  static AppManager *m_instance;

  ToolWindow toolwin;

  QList<QKeySequence> registeredSeq; // 已注册的键盘序列
};

#endif // APPMANAGER_H
