/***********************************************************************
**
**   mapmatrix.cpp
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

#include <cmath>

#include "mapmatrix.h"

#define VAR1   ( cos(v1) * cos(v1) )
#define VAR2   ( sin(v1) + sin(v2) )

// Projektions-Ma�stab
// 10 Meter H�he pro Pixel ist die st�rkste Vergr��erung.
// Bei dieser Vergr��erung erfolgt die eigentliche Projektion
#define MAX_SCALE 10.0
#define MIN_SCALE 5000.0

#define MAX(a,b)   ( ( a > b ) ? a : b )
#define MIN(a,b)   ( ( a < b ) ? a : b )

// Mit welchem Radius m�ssen wir rechnen ???
#define RADIUS 6370289.509
#define PI 3.141592654
#define NUM_TO_RAD(num) ( ( PI * (double)(num) ) / 108000000.0 )
#define RAD_TO_NUM(rad) ( (int)( (rad) * 108000000.0 / PI ) )

MapMatrix::MapMatrix()
  : cScale(0), rotationArc(0)
{
  viewBorder.setTop(32000000);
  viewBorder.setBottom(25000000);
  viewBorder.setLeft(2000000);
  viewBorder.setRight(7000000);
}

MapMatrix::~MapMatrix()
{

}

QPoint MapMatrix::wgsToMap(QPoint origPoint) const
{
   return wgsToMap(origPoint.x(), origPoint.y());
}

QPoint MapMatrix::wgsToMap(int lat, int lon) const
{
  return QPoint(__calc_X_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon)) *
                    RADIUS / MAX_SCALE,
                __calc_Y_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon)) *
                    RADIUS / MAX_SCALE);
}

QRect MapMatrix::wgsToMap(QRect rect)
{
  return QRect(wgsToMap(rect.topLeft()), wgsToMap(rect.bottomRight()));
}

QPoint MapMatrix::__mapToWgs(QPoint origPoint) const
{
  return __mapToWgs(origPoint.x(), origPoint.y());
}

QPoint MapMatrix::__mapToWgs(int x, int y) const
{
  double lat = __invert_Lambert_Lat(x * MAX_SCALE / RADIUS,
                                    y * MAX_SCALE / RADIUS);
  double lon = __invert_Lambert_Lon(x * MAX_SCALE / RADIUS,
                                    y * MAX_SCALE / RADIUS);

  return QPoint((int)lon, (int)lat);
}

bool MapMatrix::isVisible(QPoint pos) const
{
  return (mapBorder.contains(pos));
}

bool MapMatrix::isVisible(QRect itemBorder) const
{
  // Grenze: Nahe 15Bit
  // Vereinfachung kann zu Fehlern f�hren ...
  return ( ( mapBorder.intersects(itemBorder) ) &&
           ( itemBorder.width() * ( MAX_SCALE / cScale ) < 30000 ) &&
           ( itemBorder.height() * ( MAX_SCALE / cScale ) < 30000 ) );

//  return ( ( mapBorder.intersects(itemBorder) ) &&
//           ( itemBorder.width() * ( MAX_SCALE / cScale ) > 2 ) &&
//           ( itemBorder.height() * ( MAX_SCALE / cScale ) > 2 ) );
}

QPointArray MapMatrix::map(QPointArray origArray) const
{
  return worldMatrix.map(origArray);
}

QPoint MapMatrix::map(QPoint origPoint) const
{
  return worldMatrix.map(origPoint);
}

double MapMatrix::map(double arc) const
{
  return (arc + rotationArc);
}

QPointArray MapMatrix::print(QPointArray pArray) const
{
  return printMatrix.map(pArray);
}

QPoint MapMatrix::print(int lat, int lon, double dX, double dY) const
{
  QPoint temp;

  if(dX == 0 &&  dY == 0)
    {
      temp = QPoint(
      ( __calc_X_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon - mapCenterLon) )
          * RADIUS / pScale ) + dX,
      ( __calc_Y_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon - mapCenterLon) )
           * RADIUS / pScale ) + dY );
    }
  else
    {
      temp = QPoint(
      ( __calc_X_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon - mapCenterLon) )
          * RADIUS / ( pScale * 0.5 ) ) + dX,
      ( __calc_Y_Lambert( NUM_TO_RAD(lat), NUM_TO_RAD(lon - mapCenterLon) )
           * RADIUS / ( pScale * 0.5 ) ) + dY );
    }
  return temp;
}

QRect MapMatrix::getViewBorder() const  { return viewBorder; }

QRect MapMatrix::getPrintBorder(double a1, double a2, double b1, double b2,
        double c1, double c2, double d1, double d2) const
{
  QRect temp;

  temp.setTop( __invert_Lambert_Lat(a2 * pScale / RADIUS,
      a1 * pScale / RADIUS) );
  temp.setBottom( __invert_Lambert_Lat(b2 * pScale / RADIUS,
      b1 * pScale / RADIUS) );
  temp.setRight( __invert_Lambert_Lon(c2 * pScale / RADIUS,
      c1 * pScale / RADIUS) + mapCenterLon );
  temp.setLeft( __invert_Lambert_Lon(d2 * pScale / RADIUS,
      d1 * pScale / RADIUS) + mapCenterLon );

  return temp;
}

QPoint MapMatrix::getMapCenter() const
{
  return QPoint(mapCenterLat, mapCenterLon);
}

double MapMatrix::scale() const  { return cScale; }

void MapMatrix::centerToMouse(QPoint center)
{
  bool result = true;
  QWMatrix invertMatrix = worldMatrix.invert(&result);
  if(!result)
      // Houston, wir haben ein Problem !!!
      qFatal("KFLog: Cannot invert worldmatrix!");

  QPoint projCenter = __mapToWgs(invertMatrix.map(center));
  mapCenterLat = projCenter.y();
  mapCenterLon = projCenter.x();
}

void MapMatrix::centerToRect(QRect center)
{
  int centerX = (center.left() + center.right()) / 2;
  int centerY = (center.top() + center.bottom()) / 2;

  // Was passiert mit dem Ma�stab ???
  double xScaleDelta = (double)(sqrt(center.width() * center.width())) /
      (double)mapViewSize.width();
  double yScaleDelta = (double)(sqrt(center.height() * center.height())) /
      (double)mapViewSize.height();

  double tempScale = cScale * MAX(xScaleDelta, yScaleDelta) * 1.05;
  // Maximale Vergr��erung
  if(tempScale < MAX_SCALE)
      tempScale = MAX_SCALE;

  // �nderung nur, wenn Unterschied zu gross:
  if((tempScale / cScale) > 1.1 || (tempScale / cScale) < 0.8)
      cScale = tempScale;

  centerToMouse(QPoint(centerX, centerY));
}

QPoint MapMatrix::mapToWgs(QPoint pos) const
{
  bool result = true;
  QWMatrix invertMatrix = worldMatrix.invert(&result);
  if(!result)
      // Houston, wir haben ein Problem !!!
      qFatal("KFLog: Cannot invert worldmatrix!");

  return __mapToWgs(invertMatrix.map(pos));
}

void MapMatrix::moveMap(int dir)
{
  switch(dir)
    {
      case North:
        mapCenterLat = viewBorder.top();
        break;
      case North | West:
        mapCenterLat = viewBorder.top();
        mapCenterLon = viewBorder.left();
        break;
      case North | East:
        mapCenterLat = viewBorder.top();
        mapCenterLon = viewBorder.right();
        break;
      case West:
        mapCenterLon = viewBorder.left();
        break;
      case East:
        mapCenterLon = viewBorder.right();
        break;
      case South:
        mapCenterLat = viewBorder.bottom();
        break;
      case South | West:
        mapCenterLat = viewBorder.bottom();
        mapCenterLon = viewBorder.left();
        break;
      case South | East:
        mapCenterLat = viewBorder.bottom();
        mapCenterLon = viewBorder.right();
        break;
      case Home:
        mapCenterLat = homeLat;
        mapCenterLon = homeLon;
    }
}

void MapMatrix::createMatrix(QSize newSize)
{
  // Nicht gerde ideal, vielleicht kann _currentScale mal rausfliegen ...
  extern double _currentScale;
  _currentScale = cScale;

  const QPoint tempPoint(wgsToMap(mapCenterLat, mapCenterLon));

  worldMatrix.reset();

  /* Set rotating and scaling */
  double scale = MAX_SCALE / cScale;
  rotationArc = atan(tempPoint.x() * 1.0 / tempPoint.y() * 1.0);

  worldMatrix.setMatrix(cos(rotationArc) * scale, sin(rotationArc) * scale,
      -sin(rotationArc) * scale, cos(rotationArc) * scale, 0,0);

  /* Set the tranlation */
  QWMatrix translateMatrix(1, 0, 0, 1, newSize.width() / 2,
    ( newSize.height() / 2 ) - worldMatrix.map(tempPoint).y() );

  worldMatrix = worldMatrix * translateMatrix;

  /* Set the viewBorder */
  bool result = true;
  QWMatrix invertMatrix = worldMatrix.invert(&result);
  if(!result)
      // Houston, wir haben ein Problem !!!
      qFatal("KFLog: Cannot invert worldmatrix!");

  /*
   * Die Berechnung der Kartengrenze funktioniert so nur auf der
   * Nordhalbkugel. Auf der S�dhalbkugel stimmen die Werte nur
   * n�herungsweise.
   */
  QPoint tCenter = __mapToWgs(invertMatrix.map(QPoint(newSize.width() / 2, 0)));
  QPoint tlCorner = __mapToWgs(invertMatrix.map(QPoint(0, 0)));
  QPoint trCorner = __mapToWgs(invertMatrix.map(QPoint(newSize.width(), 0)));
  QPoint blCorner = __mapToWgs(invertMatrix.map(QPoint(0, newSize.height())));
  QPoint brCorner = __mapToWgs(invertMatrix.map(QPoint(newSize.width(),
      newSize.height())));

  viewBorder.setTop(tCenter.y());
  viewBorder.setLeft(tlCorner.x());
  viewBorder.setRight(trCorner.x());
  viewBorder.setBottom(MIN(blCorner.y(), brCorner.y()));

  mapBorder = invertMatrix.map(QRect(0,0, newSize.width(), newSize.height()));
  mapViewSize = newSize;
}

