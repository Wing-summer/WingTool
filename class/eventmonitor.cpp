#include "eventmonitor.h"
#include <QApplication>
#include <X11/Xlibint.h>

EventMonitor::EventMonitor(QObject *parent) : QThread(parent) {
  isPress = false;
}

EventMonitor::~EventMonitor() {
  Display *display = XOpenDisplay(nullptr);
  XRecordDisableContext(display, context);
  XSync(display, 0);
  wait();
  XRecordFreeContext(display, context);
  XCloseDisplay(display_datalink);
}

void EventMonitor::run() {
  // Init x11 display.
  Display *display = XOpenDisplay(nullptr);
  if (display == nullptr) {
    fprintf(stderr, "unable to open display\n");
    return;
  }

  // Receive from ALL clients, including future clients.
  XRecordClientSpec clients = XRecordAllClients;
  XRecordRange *range = XRecordAllocRange();
  if (range == nullptr) {
    fprintf(stderr, "unable to allocate XRecordRange\n");
    return;
  }

  // Receive ButtonPress, ButtonRelease and MotionNotify
  // events.
  memset(range, 0, sizeof(XRecordRange));
  range->device_events.first = ButtonPress;
  range->device_events.last = MotionNotify;

  // And create the XRECORD context.
  context = XRecordCreateContext(display, 0, &clients, 1, &range, 1);
  if (context == 0) {
    fprintf(stderr, "XRecordCreateContext failed\n");
    return;
  }
  XFree(range);

  // Sync x11 display message.
  XSync(display, True);

  display_datalink = XOpenDisplay(nullptr);
  if (display_datalink == nullptr) {
    fprintf(stderr, "unable to open second display\n");
    return;
  }

  // Enter in xrecord event loop.
  if (!XRecordEnableContext(display_datalink, context, callback,
                            reinterpret_cast<XPointer>(this))) {
    fprintf(stderr, "XRecordEnableContext() failed\n");
    return;
  }

  // init timer
  clickbefore = std::chrono::system_clock::now();
  isClicked = false;
}

void EventMonitor::callback(XPointer ptr, XRecordInterceptData *data) {
  (reinterpret_cast<EventMonitor *>(ptr))->handleRecordEvent(data);
}

void EventMonitor::handleRecordEvent(XRecordInterceptData *data) {
  if (data->category == XRecordFromServer) {
    auto event = reinterpret_cast<xEvent *>(data->data);

    switch (event->u.u.type) {
    case ButtonPress:
      if (handleWheelEvent(event->u.u.detail)) {
        isPress = true;
        MouseButton btn = MouseButton::NoneButton;
        switch (event->u.u.detail) {
        case Button1: {
          btn = MouseButton::LeftButton;

          auto clicknow = std::chrono::system_clock::now();
          double diff_ms =
              std::chrono::duration<double, std::milli>(clicknow - clickbefore)
                  .count();
          clickbefore = clicknow;
          if (isClicked && diff_ms <= QApplication::doubleClickInterval()) {
            emit doubleClicked(event->u.keyButtonPointer.rootX,
                               event->u.keyButtonPointer.rootY);
            isClicked = false;
          } else {
            emit clicked(event->u.keyButtonPointer.rootX,
                         event->u.keyButtonPointer.rootY);
            isClicked = true;
          }

        } break;
        case Button2:
          btn = MouseButton::MiddleButton;
          break;
        case Button3:
          btn = MouseButton::RightButton;
          break;
        case XButton1:
          btn = MouseButton::XButton_1;
          break;
        case XButton2:
          btn = MouseButton::XButton_2;
          break;
        }

        emit buttonPress(btn, event->u.keyButtonPointer.rootX,
                         event->u.keyButtonPointer.rootY);
      }
      break;
    case MotionNotify:
      if (isPress) {
        emit mouseDrag(event->u.keyButtonPointer.rootX,
                       event->u.keyButtonPointer.rootY);
      } else {
        emit mouseMove(event->u.keyButtonPointer.rootX,
                       event->u.keyButtonPointer.rootY);
      }
      break;
    case ButtonRelease:
      if (handleWheelEvent(event->u.u.detail)) {
        isPress = false;

        MouseButton btn = MouseButton::NoneButton;
        switch (event->u.u.detail) {
        case Button1:
          btn = MouseButton::LeftButton;
          break;
        case Button2:
          btn = MouseButton::MiddleButton;
          break;
        case Button3:
          btn = MouseButton::RightButton;
          break;
        case XButton1:
          btn = MouseButton::XButton_1;
          break;
        case XButton2:
          btn = MouseButton::XButton_2;
          break;
        }
        emit buttonRelease(btn, event->u.keyButtonPointer.rootX,
                           event->u.keyButtonPointer.rootY);
      }
      break;
    default:
      break;
    }
  }

  fflush(stdout);
  XRecordFreeData(data);
}

bool EventMonitor::handleWheelEvent(int detail) {
  MouseWheel wheel = MouseWheel::None;
  switch (detail) {
  case WheelUp:
    wheel = MouseWheel::Up;
    break;
  case WheelDown:
    wheel = MouseWheel::Down;
    break;
  case WheelLeft:
    wheel = MouseWheel::Left;
    break;
  case WheelRight:
    wheel = MouseWheel::Right;
    break;
  default:
    return true;
  }
  emit mouseWheel(wheel);
  return false;
}
