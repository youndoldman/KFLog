/***********************************************************************
**
**   translationlist.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "translationlist.h"

TranslationList::TranslationList()
{
  setAutoDelete(true);
}

TranslationList::~TranslationList(){
}

int TranslationList::compareItems(QCollection::Item e1, QCollection::Item e2)
{
  return (((TranslationElement *)e1)->text.compare(((TranslationElement *)e2)->text));
}

/** append new element and idex by id */
void TranslationList::append(const TranslationElement *item)
{
  QGList::append((void *)item);
  // index by id
  idIndex.insert(item->id, item);
}
/** return index by ID */
int TranslationList::idxById(int id)
{
  // find item and return index
  return findRef(idIndex[id]);
}
/** return a transaltion element item by given id */
TranslationElement *TranslationList::itemById(int id)
{
  return idIndex[id];
}
