#ifndef HOTKEY_H
#define HOTKEY_H

#include "QHotkey/qhotkey.h"

class Hotkey : public QHotkey {
public:
  Hotkey(bool isHostHotkey, const QKeySequence &shortcut,
         bool autoRegister = false, QObject *parent = nullptr);

  bool isHostHotkey() const;

private:
  bool m_isHostHotkey;
};

#endif // HOTKEY_H
