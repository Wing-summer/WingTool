#include "wintoolitem.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

WinToolItem::WinToolItem(QWidget *parent) : QWidget(parent) {
  setFixedHeight(30);
  auto layout = new QHBoxLayout(this);
  lbl = new DLabel(this);
  lbl->setFixedSize(25, 25);
  lbl->setScaledContents(true);
  layout->addWidget(lbl);
  auto ilayout = new QVBoxLayout(this);
  layout->addLayout(ilayout, 1);
  m_name = new DLabel(this);
  ilayout->addWidget(m_name);
  m_srv = new DLabel(this);
  ilayout->addWidget(m_srv);
  m_param = new DLabel(this);
  ilayout->addWidget(m_param);
}

WinToolItem::WinToolItem(const QPixmap pix, const QString &name,
                         const QString &srv, const QString &params,
                         QWidget *parent)
    : QWidget(parent) {
  setFixedHeight(30);
  auto layout = new QHBoxLayout(this);
  lbl = new DLabel(this);
  lbl->setPixmap(pix);
  lbl->setFixedSize(25, 25);
  lbl->setScaledContents(true);
  layout->addWidget(lbl);
  auto ilayout = new QVBoxLayout(this);
  layout->addLayout(ilayout, 1);
  m_name = new DLabel(name, this);
  ilayout->addWidget(m_name);
  m_srv = new DLabel(srv, this);
  ilayout->addWidget(m_srv);
  m_param = new DLabel(params, this);
  ilayout->addWidget(m_param);
}

void WinToolItem::setIcon(QPixmap pix) { lbl->setPixmap(pix); }

void WinToolItem::setName(const QString &name) { m_name->setText(name); }

void WinToolItem::setSrvName(const QString &srv) { m_srv->setText(srv); }

void WinToolItem::setParams(const QString &params) { m_param->setText(params); }
