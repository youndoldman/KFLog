/***********************************************************************
 **
 **   taskdialog.cpp
 **
 **   This file is part of KFLog2.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by Harald Maier
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>

#include <klocale.h>
#include <kseparator.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kconfig.h>

#include "taskdialog.h"
#include "mapcontents.h"

TaskDialog::TaskDialog(QWidget *parent, const char *name )
  : KDialog(parent, name, true)
{
  setCaption(i18n("Task definition"));
  __initDialog();
  setMinimumWidth(500);
  setMinimumHeight(300);
  pTask = 0;
}

TaskDialog::~TaskDialog()
{
}

/** No descriptions */
void TaskDialog::__initDialog()
{
  QLabel *l;
  QPushButton *b;

  QVBoxLayout *topLayout = new QVBoxLayout(this, 10);
  QHBoxLayout *header = new QHBoxLayout(10);
  QHBoxLayout *type = new QHBoxLayout(10);
  QHBoxLayout *buttons = new QHBoxLayout(10);
  QVBoxLayout *leftLayout = new QVBoxLayout(5);
  QVBoxLayout *middleLayout = new QVBoxLayout(5);
  QVBoxLayout *rightLayout = new QVBoxLayout(5);
  QHBoxLayout *topGroup = new QHBoxLayout(10);
  QHBoxLayout *smallButtons = new QHBoxLayout(5);

  buttons->addStretch();
  b = new QPushButton(i18n("&Ok"), this);
  b->setDefault(true);
  connect(b, SIGNAL(clicked()), SLOT(accept()));
  buttons->addWidget(b);
  b = new QPushButton(i18n("&Cancel"), this);
  connect(b, SIGNAL(clicked()), SLOT(reject()));
  buttons->addWidget(b);

  name = new QLineEdit(this);
  name->setReadOnly(true);
  l = new QLabel(name, i18n("&Name") + ":", this);
  header->addWidget(l);
  header->addWidget(name);

  // Create an exclusive button group
  QButtonGroup *bgrp1 = new QButtonGroup(1, QGroupBox::Vertical, i18n("Planning"), this);
  connect(bgrp1, SIGNAL(clicked(int)), SLOT(slotSetPlanningType(int)));
  bgrp1->setExclusive(true);

  // insert 2 radiobuttons
  routeBased = new QRadioButton(i18n( "&Route based"), bgrp1);
  routeBased->setChecked(true);
  areaBased = new QRadioButton(i18n("&Area based"), bgrp1);

  // Create an non-exclusive button group
  QButtonGroup *bgrp2 = new QButtonGroup(1, QGroupBox::Vertical, i18n("Side of FAI area"), this);
  connect(bgrp2, SIGNAL(clicked(int)), SLOT(slotSetPlanningDirection(int)));
  bgrp2->setExclusive(false);

  // insert 2 checkbuttons
  left = new QCheckBox(i18n( "&left"), bgrp2);
  left->setChecked(true);
  right = new QCheckBox(i18n("ri&ght"), bgrp2);

  taskType = new QLabel(this);
  taskType->setMinimumWidth(100);

  type->addWidget(bgrp1);
  type->addWidget(bgrp2);
  type->addStretch();
  type->addWidget(taskType);

  route =  new KFLogListView("Route", this, "route");
  route->setShowSortIndicator(false);
  route->setSorting(-1);
  route->setAllColumnsShowFocus(true);

  colType = route->addColumn(i18n("Type"));
  colWaypoint = route->addColumn(i18n("Waypoint"));
  colDist = route->addColumn(i18n("Distance"));

  l = new QLabel(route, i18n("&Task"), this);

  route->loadConfig();

  smallButtons->addStretch();
  b = new QPushButton(this);
  b->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("up", KIcon::NoGroup, KIcon::SizeSmall));
  connect(b, SIGNAL(clicked()), SLOT(slotMoveUp()));
  smallButtons->addWidget(b);
  b = new QPushButton(this);
  b->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("down", KIcon::NoGroup, KIcon::SizeSmall));
  connect(b, SIGNAL(clicked()), SLOT(slotMoveDown()));
  smallButtons->addWidget(b);
  smallButtons->addStretch();

  leftLayout->addWidget(l);
  leftLayout->addWidget(route);
  leftLayout->addLayout(smallButtons);

  middleLayout->addStretch();
  back = new QPushButton(this);
  back->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("back", KIcon::NoGroup, KIcon::SizeSmall));
  connect(back, SIGNAL(clicked()), SLOT(slotAddWaypoint()));
  middleLayout->addWidget(back);
  b = new QPushButton(this);
  b->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("reload", KIcon::NoGroup, KIcon::SizeSmall));
  connect(b, SIGNAL(clicked()), SLOT(slotReplaceWaypoint()));
  middleLayout->addWidget(b);
  forward = new QPushButton(this);
  forward->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("forward", KIcon::NoGroup, KIcon::SizeSmall));
  connect(forward, SIGNAL(clicked()), SLOT(slotRemoveWaypoint()));
  middleLayout->addWidget(forward);
  middleLayout->addStretch();

  waypoints = new KListBox(this, "waypoints");
  l = new QLabel(waypoints, i18n("&Waypoint's"), this);
  rightLayout->addWidget(l);
  rightLayout->addWidget(waypoints);

  topGroup->addLayout(leftLayout);
  topGroup->addLayout(middleLayout);
  topGroup->addLayout(rightLayout);

  topLayout->addLayout(header);
  topLayout->addLayout(type);
  topLayout->addLayout(topGroup);
  topLayout->addWidget(new KSeparator(this));
  topLayout->addLayout(buttons);
}

