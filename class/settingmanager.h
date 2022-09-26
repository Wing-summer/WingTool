#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include <QObject>

class SettingManager : public QObject {
  Q_OBJECT
public:
  explicit SettingManager(QObject *parent = nullptr);

private:
};

#endif // SETTINGMANAGER_H
