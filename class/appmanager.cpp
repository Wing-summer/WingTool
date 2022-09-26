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
  auto hotkey = new QHotkey(keyseq, true);
  hotkeys += hotkey;
  connect(hotkey, &QHotkey::activated, this,
          [=] { emit this->hotkeyTirggered(hotkey, hotkeys.indexOf(hotkey)); });
  connect(hotkey, &QHotkey::released, this,
          [=] { emit this->hotkeyReleased(hotkey, hotkeys.indexOf(hotkey)); });
  connect(hotkey, &QHotkey::registeredChanged, this, [=](bool registered) {
    emit this->hotkeyEnableChanged(registered, hotkey, hotkeys.indexOf(hotkey));
  });
  return hotkey;
}

bool AppManager::enableHotKey(int index, bool enabled) {
  if (index < 0 || index >= hotkeys.count())
    return false;
  hotkeys[index]->setRegistered(enabled);
  return true;
}

bool AppManager::unregisterHotkey(QHotkey *hotkey) {
  auto i = hotkeys.indexOf(hotkey);
  if (i < 0)
    return false;
  hotkeys.removeAt(i);
  hotkey->disconnect();
  delete hotkey;
  return true;
}

bool AppManager::unregisterHotkey(int index) {
  if (index < 0 || index >= hotkeys.count())
    return false;
  auto del = hotkeys[index];
  del->disconnect();
  hotkeys.removeAt(index);
  delete del;
  return true;
}

bool AppManager::editHotkey(int index, QKeySequence &keyseq) {
  if (index < 0 || index >= hotkeys.count())
    return false;
  auto del = hotkeys[index];
  del->setShortcut(keyseq, true);
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
  hotkeys.clear();
}
