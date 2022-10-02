#ifndef WINGAPPLICATION_H
#define WINGAPPLICATION_H

#include <DApplication>

DWIDGET_USE_NAMESPACE

class WingApplication : public DApplication {
  Q_OBJECT
public:
  WingApplication(int &argc, char **argv);

private:
  bool notify(QObject *obj, QEvent *event) override;
};

#endif // WINGAPPLICATION_H