/** No descriptions */
void TaskDialog::polish()
{
  KDialog::polish();

  if (pTask == 0) {
    KMessageBox::error(this, "INTERNAL ERROR:\nNo task set!");
    reject();
  }

  // load current waypoint list from mapcontents
  extern MapContents _globalMapContents;
  QList<wayPoint>* wpList = _globalMapContents.getWaypointList();
  wayPoint *wp;
  waypoints->clear();
  QString t;
  for (wp = wpList->first(); wp; wp = wpList->next()) {
    if (!wp->description.isEmpty()) {
      t = wp->description;
    }
    else {
      t = wp->name;
    }
    waypoints->insertItem(t);
    waypointDict.insert(t, wp);
  }
  waypoints->sort();
}

void TaskDialog::slotSetPlanningType(int)
{
  unsigned int n, cnt;

  if (areaBased->isChecked()) {
    cnt = wpList.count();
    
    left->setEnabled(true);
    right->setEnabled(true);
    left->setChecked(pTask->getPlanningDirection() & FlightTask::leftOfRoute);
    right->setChecked(pTask->getPlanningDirection() & FlightTask::rightOfRoute);
    if (cnt > 5) {
      // remove route points
      for(n = cnt - 3; n >= 2; n--) {
        wpList.take(n);
      }
      pTask->setWaypointList(wpList);
    }        
    pTask->setPlanningType(FlightTask::AreaBased);
  }
  else {
    left->setEnabled(false);
    right->setEnabled(false);
    left->setChecked(false);
    right->setChecked(false);

    pTask->setPlanningType(FlightTask::RouteBased);
  }

  fillWaypoints();
  enableWaypointButtons();
  route->setSelected(route->firstChild(), true);
}

void TaskDialog::slotSetPlanningDirection(int)
{
  int dir = 0;

  if (left->isChecked()) {
    dir |= FlightTask::leftOfRoute;
  }

  if (right->isChecked()) {
    dir |= FlightTask::rightOfRoute;
  }

  pTask->setPlanningDirection(dir);
}

void TaskDialog::fillWaypoints()
{
  wayPoint *wp;
  QListViewItem *item, *lastItem = 0;
  QString txt;

  route->clear();
  for (wp = wpList.first(); wp; wp = wpList.next()) {
    item = new QListViewItem(route, lastItem);
    switch (wp->type) {
    case FlightTask::TakeOff:
      txt = i18n("Take Off");
      break;
    case FlightTask::Begin:
      txt = i18n("Begin of Task");
      break;
    case FlightTask::RouteP:
      txt = i18n("Route Point");
      break;
    case FlightTask::End:
      txt = i18n("End of Task");
      break;
    case FlightTask::FreeP:
      txt = i18n("Free Point");
      break;
    case FlightTask::Landing:
      txt = i18n("Landing");
      break;
    default:
      txt = "???";
    }
    item->setText(colType, txt);
    item->setText(colWaypoint, wp->name);

    if (lastItem) {
      txt.sprintf("%.2f km", wp->distance);
      item->setText(colDist, txt);
    }
    lastItem = item;
  }

  taskType->setText(pTask->getTaskTypeString());
}

