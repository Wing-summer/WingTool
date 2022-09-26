#ifndef TOOLWINDOW_H
#define TOOLWINDOW_H

#include <DDialog>
#include <DIconButton>
#include <QGridLayout>
#include <QObject>

DWIDGET_USE_NAMESPACE

class ToolWindow : public DDialog {
  Q_OBJECT
public:
  ToolWindow(DDialog *parent = nullptr);

public slots:
  void setIcons(QVector<QIcon> icons);
  void setIcon(int index, QIcon icon);

  void popup(QPoint pos);

  // 这个函数是用来提供 UI 鼠标交互和选择情况的，mouseMoveEvent 不触发就挺烦
  // 最后只能用自己封装好的鼠标 Hook 进行通知
  void sendMousePosUpdated();

private:
  QGridLayout *mlayout;
  DIconButton *lbls[9] = {nullptr};
};

#endif // TOOLWINDOW_H
