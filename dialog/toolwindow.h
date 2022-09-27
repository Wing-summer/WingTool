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

  // 表示选择结束
  void finished();

signals:
  // 当选中有效任务时会触发
  // 注：当 index >= 4 时，会自动减1，即为实际任务索引
  // 也就是说，收到信号无需转化
  void triggered(int index);

private:
  QGridLayout *mlayout;
  DIconButton *lbls[9] = {nullptr};

  QPoint m_sel;

  int gridtotal;
};

#endif // TOOLWINDOW_H
