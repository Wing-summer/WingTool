/***********************************************************/
// 此代码时基于 Deepin 的取色应用进行定制修改而成，GPL 协议
// 原来的代码无法直接进行编译，就算配置了对应的环境，本人（寂静的羽夏）
// 对此进行了大量资料的查阅修复了该问题，并做了自己的修改
/***********************************************************/

#ifndef EVENTMONITOR_H
#define EVENTMONITOR_H

#include <QCursor>
#include <QDataStream>
#include <QEvent>
#include <QTextStream>
#include <QThread>

/*== X11 头文件有和 QT 头文件冲突的地方 ==*/
// 所以 undef 使其编译通过
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/extensions/record.h>
#undef Unsorted
#undef None
#undef Bool
#undef Data
/*==================================*/

// Virtual button codes that are not defined by X11.
#define Button1 1
#define Button2 2
#define Button3 3
#define WheelUp 4
#define WheelDown 5
#define WheelLeft 6
#define WheelRight 7
#define XButton_1 8
#define XButton_2 9

class EventMonitor : public QThread {
  Q_OBJECT

public:
  enum class MouseWheel { None, Up, Down, Left, Right };
  Q_ENUM(MouseWheel)

public:
  EventMonitor(QObject *parent = nullptr);
  ~EventMonitor() override;

  Qt::KeyboardModifier getKeyModifier() const;

signals:
  void buttonPress(Qt::MouseButton btn, int x, int y); // 当鼠标按键被按下时
  void buttonRelease(Qt::MouseButton btn, int x, int y); // 当鼠标按键被释放时
  void clicked(int x, int y);            // 当鼠标进行单击操作时
  void doubleClicked(int x, int y);      // 当鼠标进行双击操作时
  void mouseWheel(MouseWheel direction); // 当鼠标滚轮滚动时
  void mouseMove(int x, int y);          // 当鼠标移动时
  void mouseDrag(int x, int y);          // 当鼠标拖拽时

protected:
  static void callback(XPointer trash, XRecordInterceptData *data);
  bool handleWheelEvent(int detail);
  void handleRecordEvent(XRecordInterceptData *);
  void run() override;

private:
  bool isPress, isClicked;
  XRecordContext context;
  Display *display_datalink;

  std::chrono::system_clock::time_point clickbefore;

  Qt::KeyboardModifier keyModifier;
};

#endif