void TaskDialog::slotMoveUp()
{
  int curPos = route->itemIndex(route->currentItem());
  wayPoint *wp;

  if (curPos) {
    wp = wpList.take(curPos);
    wpList.insert(curPos - 1, wp);
    pTask->setWaypointList(wpList);
    fillWaypoints();
    route->setSelected(route->itemAtIndex(curPos - 1), true);
  }
}

void TaskDialog::slotMoveDown()
{
  unsigned int curPos = route->itemIndex(route->currentItem());
  wayPoint *wp;

  if (curPos < wpList.count() - 1) {
    wp = wpList.take(curPos);
    wpList.insert(curPos + 1, wp);
    pTask->setWaypointList(wpList);
    fillWaypoints();
    route->setSelected(route->itemAtIndex(curPos + 1), true);
  }
}

void TaskDialog::slotReplaceWaypoint()
{
  unsigned int cnt = wpList.count();
  unsigned int curPos = route->itemIndex(route->currentItem());
  QString selText = waypoints->currentText();

  if (!selText.isEmpty()) {
    wpList.remove(curPos);
    struct wayPoint *wp = new wayPoint;
    *wp = *waypointDict[selText];
     wp->sector1 = 0;
     wp->sector2 = 0;
     wp->sectorFAI = 0;

    wpList.insert(curPos, wp);
    pTask->setWaypointList(wpList);
    fillWaypoints();
    route->setSelected(route->itemAtIndex(curPos + 1 < cnt ? curPos + 1 : cnt), true);
  }
}

void TaskDialog::slotAddWaypoint()
{
  unsigned int cnt = wpList.count();
  QString selText = waypoints->currentText();

  if (!selText.isEmpty()) {
    struct wayPoint *wp = new wayPoint;
    *wp = *waypointDict[selText];
     wp->sector1 = 0;
     wp->sector2 = 0;
     wp->sectorFAI = 0;

    wpList.insert(cnt - 2, wp);
    pTask->setWaypointList(wpList);
    fillWaypoints();
    route->setSelected(route->itemAtIndex(cnt - 1), true);
    enableWaypointButtons();
  }
}

void TaskDialog::slotRemoveWaypoint()
{
  unsigned int cnt = wpList.count();
  unsigned int curPos = route->itemIndex(route->currentItem());

  if (cnt > 4) {
    if (curPos > 1 && curPos < cnt - 2) {
      wpList.remove(curPos);
      pTask->setWaypointList(wpList);
      fillWaypoints();
      route->setSelected(route->itemAtIndex(curPos), true);
      enableWaypointButtons();
    }
  }
}

void TaskDialog::setTask(FlightTask *orig)
{
  KConfig* config = KGlobal::config();
  config->setGroup("Map Data");
  extern MapMatrix _globalMapMatrix;

  if (pTask == 0) {
    pTask = new FlightTask(orig->getFileName());
  }

  // make a work copy of the task with at least 4 points
  wpList = orig->getWPList();
  wayPoint *wp;
  if (wpList.count() < 4) {
    for (unsigned int i = wpList.count(); i < 4; i++) {
      wp = new wayPoint;
      wp->origP.setLat(config->readNumEntry("Homesite Latitude"));
      wp->origP.setLon(config->readNumEntry("Homesite Longitude"));
      wp->projP = _globalMapMatrix.wgsToMap(wp->origP);
      wp->name = config->readEntry("Homesite").left(6).upper();
      wp->sector1 = 0;
      wp->sector2 = 0;
      wp->sectorFAI = 0;

      wpList.append(wp);
    }
  }
  pTask->setWaypointList(wpList);
  pTask->setPlanningType(orig->getPlanningType());
  pTask->setPlanningDirection(orig->getPlanningDirection());

  name->setText(pTask->getFileName());
  switch (pTask->getPlanningType()) {
  case FlightTask::RouteBased:
    routeBased->setChecked(true);
    break;
  case FlightTask::AreaBased:
    areaBased->setChecked(true);
    break;
  }

  left->setChecked(pTask->getPlanningDirection() & FlightTask::leftOfRoute);
  right->setChecked(pTask->getPlanningDirection() & FlightTask::rightOfRoute);

  slotSetPlanningType(-1);
}

/** No descriptions */
void TaskDialog::enableWaypointButtons()
{
  switch (pTask->getPlanningType()) {
  case FlightTask::RouteBased:
    forward->setEnabled(true);
    back->setEnabled(true);
    break;
  case FlightTask::AreaBased:
    int cnt = wpList.count();
    forward->setEnabled(cnt > 4);
    back->setEnabled(cnt == 4);
    break;
  }
}