/***********************************************************************
**
**   dataview.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "dataview.h"

#include <kglobal.h>
#include <klocale.h>

#include <qlayout.h>
#include <qstrlist.h>
#include <qfileinfo.h>

#include <flight.h>
#include <mapcalc.h>
#include <flighttask.h>
#include <flightgroup.h>
#include <mapcontents.h>

#include <kmessagebox.h>
#include <iostream>

DataView::DataView(QWidget* parent)
: QFrame(parent, "FlightData")
{
  flightDataText = new KTextBrowser(this, "flightDataBrowser", true);

  QHBoxLayout* flightLayout = new QHBoxLayout(this, 5);
  flightLayout->addWidget(flightDataText);

  connect(flightDataText, SIGNAL(urlClick(const QString &)), this,
      SLOT(slotWPSelected(const QString &)));
}

DataView::~DataView()
{

}

QString DataView::__writeTaskInfo(FlightTask* task)
{
  QString htmlText;
  QString txt, tmp,speed;
  QString idString, timeString;
  struct wayPoint *wp;

  htmlText = "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
      <TR><TD COLSPAN=3 BGCOLOR=#BBBBBB><B>" +
      i18n("Task") + ":</B></TD></TR>";

  QList<wayPoint> wpList = task->getWPList();

  for(unsigned int loop = 0; loop < wpList.count(); loop++)
    {
      wp = wpList.at(loop);
      if(loop > 0)
        {
          tmp.sprintf("%.2f km / %03.0f� / %.1f km/h",
            wp->distance,
            getTrueCourse(wp->origP, wpList.at(loop - 1)->origP),
            wp->distance/(wp->sectorFAI-wpList.at(loop-1)->sectorFAI)*3600.0);
          htmlText += "<TR><TD ALIGN=center COLSPAN=3 BGCOLOR=#EEEEEE>" +
              tmp + "</TD></TR>";
        }

      idString.sprintf("%d", loop);

      if(wp->sector1 != 0)
          timeString = printTime(wp->sector1);
      else if(wp->sector2 != 0)
          timeString = printTime(wp->sector2);
      else if(wp->sectorFAI != 0)
          timeString = printTime(wp->sectorFAI);
      else
          timeString = "--";

      htmlText += "<TR><TD COLSPAN=2><A HREF=" + idString + ">" +
          wp->name + "</A></TD>\
          <TD ALIGN=right>" + timeString + "</TD></TR>\
          <TR><TD WIDTH=15></TD>\
          <TD>" + printPos(wp->origP.lat()) + "</TD>\
          <TD ALIGN=right>" + printPos(wp->origP.lon(), false) +
          "</TD></TR>";
    }
  
  if (task->getTaskType()==FlightTask::OLC){
    txt.sprintf("%.2f", task->getOlcPoints());
    speed.sprintf("%.2f",task->getAverageSpeed());
    htmlText += "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("Total Distance") +
      ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
      task->getTotalDistanceString() + "</TD></TR>\
      <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("Task Distance") +
      ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
      task->getTaskDistanceString() + "</TD></TR>\
      <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("Average Speed") +
      ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
      speed + "</TD>" +
      "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("Points") +
      ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
      txt + "</TD>" +
      "</TR></TABLE>";
  }
  else
  {
    if  (task->getPlanningType()==FlightTask::Route){
      txt.sprintf("%d", task->getPlannedPoints());
      htmlText += "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("total Distance") +
        ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
        task->getTotalDistanceString() + "</TD></TR>\
        <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("Task Distance") +
        ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
        task->getTaskDistanceString() + "</TD></TR>\
        <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("Points") +
        ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
        txt + "</TD></TR></TABLE>";
    }
    else{  // area based
      if (wpList.count() < 3)
        tmp = "--";
      else
        tmp.sprintf("%.2f km", wpList.at(2)->distance);
      htmlText += "<TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("Leg Distance") +
        ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
        tmp + "</TD></TR>\
        <TR><TD COLSPAN=2 BGCOLOR=#BBBBBB><B>" + i18n("FAI Distance") +
        ":</B></TD><TD ALIGN=right BGCOLOR=#BBBBBB>" +
        task->getFAIDistanceString() + "</TD></TR></TABLE>";
    }
  }
  return htmlText;
}

void DataView::slotShowTaskText(FlightTask* task)
{
  QList<wayPoint> taskPointList = task->getWPList();
  QString htmlText = "";
  QString tmp;

  htmlText = "<B>" + i18n("Task-Type: ") + "</B>" +
      task->getTaskTypeString() + "<BR>";

  if(task->getWPList().count() > 0)  htmlText += __writeTaskInfo(task);

  // Frage
  if(taskPointList.count() == 0)
    {
      htmlText += "Bitte w�hlen Sie den <b>Startort</b> der Aufgabe in der Karte<br>";
    }
  else if(taskPointList.count() == 1)
    {
      htmlText += "<b>" + i18n("Begin of task") + "?</b>";
    }
  else
    {
      htmlText += "<b>" + i18n("Next waypoint") + "?</b>";
    }

  flightDataText->setText(htmlText);
}

void DataView::setFlightData()
{
  extern MapContents _globalMapContents;
  BaseFlightElement* e = _globalMapContents.getFlight();
  QString htmlText;
  QString idString;
  QList<Flight> fl;
  QStrList h;
  FlightTask fTask("");
  QFileInfo fi;

  slotClearView();

  if(e)
    {
      switch (e->getTypeID())
        {
          case BaseMapElement::Flight:
            h = ((Flight*)e)->getHeader();
            fTask = ((Flight*)e)->getTask();

            //
            // For some strange reason, the widget adds a large vertical space
            // between the the first table and the following rule. Therfore I
            // have removed the rule
            //                                                     Heiner, 2003-01-02
            //
            htmlText = (QString)"<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>" +
                "<TR><TD>" + i18n("Date") + ":</TD><TD>" + h.at(3) + "</TD></TR>" +
                "<TR><TD>" + i18n("Pilot") + ":</TD><TD> " + h.at(0) + "</TD></TR>" +
                "<TR><TD>" + i18n("Glider") + ":</TD><TD>" + h.at(2) +
                " / " + h.at(1) + "</TD></TR>" +
                "</TABLE><BR>";

            if(fTask.getWPList().count())
                htmlText += __writeTaskInfo(&fTask);
            else
                htmlText += "<EM>" + i18n("Flight contains no waypoints") + "</EM>";
            break;
          case BaseMapElement::Task:
            if(e->getWPList().count())
                htmlText = __writeTaskInfo((FlightTask*)e);
            else
              {
                htmlText = i18n(
                  "You can select waypoints with the left mouse button."
                  "You can also select free waypoints by clicking anywhere in the map."
                  "<br><br>"
                  "When you press &lt;CTRL&gt; and click with the left mouse button on a taskpoint, "
                  "it will be deleted.<br>"
                  "You can compute the task up to your current mouse position by pressing &lt;SHIFT&gt;."
                  "<br>"
                  "To finish the task, press &lt;CTRL&gt; and click the right mouse button.<br>"
                  "It's possible to move and delete taskpoints from the finished task."
                  );
              }
            break;
          case BaseMapElement::FlightGroup:
            htmlText = "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
                <TR><TD>" + i18n("Flight group") + ":</TD><TD><A HREF=EDITGROUP>" +
                e->getFileName() +  + "</A></TD></TR>\
                </TABLE><HR NOSHADE>";
            fl = ((FlightGroup*)e)->getFlightList();
            if(fl.count()) {
              htmlText += "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0>\
                <TR><TD COLSPAN=3 BGCOLOR=#BBBBBB><B>" +
                 i18n("Flights") + ":</B></TD></TR>";

              for (unsigned int loop = 0; loop < fl.count(); loop++) {
                Flight *flight = fl.at(loop);
                // store pointer of flight instead of index
                // flight list of mapcontents will change
                idString.sprintf("%d", (int)flight);
                fi.setFile(flight->getFileName());

                htmlText += "<TR><TD><A HREF=" + idString + ">" +
                  fi.fileName() + "</A></TD><TD ALIGN=right>" +
                  KGlobal::locale()->formatDate(flight->getDate(), true) + "</TD></TR>\
                  <TR><TD>" + flight->getDistance() + "</TD><TD ALIGN=right>" +
                  printTime(flight->getLandTime() - flight->getStartTime()) + "</TD></TR>";
              }
            }
            else {
              htmlText += i18n("Click on the group name to start editing");
            }
            break;
        }
      flightDataText->setText(htmlText);
    }
}

void DataView::slotWPSelected(const QString &url)
{
  extern MapContents _globalMapContents;
  BaseFlightElement* e = _globalMapContents.getFlight();

  switch(e->getTypeID()) {
    case BaseMapElement::Flight:
      emit wpSelected(url.toUInt());
      break;
    case BaseMapElement::Task:
      if (url == "EDITTASK") {
        KMessageBox::information(0, "This will bring up the task editing dialog");
      }
      else {
        emit wpSelected(url.toUInt());
      }
      break;
    case BaseMapElement::FlightGroup:
      if (url == "EDITGROUP") {
        emit editFlightGroup();
      }
      else {
        emit flightSelected((BaseFlightElement *)url.toUInt());
      }
      break;
  }
}

void DataView::slotClearView()
{
  QString htmlText = "";
  flightDataText->setText(htmlText);
}

