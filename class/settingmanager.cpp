#include "settingmanager.h"
#include <QApplication>
#include <QFile>
#include <QStandardPaths>

SettingManager *SettingManager::m_instance = nullptr;

SettingManager::SettingManager(QObject *parent) : QObject(parent) {
  m_instance = this;
}

SettingManager *SettingManager::instance() { return m_instance; }

bool SettingManager::loadSettings() {
  QString strConfigPath =
      QString("%1/%2/%3/config.conf")
          .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
          .arg(qApp->organizationName())
          .arg(qApp->applicationName());

  QFile f(strConfigPath);
  if (f.open(QFile::ReadOnly)) {
    emit this->loadingFinish();
  }
  return true;
}

bool SettingManager::saveSettings() {
  QString strConfigPath =
      QString("%1/%2/%3/config.conf")
          .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
          .arg(qApp->organizationName())
          .arg(qApp->applicationName());
  return exportSettings(strConfigPath);
}

bool SettingManager::exportSettings(QString filename) {
  QFile f(filename);
  QDataStream stream(&f);
  if (f.open(QFile::WriteOnly)) {

    return true;
  }
  return false;
}

int SettingManager::toolGridSize() { return m_toolGridSize; }

void SettingManager::setToolGridSize(int v) {
  if (v > 0) {
    m_toolGridSize = v;
    emit sigToolGridSizeChanged(v);
  }
}

void SettingManager::getHokeysBuffer(QList<Hotkey *> &hotkeysBuf,
                                     QMap<Hotkey *, ToolStructInfo> &buffer) {}

void SettingManager::getToolLeftBuffer(ToolStructInfo buffer[]) {}

void SettingManager::getToolRightBuffer(QList<ToolStructInfo> &buffer) {}
