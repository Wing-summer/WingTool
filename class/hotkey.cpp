#include "hotkey.h"

Hotkey::Hotkey(bool isHostHotkey, const QKeySequence &shortcut,
               bool autoRegister, QObject *parent)
    : QHotkey(shortcut, autoRegister, parent), m_isHostHotkey(isHostHotkey) {}

bool Hotkey::isHostHotkey() const { return m_isHostHotkey; }
