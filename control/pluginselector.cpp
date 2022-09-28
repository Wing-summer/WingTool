#include "pluginselector.h"
#include "utilities.h"

PluginSelector::PluginSelector(QWidget *parent) : DPushButton(parent) {
  plgsys = PluginSystem::instance();
  setText("/");
  setIcon(ICONRES("plugin"));

  connect(this, &PluginSelector::clicked, this,
          [=](bool) { this->selectPlugin(); });
}

void PluginSelector::selectPlugin() {
  PluginSelDialog d;
  // 此时返回的是插件索引
  auto index = d.exec();
  if (index == -2) // -2 表示用户点击了取消按钮
    return;
  if (index >= 0) {
    auto plg = plgsys->plugin(index);
    setIcon(Utilities::processPluginIcon(plg));
    setText(plg->pluginName());
  } else {
    setText("/");
    setIcon(ICONRES("plugin"));
  }
  selplgindex = index;
  emit finished();
}

int PluginSelector::getSelectedIndex() { return selplgindex; }

IWingToolPlg *PluginSelector::getSelectedPlg() {
  return plgsys->plugin(selplgindex);
}

bool PluginSelector::setSelectedIndex(int index) {
  if (index < 0 || index >= plgsys->pluginCounts())
    return false;
  selplgindex = index;
  auto plg = plgsys->plugin(index);
  setIcon(Utilities::processPluginIcon(plg));
  setText(plg->pluginName());
  return true;
}
