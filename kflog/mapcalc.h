/***********************************************************************
**
**   mapcalc.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef MAPCALC_H
#define MAPCALC_H

#include "wp.h"
#include "waypoint.h"
#include <time.h>

#define PI  3.141592654
#define PI2 6.28318530718

/**
 * The earth's radius used for calculation, given in Meter.
 * NOTE: We use the earth as a sphere, not as a spheroid!
 */
#define RADIUS 6370289.509

/**
 * Calculates the distance between two given points (in km).
 */
double dist(double lat1, double lon1, double lat2, double lon2);

/**
 * Calculates the distance between two given points (in km).
 */
double dist(Waypoint* wp1, Waypoint* wp2);

/**
 * Calculates the distance between two given points (in km).
 */
double dist(Waypoint* wp, flightPoint* fp);

/**
 * Calculates the distance between two given points (in km).
 */
double dist( flightPoint* fp1, flightPoint* fp2);

/**
 * Converts the given coordinate into a readable string.
 * ( xxx,xxxx�[N,S,E,W] )
 */
QString printPos(int coord, bool isLat = true);

/**
 * Converts the given time (in sec. from 1.1.1970 00:00:00) into a readable string.
 * ( hh:mm:ss )
 */
QString printTime(time_t time, bool isZero = false, bool isSecond = true);

// Conversion for time delays
QString printTime(int time, bool isZero = false, bool isSecond = true);

// Compute the time_t equivalent from  year [1970 - ...], month  [ 1-12], day [1-31]
time_t timeToDay(const int year, const int month, const int day, const char *monabb = NULL);

/**
 * Calculates the vario of a given point
 */
float getVario(flightPoint p);

/**
 * Calculates the speed of a given point
 */
float getSpeed(flightPoint p);

/**
 * Calculates the bearing to the previous point
 */
float getBearing(flightPoint p1, flightPoint p2);

/**
 * Converts a x/y position into a polar-coordinate.
 */
double polar(double x, double y);

/* convert internal point into radians */
double int2rad(int deg);

/* convert radians into internal point */
double rad2int(double rad);

/* calculate true course in radians to point 2 */
double tc(double lat1, double lon1, double lat2, double lon2);

/* calculate angle between leg a and b in radians */
double angle(double a, double b, double c);

/* calculate pos of point from dist and bearing */ 
WGSPoint posOfDistAndBearing(double lat1, double lon1, double bearing, double dist);

/* calculate true course from prev point (p2) */
double getTrueCourse(WGSPoint p1, WGSPoint p2);

/**
 * Calculates the direction of the vector pointing to the outside
 * of the area spanned by the two vectors.
 */
double outsideVector(QPoint center, QPoint p1, QPoint p2);
double outsideVector(double angle1, double angle2);

double normalize(double angle);

int normalize(int angle);


/**
  * Calculates the difference between two angles, returning the smallest
  * angle. It returns an angle between -180 and 180 in degrees. Positive
  * in clockwise direction.
  */
int angleDiff(int ang1, int ang2);

/**
  * Calculates the difference between two angles, returning the smallest
  * angle. It returns an angle between -Pi and Pi in rad.
  */
double angleDiff(double ang1, double ang2);

#endif
