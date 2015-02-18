//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/* TO DO:
- XML export

NICE-TO-HAVE TODO:
- draggable handles of glissando segments
- re-attachable glissando extrema (with [Shift]+arrows, use SlurSegment::edit()
      and SlurSegment::changeAnchor() in slur.cpp as models)
*/

#include "arpeggio.h"
#include "glissando.h"
#include "chord.h"
#include "ledgerline.h"
#include "note.h"
#include "notedot.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"
#include "style.h"
#include "sym.h"
#include "xml.h"

namespace Ms {

static const qreal      GLISS_DEFAULT_LINE_TICKNESS   = 0.15;
static const qreal      GLISS_PALETTE_WIDTH           = 4.0;
static const qreal      GLISS_PALETTE_HEIGHT          = 4.0;

//---------------------------------------------------------
//   GlisandoSegment
//---------------------------------------------------------

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void GlissandoSegment::layout()
      {
      QRectF r = QRectF(0.0, 0.0, pos2().x(), pos2().y()).normalized();
      qreal lw = spatium() * glissando()->lineWidth().val() * .5;
      setbbox(r.adjusted(-lw, -lw, lw, lw));
      adjustReadPos();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void GlissandoSegment::draw(QPainter* painter) const
      {
      painter->save();
      qreal _spatium = spatium();

      QPen pen(glissando()->curColor());
      pen.setWidthF(glissando()->lineWidth().val() * spatium());
      pen.setCapStyle(Qt::RoundCap);
      painter->setPen(pen);
//      painter->drawLine(QPointF(), pos2());               // DEBUG

      // rotate painter so that the line become horizontal
      qreal w     = pos2().x();
      qreal h     = pos2().y();
      qreal l     = sqrt(w * w + h * h);
      qreal wi = asin(-h / l) * 180.0 / M_PI;
      painter->rotate(-wi);

      if (glissando()->glissandoType() == Glissando::Type::STRAIGHT) {
            painter->drawLine(QLineF(0.0, 0.0, l, 0.0));
            }
      else if (glissando()->glissandoType() == Glissando::Type::WAVY) {
            QRectF b = symBbox(SymId::wiggleTrill);
//            qreal h  = symHeight(SymId::wiggleTrill);     // DEBUG
            qreal w  = symWidth(SymId::wiggleTrill);
            int n    = (int)(l / w);      // always round down (truncate) to avoid overlap
            qreal x  = (l - n*w) * 0.5;   // centre line in available space
            drawSymbol(SymId::wiggleTrill, painter, QPointF(x, b.height()*.70), n);
            }
      if (glissando()->showText()) {
            const TextStyle& st = score()->textStyle(TextStyleType::GLISSANDO);
            QFont f = st.fontPx(_spatium);
            QRectF r = QFontMetricsF(f).boundingRect(glissando()->text());
            // if text longer than available space, skip it
            if (r.width() < l) {
                  qreal yOffset = r.height() + r.y();       // find text descender height
                  // raise text slightly above line and slightly more with WAVY than with STRAIGHT
                  yOffset += _spatium * (glissando()->glissandoType() == Glissando::Type::WAVY ? 0.5 : 0.1);
                  painter->setFont(f);
                  qreal x = (l - r.width()) * 0.5;
                  painter->drawText(QPointF(x, -yOffset), glissando()->text());
                  }
            }
      painter->restore();
      }

//---------------------------------------------------------
//   Glissando
//---------------------------------------------------------

Glissando::Glissando(Score* s)
  : SLine(s)
      {
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE);

      _glissandoType = Type::STRAIGHT;
      _text          = "gliss.";
      _showText      = true;
      setDiagonal(true);
      setLineWidth(Spatium(GLISS_DEFAULT_LINE_TICKNESS));
      setAnchor(Spanner::Anchor::NOTE);
      }

Glissando::Glissando(const Glissando& g)
   : SLine(g)
      {
      _glissandoType = g._glissandoType;
      _text          = g._text;
      _showText      = g._showText;
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Glissando::createLineSegment()
      {
      GlissandoSegment* seg = new GlissandoSegment(score());
      seg->setFlag(ElementFlag::ON_STAFF, false);
      seg->setTrack(track());
      seg->setColor(color());
      return seg;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Glissando::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      func(data, this);
      SLine::scanElements(data, func, all);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Glissando::layout()
      {
      qreal       _spatium    = spatium();

      // for use in palettes
      if (score() == gscore) {
            if (spannerSegments().isEmpty())
                  add(createLineSegment());
            LineSegment* s = frontSegment();
            s->setPos(QPointF());
            s->setPos2(QPointF(_spatium * GLISS_PALETTE_WIDTH, -_spatium * GLISS_PALETTE_HEIGHT));
            s->layout();
            return;
            }

      SLine::layout();
      setPos(0.0, 0.0);
      adjustReadPos();

      Note*       anchor1     = static_cast<Note*>(startElement());
      Note*       anchor2     = static_cast<Note*>(endElement());
      Chord*      cr1         = anchor1->chord();
      Chord*      cr2         = anchor2->chord();
      GlissandoSegment*       segm1 = static_cast<GlissandoSegment*>(frontSegment());
      GlissandoSegment*       segm2 = static_cast<GlissandoSegment*>(backSegment());

      // Note: line segments are defined by
      // initial point: ipos() (relative to system origin)
      // ending point:  pos2() (relative to initial point)

      // LINE ENDING POINTS TO NOTE HEAD CENTRES

      // assume gliss. line goes from centre of initial note centre to centre of ending note:
      // move first segment origin and last segment ending point from note head origin to note head centre
      QPointF     offs1       = QPointF(anchor1->headWidth() * 0.5, 0.0);
      QPointF     offs2       = QPointF(anchor2->headWidth() * 0.5, 0.0);

      // AVOID HORIZONTAL LINES

      int         upDown      = (0 < (anchor2->pitch() - anchor1->pitch())) - ((anchor2->pitch() - anchor1->pitch()) < 0);
      // on TAB's, glissando are by necessity on the same string, this gives an horizontal glissando line;
      // make bottom end point lower and top ending point higher
      if (cr1->staff()->isTabStaff()) {
                  qreal yOff = cr1->staff()->lineDistance() * 0.3 * _spatium;
                  offs1.ry() += yOff * upDown;
                  offs2.ry() -= yOff * upDown;
            }
      // if not TAB, angle glissando between notes on the same line
      else {
            if (anchor1->line() == anchor2->line()) {
                  offs1.ry() += _spatium * 0.25 * upDown;
                  offs2.ry() -= _spatium * 0.25 * upDown;
                  }
            }

      // move initial point of first segment and adjust its length accordingly
      segm1->setPos (segm1->ipos()  + offs1);
      segm1->setPos2(segm1->ipos2() - offs1);
      // adjust ending point of last segment
      segm2->setPos2(segm2->ipos2() + offs2);

      // FINAL SYSTEM-INITIAL NOTE
      // if the last gliss. segment attaches to a system-initial note, some extra width has to be added
      if (cr2->segment()->measure() == cr2->segment()->system()->firstMeasure() && cr2->rtick() == 0)
      {
            segm2->rxpos() -= GLISS_STARTOFSYSTEM_WIDTH * _spatium;
            segm2->rxpos2()+= GLISS_STARTOFSYSTEM_WIDTH * _spatium;
      }

      // INTERPOLATION OF INTERMEDIATE POINTS
      // This probably belongs to SLine class itself; currently it does not seem
      // to be needed for anything else than Glissando, though

      // get total x-width and total y-height of all segments
      qreal xTot = 0.0;
      for (SpannerSegment* segm : spannerSegments())
            xTot += segm->ipos2().x();
      qreal y0   = segm1->ipos().y();
      qreal yTot = segm2->ipos().y() + segm2->ipos2().y() - y0;
      qreal ratio = yTot / xTot;
      // interpolate y-coord of intermediate points across total width and height
      qreal xCurr = 0.0;
      qreal yCurr;
      for (int i = 0; i < spannerSegments().count()-1; i++)
      {
           SpannerSegment* segm = segmentAt(i);
           xCurr += segm->ipos2().x();
           yCurr = y0 + ratio * xCurr;
           segm->rypos2() = yCurr - segm->ipos().y();       // position segm. end point at yCurr
           // next segment shall start where this segment stopped
           segm = segmentAt(i+1);
           segm->rypos2() += segm->ipos().y() - yCurr;      // adjust next segm. vertical length
           segm->rypos() = yCurr;                           // position next segm. start point at yCurr
      }

      // STAY CLEAR OF NOTE APPENDAGES

      // initial note dots / ledger line / note head
      offs1 *= -1.0;          // discount changes already applied
      int dots = cr1->dots();
      LedgerLine * ledLin = cr1->ledgerLines();
      // if dots, start at right of last dot
      // if no dots, from right of ledger line, if any; from right of note head, if no ledger line
      offs1.rx() += (dots && anchor1->dot(dots-1) ? anchor1->dot(dots-1)->pos().x() + anchor1->dot(dots-1)->width()
                  : (ledLin ? ledLin->pos().x() + ledLin->width() : anchor1->headWidth()) );

      // final note arpeggio / accidental / ledger line / accidental / arpeggio (i.e. from outermost to innermost)
      offs2 *= -1.0;          // discount changes already applied
      if (Arpeggio* a = cr2->arpeggio())
            offs2.rx() += a->pos().x() + a->userOff().x();
      else if (Accidental* a = anchor2->accidental())
            offs2.rx() += a->pos().x() + a->userOff().x();
      else if ( (ledLin = cr2->ledgerLines()) != nullptr)
            offs2.rx() += ledLin->pos().x();

      // add another a quarter spatium of 'air'
      offs1.rx() += _spatium * 0.25;
      offs2.rx() -= _spatium * 0.25;

      // apply offsets: shorten first segment by x1 (and proportionally y) and adjust its length accordingly
      offs1.ry() = segm1->ipos2().y() * offs1.x() / segm1->ipos2().x();
      segm1->setPos(segm1->ipos() + offs1);
      segm1->setPos2(segm1->ipos2() - offs1);
      // adjust last segment length by x2 (and proportionally y)
      offs2.ry() = segm2->ipos2().y() * offs2.x() / segm2->ipos2().x();
      segm2->setPos2(segm2->ipos2() + offs2);

      for (SpannerSegment* segm : spannerSegments())
            static_cast<GlissandoSegment*>(segm)->layout();

      // compute glissando bbox as the bbox of the last segment, relative to the end anchor note
      QPointF anchor2PagePos = anchor2->pagePos();
      QPointF system2PagePos = cr2->segment()->system()->pagePos();
      QPointF anchor2SystPos = anchor2PagePos - system2PagePos;
      QRectF r = QRectF(anchor2SystPos - segm2->pos(), anchor2SystPos - segm2->pos() - segm2->pos2()).normalized();
      qreal lw = _spatium * lineWidth().val() * .5;
      setbbox(r.adjusted(-lw, -lw, lw, lw));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Glissando::write(Xml& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      if (_showText && !_text.isEmpty())
            xml.tag("text", _text);
      xml.tag("subtype", int(_glissandoType));
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Glissando::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();
      e.addSpanner(e.intAttribute("id", -1), this);

      _showText = false;
      while (e.readNextStartElement()) {
            const QStringRef& tag = e.name();
            if (tag == "text") {
                  _showText = true;
                  _text = e.readElementText();
                  }
            else if (tag == "subtype")
                  _glissandoType = Type(e.readInt());
            else if (!SLine::readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------
/*
void Glissando::draw(QPainter* painter) const
      {
      painter->save();
      qreal _spatium = spatium();

      QPen pen(curColor());
      pen.setWidthF(_spatium * .15);
      pen.setCapStyle(Qt::RoundCap);
      painter->setPen(pen);

      qreal w = line.dx();
      qreal h = line.dy();

      qreal l = sqrt(w * w + h * h);
      painter->translate(line.p1());
      qreal wi = asin(-h / l) * 180.0 / M_PI;
      painter->rotate(-wi);

      if (glissandoType() == Type::STRAIGHT) {
            painter->drawLine(QLineF(0.0, 0.0, l, 0.0));
            }
      else if (glissandoType() == Type::WAVY) {
            QRectF b = symBbox(SymId::wiggleTrill);
            qreal w  = symWidth(SymId::wiggleTrill);
            int n    = (int)(l / w);      // always round down (truncate) to avoid overlap
            qreal x  = (l - n*w) * 0.5;   // centre line in available space
            drawSymbol(SymId::wiggleTrill, painter, QPointF(x, b.height()*.5), n);
            }
      if (_showText) {
            const TextStyle& st = score()->textStyle(TextStyleType::GLISSANDO);
            QFont f = st.fontPx(_spatium);
            QRectF r = QFontMetricsF(f).boundingRect(_text);
            // if text longer than available space, skip it
            if (r.width() < l) {
                  qreal yOffset = r.height() + r.y();       // find text descender height
                  // raise text slightly above line and slightly more with WAVY than with STRAIGHT
                  yOffset += _spatium * (glissandoType() == Type::WAVY ? 0.75 : 0.05);
                  painter->setFont(f);
                  qreal x = (l - r.width()) * 0.5;
                  painter->drawText(QPointF(x, -yOffset), _text);
                  }
            }
      painter->restore();
      }
*/
//---------------------------------------------------------
//   space
//---------------------------------------------------------

Space Glissando::space() const
      {
      return Space(0.0, spatium() * 2.0);
      }

//---------------------------------------------------------
//   computeStartElement
//---------------------------------------------------------
/*
void Glissando::computeStartElement()
      {
      // if there is already a start note, done.
      if (_startElement != nullptr && _startElement->type() == Element::Type::NOTE)
            return;
      // if neither a start note or an end note, we got a problem!
      if (_endElement == nullptr || _endElement->type() != Element::Type::NOTE) {
            // TODO: no start, no end: we probably should delete this glissando or just abort() ?
            return;
            }

      int         trk   = track();
      Part*       part  = _endElement->staff()->part();
      Segment*    segm  = static_cast<Note*>(_endElement)->chord()->segment();

      if (segm != nullptr)
            segm = segm->prev1();
      while (segm) {
            // if previous segment is a ChordRest segment
            if (segm->segmentType() == Segment::Type::ChordRest) {
                  // look for a Chord in the same track and get its top note, if found
                  if (segm->element(trk) && segm->element(trk)->type() == Element::Type::CHORD) {
                        _startElement = static_cast<Chord*>(segm->element(trk))->upNote();
                        _startElement->add(this);
                        return;
                        }
                  // if no chord, look for other chords in the same instrument
                  for (Element* currChord : segm->elist())
                        if (currChord != nullptr && currChord->type() == Element::Type::CHORD
                                    && static_cast<Chord*>(currChord)->staff()->part() == part) {
                              _startElement = static_cast<Chord*>(currChord->upNote();
                              _startElement->add(this);
                              return;
                        }
                  }
            segm = segm->prev1();
            }

      // we have a problem! delete this glissando? abort()?
      qDebug("no first note for glissando found");
      }
*/
//---------------------------------------------------------
//   undoSetGlissandoType
//---------------------------------------------------------

void Glissando::undoSetGlissandoType(Type t)
      {
      score()->undoChangeProperty(this, P_ID::GLISS_TYPE, int(t));
      }

//---------------------------------------------------------
//   undoSetText
//---------------------------------------------------------

void Glissando::undoSetText(const QString& s)
      {
      score()->undoChangeProperty(this, P_ID::GLISS_TEXT, s);
      }

//---------------------------------------------------------
//   undoSetShowText
//---------------------------------------------------------

void Glissando::undoSetShowText(bool f)
      {
      score()->undoChangeProperty(this, P_ID::GLISS_SHOW_TEXT, f);
      }

//---------------------------------------------------------
//   STATIC FUNCTIONS: guessInitialNote
//
//    Used while reading old scores to determine (guess!) the glissando initial note
//    from its final chord. Returns the top note of previous chord of the same instrument,
//    preferring the chord in the same track as chord, if it exists.
//
//    Can be called when the final chord and/or its segment do not exist yet in the score
//    (i.e. while reading the chord itself).
//
//    Parameter:  chord: the chord this glissando ends into
//    Returns:    the top note in a suitable previous chord or nullptr if none found.
//---------------------------------------------------------

Note* Glissando::guessInitialNote(Chord* chord)
      {
      int         chordTrack  = chord->track();
      Segment*    segm        = chord->segment();
      Part*       part        = chord->staff()->part();
      if (segm != nullptr)
            segm = segm->prev1();
      while (segm) {
            // if previous segment is a ChordRest segment
            if (segm->segmentType() == Segment::Type::ChordRest) {
                  // look for a Chord in the same track and returns its top note, if found
                  if (segm->element(chordTrack) && segm->element(chordTrack)->type() == Element::Type::CHORD)
                        return static_cast<Chord*>(segm->element(chordTrack))->upNote();
                  // if no chord, look for other chords in the same instrument
                  for (Element* currChord : segm->elist())
                        if (currChord != nullptr && currChord->type() == Element::Type::CHORD
                                    && static_cast<Chord*>(currChord)->staff()->part() == part)
                              return static_cast<Chord*>(currChord)->upNote();
                  }
            segm = segm->prev1();
            }
      qDebug("no first note for glissando found");
      return nullptr;
      }

//---------------------------------------------------------
//   STATIC FUNCTIONS: guessFinalNote
//
//    Used while dropping a glissando on a note to determine (guess!) the glissando final note from its initial chord.
//    Returns the top note of next chord of the same instrument,
//    preferring the chord in the same track as chord, if it exists.
//
//    Parameter:  chord: the chord this glissando start from
//    Returns:    the top note in a suitable following chord or nullptr if none found
//---------------------------------------------------------

Note* Glissando::guessFinalNote(Chord* chord)
      {
      int         chordTrack  = chord->track();
      Segment*    segm        = chord->segment();
      Part*       part        = chord->staff()->part();
      if (segm != nullptr)
            segm = segm->next1();
      while (segm) {
            // if next segment is a ChordRest segment
            if (segm->segmentType() == Segment::Type::ChordRest) {
                  // look for a Chord in the same track and returns its top note, if found
                  if (segm->element(chordTrack) && segm->element(chordTrack)->type() == Element::Type::CHORD)
                        return static_cast<Chord*>(segm->element(chordTrack))->upNote();
                  // if no chord, look for other chords in the same instrument
                  for (Element* currChord : segm->elist())
                        if (currChord != nullptr && currChord->type() == Element::Type::CHORD
                                    && static_cast<Chord*>(currChord)->staff()->part() == part)
                              return static_cast<Chord*>(currChord)->upNote();
                  }
            segm = segm->next1();
            }
      qDebug("no second note for glissando found");
      return nullptr;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Glissando::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::GLISS_TYPE:
                  return int(glissandoType());
            case P_ID::GLISS_TEXT:
                  return text();
            case P_ID::GLISS_SHOW_TEXT:
                  return showText();
            default:
                  break;
            }
      return SLine::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Glissando::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_ID::GLISS_TYPE:
                  setGlissandoType(Type(v.toInt()));
                  break;
            case P_ID::GLISS_TEXT:
                  setText(v.toString());
                  break;
            case P_ID::GLISS_SHOW_TEXT:
                  setShowText(v.toBool());
                  break;
            default:
                  if (!SLine::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll(true);
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Glissando::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::GLISS_TYPE:
                  return int(Type::STRAIGHT);
            case P_ID::GLISS_TEXT:
                  return "gliss.";
            case P_ID::GLISS_SHOW_TEXT:
                  return true;
            default:
                  break;
            }
      return SLine::propertyDefault(propertyId);
      }

}

