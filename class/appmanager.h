#ifndef APPMANAGER_H
#define APPMANAGER_H

#include "class/eventmonitor.h"
#include "class/hotkey.h"
#include "dialog/toolboxwindow.h"
#include "dialog/toolwindow.h"
#include "utilities.h"
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
  bool enableHotKey(Hotkey *hotkey, bool enabled = true);
  bool unregisterHotkey(Hotkey *hotkey);
  bool editHotkey(Hotkey *hotkey, QKeySequence &keyseq);
  void clearHotkey();

  Qt::KeyboardModifiers getKeyModifiers() const;
  Qt::MouseButtons getMouseButtons() const;
  bool isRegistered(QKeySequence &seq) const;

signals:
  void buttonPress(Qt::MouseButton btn, int x, int y);
  void buttonRelease(Qt::MouseButton btn, int x, int y);
  void clicked(int x, int y);
  void doubleClicked(int x, int y);
  void mouseWheel(EventMonitor::MouseWheel direction);
  void mouseMove(int x, int y);
  void mouseDrag(int x, int y);

  void hotkeyTirggered(const Hotkey *hotkey);
  void hotkeyReleased(const Hotkey *hotkey);
  void hotkeyEnableChanged(bool value, const Hotkey *hotkey);

  bool checkToolShow(Qt::MouseButton btn);

  void toolSelTriggered(int index);

public slots:
  void setToolIcon(int index, QIcon icon, QString tip);

private:
  EventMonitor monitor;
  QList<Hotkey *> hotkeys;

  static AppManager *m_instance;

  ToolWindow toolwin;

  QList<QKeySequence> registeredSeq; // 已注册的键盘序列
};

#endif // APPMANAGER_H
