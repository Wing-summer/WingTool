#include "appmanager.h"
#include <QApplication>
#include <QClipboard>
#include <QKeySequence>

AppManager *AppManager::m_instance = nullptr;

AppManager::AppManager(QObject *parent) : QObject(parent) {
  // 初始化全局鼠标监控

#define CONNECT(sig) connect(&monitor, SIGNAL(sig), SLOT(sig));
  CONNECT(clicked);
  CONNECT(doubleClicked);
  CONNECT(mouseWheel);
  CONNECT(mouseMove);

  connect(&monitor, &EventMonitor::buttonPress,
          [=](Qt::MouseButton btn, int x, int y) {
            if (this->checkToolShow(btn)) {
              toolwin.popup(QCursor::pos());
            }
            emit this->buttonPress(btn, x, y);
          });
  connect(&monitor, &EventMonitor::mouseDrag, [=](int x, int y) {
    if (this->toolwin.isVisible())
      this->toolwin.sendMousePosUpdated();
    emit this->mouseDrag(x, y);
  });
  connect(&monitor, &EventMonitor::buttonRelease,
          [=](Qt::MouseButton btn, int x, int y) {
            if (toolwin.isVisible())
              toolwin.finished();
            emit this->buttonRelease(btn, x, y);
          });

  monitor.start();

  connect(&toolwin, &ToolWindow::triggered, this,
          &AppManager::toolSelTriggered);

  // 存单实例
  m_instance = this;
}

AppManager::~AppManager() { clearHotkey(); }

AppManager *AppManager::instance() { return m_instance; }

Hotkey *AppManager::registerHotkey(QKeySequence &keyseq, bool isHostHotkey) {
  if (registeredSeq.contains(keyseq))
    return nullptr;
  auto hotkey = new Hotkey(isHostHotkey, keyseq, true);
  hotkeys.append(hotkey);
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

bool AppManager::enableHotKey(Hotkey *hotkey, bool enabled) {
  if (hotkey == nullptr)
    return false;
  return hotkey->setRegistered(enabled);
}

bool AppManager::unregisterHotkey(Hotkey *hotkey) {
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

bool AppManager::editHotkey(Hotkey *hotkey, QKeySequence &keyseq) {
  if (hotkey == nullptr)
    return false;
  auto i = registeredSeq.indexOf(hotkey->shortcut());
  if (i < 0)
    return false;
  registeredSeq[i] = keyseq;
  hotkey->setShortcut(keyseq, true);
  return true;
}

void AppManager::clearHotkey() {
  for (auto item : hotkeys) {
    delete item;
  }
  registeredSeq.clear();
  hotkeys.clear();
}

Qt::KeyboardModifier AppManager::getKeyModifier() const {
  return monitor.getKeyModifier();
}

void AppManager::setToolIcons(QVector<QIcon> icons) { toolwin.setIcons(icons); }

void AppManager::setToolIcon(int index, QIcon icon) {
  toolwin.setIcon(index, icon);
}
