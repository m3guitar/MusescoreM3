//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: spacer.cpp 5658 2012-05-21 18:40:58Z wschweer $
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "spacer.h"
#include "score.h"
#include "mscore.h"

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

Spacer::Spacer(Score* score)
   : Element(score)
      {
      _spacerType = SPACER_UP;
      _gap = 0.0;
      }

Spacer::Spacer(const Spacer& s)
   : Element(s)
      {
      _gap    = s._gap;
      path    = s.path;
      _spacerType = s._spacerType;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Spacer::draw(QPainter* painter) const
      {
      if (score()->printing() || !score()->showUnprintable())
            return;
      QPen pen(selected() ? MScore::selectColor[0] : MScore::layoutBreakColor,
         spatium() * 0.4);
      painter->setPen(pen);
      painter->setBrush(Qt::NoBrush);
      painter->drawPath(path);
      }

//---------------------------------------------------------
//   layout0
//---------------------------------------------------------

void Spacer::layout0()
      {
      qreal _spatium = spatium();

      path    = QPainterPath();
      qreal w = _spatium;
      qreal b = w * .5;
      qreal h = _gap;

      if (spacerType() == SPACER_DOWN) {
            path.lineTo(w, 0.0);
            path.moveTo(b, 0.0);
            path.lineTo(b, h);
            path.lineTo(0.0, h-b);
            path.moveTo(b, h);
            path.lineTo(w, h-b);
            }
      else if (spacerType() == SPACER_UP) {
            path.moveTo(b, 0.0);
            path.lineTo(0.0, b);
            path.moveTo(b, 0.0);
            path.lineTo(w, b);
            path.moveTo(b, 0.0);
            path.lineTo(b, h);
            path.moveTo(0.0, h);
            path.lineTo(w, h);
            }
      qreal lw = _spatium * 0.4;
      QRectF bb(0, 0, w, h);
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);
      }

//---------------------------------------------------------
//   setGap
//---------------------------------------------------------

void Spacer::setGap(qreal sp)
      {
      _gap = sp;
      layout0();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Spacer::spatiumChanged(qreal ov, qreal nv)
      {
      _gap = (_gap / ov) * nv;
      layout0();
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Spacer::editDrag(const EditData& ed)
      {
      qreal s = ed.delta.y();
      if (spacerType() == SPACER_DOWN)
            _gap += s;
      else if (spacerType() == SPACER_UP)
            _gap -= s;
      if (_gap < spatium() * 2.0)
            _gap = spatium() * 2;
      layout0();
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Spacer::updateGrips(int* grips, QRectF* grip) const
      {
      *grips         = 1;
      qreal _spatium = spatium();
      QPointF p;
      if (spacerType() == SPACER_DOWN)
            p = QPointF(_spatium * .5, _gap);
      else if (spacerType() == SPACER_UP)
            p = QPointF(_spatium * .5, 0.0);
      grip[0].translate(pagePos() + p);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Spacer::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("subtype", _spacerType);
      Element::writeProperties(xml);
      xml.tag("space", _gap / spatium());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Spacer::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "subtype")
                  _spacerType = SpacerType(e.readInt());
            else if (tag == "space")
                  _gap = e.readDouble() * spatium();
            else if (!Element::readProperties(e))
                  e.unknown();
            }
      layout0();
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Spacer::getProperty(P_ID propertyId) const
      {
      switch(propertyId) {
            case P_SPACE: return gap();
            default:
                  return Element::getProperty(propertyId);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Spacer::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch(propertyId) {
            case P_SPACE:
                  setGap(v.toDouble());
                  break;
            default:
                  if (!Element::setProperty(propertyId, v))
                        return false;
                  break;
            }
      layout0();
      score()->setLayoutAll(true);
      setGenerated(false);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Spacer::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_SPACE: return QVariant(0.0);
            default:
                  return Element::propertyDefault(id);
            }
      }


