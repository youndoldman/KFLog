/***********************************************************************
**
**   waypointdict.cpp
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
#include <kmessagebox.h>
#include <kdeversion.h>
#if KDE_IS_VERSION(3,2,0)
  #undef OLDVERSION   //workaround for older KDE versions that lack kinputdialog
#else
  #define OLDVERSION 1    //workaround for older KDE versions that lack kinputdialog
#endif

#ifndef OLDVERSION
  #include "kinputdialog.h"
#endif


#include "waypointdict.h"

WaypointDict::WaypointDict()
{
  setAutoDelete(true);
}

WaypointDict::~WaypointDict()
{
}

bool notEqual (const Waypoint& wp1, const Waypoint& wp2)
{
  return
    wp1.name != wp2.name ||
    wp1.angle != wp2.angle ||
    wp1.comment != wp2.comment ||
    wp1.description != wp2.description ||
    wp1.distance != wp2.distance ||
    wp1.elevation != wp2.elevation ||
    wp1.fixTime != wp2.fixTime ||
    wp1.frequency != wp2.frequency ||
    wp1.icao != wp2.icao ||
    wp1.importance != wp2.importance ||
    wp1.isLandable != wp2.isLandable ||
    wp1.length != wp2.length ||
    wp1.origP != wp2.origP ||
    wp1.runway != wp2.runway ||
    wp1.type != wp2.type ||
    wp1.surface != wp2.surface;
}

/** insert a new item into the list and check if waypoint already exist */
bool WaypointDict::insertItem(Waypoint *e)
{
  Waypoint *tmp;
  bool ins = true;
  bool OK = false;
  QString newName;
  
  if (tmp = find(e->name))
  {
    if (notEqual (*tmp, *e))
    {
#ifdef OLDVERSION
      switch (KMessageBox::warningContinueCancel(0,
            "<qt>" + QObject::tr("A waypoint with the name<BR><BR><B>%1</B><BR><BR>is already in current catalog.<BR><BR>Do you want to overwrite the existing waypoint?").arg(e->name) + "</qt>",
            QObject::tr("Add waypoint"),
            QObject::tr("&Overwrite")
           )) {
#endif
#ifndef OLDVERSION
      switch (KMessageBox::warningYesNoCancel(0,
            "<qt>" + QObject::tr("A waypoint with the name<BR><BR><B>%1</B><BR><BR>is already in current catalog.<BR><BR>Do you want to rename the waypoint you are adding or overwrite the existing waypoint?").arg(e->name) + "</qt>",
            QObject::tr("Add waypoint"),
            QObject::tr("&Rename"),
            QObject::tr("&Overwrite")
           )) {

      case KMessageBox::Yes:   //rename
        newName=KInputDialog::getText(QObject::tr("Waypoint name"), QObject::tr("Please enter a new name for the waypoint"), e->name, &OK);
        if (OK) {
          e->name=newName;
          return insertItem(e); //recursive call!
          break;
        }
        //no break!
#endif
      case KMessageBox::Cancel:   //cancel
        delete e;
        ins = false;
        break;
      case KMessageBox::No:          //overwrite, new version
      case KMessageBox::Continue:    //overwrite, old version
        remove(e->name);
        insert(e->name, e);
        break;
      }
    }
  }
  else {
    insert(e->name, e);
  }

  return ins;
}

#ifdef OLDVERSION
  //make sure we are not messing up something else elsewhere
  #undef OLDVERSION
#endif