QWMatrix* MapMatrix::createPrintMatrix(double printScale, QSize pSize, int dX,
    int dY, bool rotate)
{
  pScale = printScale;

  const QPoint tempPoint(wgsToMap(mapCenterLat, mapCenterLon));
  printMatrix.reset();

  double scale = MAX_SCALE / pScale;
  double printArc = atan(tempPoint.x() * 1.0 / tempPoint.y() * 1.0);

  printMatrix.setMatrix(cos(printArc) * scale, sin(printArc) * scale,
      -sin(printArc) * scale, cos(printArc) * scale, 0, 0);

  if(rotate)
      printMatrix.rotate(90);

  /* Set the tranlation */
  if(dX == 0 && dY == 0)
    {
      QWMatrix translateMatrix(1, 0, 0, 1, pSize.width() / 2 + dX,
        ( pSize.height() / 2 ) - printMatrix.map(tempPoint).y() + dY );

      printMatrix = printMatrix * translateMatrix;
    }
  else
    {
      QWMatrix translateMatrix(1, 0, 0, 1, dX, dY );

      printMatrix = printMatrix * translateMatrix;
    }

  return &printMatrix;
}

void MapMatrix::scaleAdd(QSize mapSize)
{
  cScale = MAX( ( cScale / 1.25 ), MAX_SCALE);
  createMatrix(mapSize);
}

