#include "appmanager.h"
#include <QApplication>
#include <QClipboard>

AppManager *AppManager::m_instance = nullptr;

AppManager::AppManager(QObject *parent) : QObject(parent) {

  // 初始化选词监控
  auto clipboard = qApp->clipboard();
  connect(clipboard, &QClipboard::selectionChanged, this, [=] {
    // 防止过多的消息干扰
    if (!ignoremsg) {
      emit this->selectionTextChanged(clipboard->text(QClipboard::Selection));
    }
  });

  // 初始化全局鼠标监控

#define CONNECT(sig) connect(&monitor, SIGNAL(sig), SLOT(sig));
  CONNECT(clicked);
  CONNECT(doubleClicked);
  CONNECT(mouseWheel);
  CONNECT(mouseMove);

  connect(&monitor, &EventMonitor::buttonPress,
          [=](EventMonitor::MouseButton btn, int x, int y) {
            if (btn == EventMonitor::MouseButton::MiddleButton) {
              toolwin.popup(QCursor::pos());
            }
            emit this->buttonPress(btn, x, y);
          });
  connect(&monitor, &EventMonitor::mouseDrag, [=](int x, int y) {
    ignoremsg = false;
    if (this->toolwin.isVisible())
      this->toolwin.sendMousePosUpdated();
    emit this->mouseDrag(x, y);
  });
  connect(&monitor, &EventMonitor::buttonRelease,
          [=](EventMonitor::MouseButton btn, int x, int y) {
            toolwin.hide();
            ignoremsg = true;
            emit this->buttonRelease(btn, x, y);
          });

  monitor.start();

  // 初始化热键存储

  // 存单实例
  m_instance = this;
}

AppManager::~AppManager() { clearHotkey(); }

AppManager *AppManager::instance() { return m_instance; }

QHotkey *AppManager::registerHotkey(QKeySequence &keyseq) {
  if (registeredSeq.contains(keyseq))
    return nullptr;
  auto hotkey = new QHotkey(keyseq, true);
  hotkeys += hotkey;
  connect(hotkey, &QHotkey::activated, this,
          [=] { emit this->hotkeyTirggered(hotkey); });
  connect(hotkey, &QHotkey::released, this,
          [=] { emit this->hotkeyReleased(hotkey); });
  connect(hotkey, &QHotkey::registeredChanged, this, [=](bool registered) {
    emit this->hotkeyEnableChanged(registered, hotkey);
  });
  registeredSeq.append(keyseq);
  return hotkey;
}

bool AppManager::enableHotKey(int index, bool enabled) {
  if (index < 0 || index >= hotkeys.count())
    return false;
  return hotkeys[index]->setRegistered(enabled);
}

bool AppManager::enableHotKey(QHotkey *hotkey, bool enabled) {
  if (hotkey == nullptr)
    return false;
  return hotkey->setRegistered(enabled);
}

bool AppManager::unregisterHotkey(QHotkey *hotkey) {
  auto i = hotkeys.indexOf(hotkey);
  if (i < 0)
    return false;
  registeredSeq.removeOne(hotkey->shortcut());
  // 由于保证了热键序列唯一性，只需找到一个删除就没了
  hotkey->disconnect();
  hotkeys.removeAt(i);
  delete hotkey;
  return true;
}

bool AppManager::unregisterHotkey(int index) {
  if (index < 0 || index >= hotkeys.count())
    return false;
  auto del = hotkeys[index];
  registeredSeq.removeOne(del->shortcut());
  // 由于保证了热键序列唯一性，只需找到一个删除就没了
  del->disconnect();
  hotkeys.removeAt(index);
  delete del;
  return true;
}

bool AppManager::editHotkey(int index, QKeySequence &keyseq) {
  if (index < 0 || index >= hotkeys.count())
    return false;
  if (registeredSeq.contains(keyseq))
    return false;
  auto del = hotkeys[index];
  registeredSeq[registeredSeq.indexOf(del->shortcut())] = keyseq;
  del->setShortcut(keyseq, true);
  return true;
}

bool AppManager::editHotkey(QHotkey *hotkey, QKeySequence &keyseq) {
  if (hotkey == nullptr)
    return false;
  auto i = registeredSeq.indexOf(hotkey->shortcut());
  if (i < 0)
    return false;
  registeredSeq[i] = keyseq;
  hotkey->setShortcut(keyseq, true);
  return true;
}

QHotkey *AppManager::hotkey(int index) {
  if (index < 0 || index >= hotkeys.count())
    return nullptr;
  return hotkeys[index];
}

void AppManager::clearHotkey() {
  for (auto item : hotkeys) {
    delete item;
  }
  registeredSeq.clear();
  hotkeys.clear();
}
