#ifndef WINTOOLITEM_H
#define WINTOOLITEM_H

#include <DLabel>
#include <QPixmap>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class WinToolItem : public QWidget {
  Q_OBJECT
public:
  explicit WinToolItem(QWidget *parent = nullptr);
  explicit WinToolItem(const QPixmap pix, const QString &name,
                       const QString &srv, const QString &params,
                       QWidget *parent = nullptr);

public slots:
  void setIcon(const QPixmap pix);
  void setName(const QString &name);
  void setSrvName(const QString &srv);
  void setParams(const QString &params);

private:
  DLabel *lbl, *m_name, *m_srv, *m_param;
};

#endif // WINTOOLITEM_H