void MapMatrix::scaleSub(QSize mapSize)
{
  cScale = MIN( ( cScale * 1.25 ), MIN_SCALE);
  createMatrix(mapSize);
}

void MapMatrix::setScale(double nScale)  {  cScale = nScale;  }

void MapMatrix::initMatrix(int centerLat, int centerLon, double scale,
        double nv1, double nv2, int hLat, int hLon)
{
  v1 = nv1;
  v2 = nv2;

  var1 = cos(v1)*cos(v1);
  var2 = sin(v1)+sin(v2);

  mapCenterLat = centerLat;
  mapCenterLon = centerLon;

  homeLat = hLat;
  homeLon = hLon;

  cScale = scale;
}

double MapMatrix::__calc_Y_Lambert(double latitude, double longitude) const
{
  return ( 2 * ( sqrt( var1 + ( sin(v1) - sin(latitude) ) * var2 ) / var2 )
             * cos( var2 * longitude / 2 ) );
}

double MapMatrix::__calc_X_Lambert(double latitude, double longitude) const
{
  return ( 2 * ( sqrt( var1 + ( sin(v1) - sin(latitude) ) * var2 ) / var2 )
             * sin( var2 * longitude / 2 ) );
}

int MapMatrix::__invert_Lambert_Lat(double x, double y) const
{
  double lat = -asin(
              ( -4.0 * pow(cos(v1), 2.0) - 4.0 * pow(sin(v1), 2.0)
                -4.0 * sin(v1) * sin(v2)
                + x * x * pow(sin(v1), 2.0) + pow(sin(v1), 2.0)* y * y
                + 2.0 * x * x * sin(v1) * sin(v2) + 2.0 * sin(v1)
                * sin(v2) * y * y + x * x * pow(sin(v2), 2.0)
                + pow(sin(v2), 2.0) * y * y
                ) /
              ( sin(v1) + sin(v2) ) / 4 );
  return RAD_TO_NUM(lat);
}

int MapMatrix::__invert_Lambert_Lon(double x, double y) const
{
  double lon = 2.0 * atan( x / y ) / ( sin(v1) + sin(v2) );
  return RAD_TO_NUM(lon);
}
