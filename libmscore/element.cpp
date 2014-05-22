//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 \file
 Implementation of Element, ElementList, StaffLines.
*/

#include "element.h"
#include "style.h"
#include "xml.h"
#include "score.h"
#include "staff.h"
#include "utils.h"
#include "sym.h"
#include "symbol.h"
#include "clef.h"
#include "volta.h"
#include "ottava.h"
#include "textline.h"
#include "trill.h"
#include "pedal.h"
#include "hairpin.h"
#include "keysig.h"
#include "timesig.h"
#include "barline.h"
#include "arpeggio.h"
#include "breath.h"
#include "bracket.h"
#include "chordrest.h"
#include "accidental.h"
#include "dynamic.h"
#include "text.h"
#include "note.h"
#include "tremolo.h"
#include "layoutbreak.h"
#include "repeat.h"
#include "page.h"
#include "system.h"
#include "stafftext.h"
#include "glissando.h"
#include "articulation.h"
#include "chord.h"
#include "spacer.h"
#include "tempotext.h"
#include "harmony.h"
#include "lyrics.h"
#include "rest.h"
#include "slur.h"
#include "measure.h"
#include "fret.h"
#include "staffstate.h"
#include "fingering.h"
#include "bend.h"
#include "tremolobar.h"
#include "chordline.h"
#include "undo.h"
#include "segment.h"
#include "box.h"
#include "instrchange.h"
#include "stafftype.h"
#include "stem.h"
#include "iname.h"
#include "mscore.h"
#include "icon.h"
#include "ossia.h"
#include "figuredbass.h"
#include "rehearsalmark.h"
#include "notedot.h"
#include "textframe.h"
#include "image.h"
#include "marker.h"
#include "jump.h"
#include "noteline.h"
#include "bagpembell.h"
#include "ambitus.h"

namespace Ms {

// extern bool showInvisible;

//
// list has to be synchronized with ElementType enum
//
static const ElementName elementNames[] = {
      ElementName("invalid",              QT_TRANSLATE_NOOP("elementName", "invalid")),
      ElementName("Symbol",               QT_TRANSLATE_NOOP("elementName", "Symbol")),
      ElementName("Text",                 QT_TRANSLATE_NOOP("elementName", "Text")),
      ElementName("InstrumentName",       QT_TRANSLATE_NOOP("elementName", "Instrument Name")),
      ElementName("SlurSegment",          QT_TRANSLATE_NOOP("elementName", "Slur Segment")),
      ElementName("StaffLines",           QT_TRANSLATE_NOOP("elementName", "Staff Lines")),
      ElementName("BarLine",              QT_TRANSLATE_NOOP("elementName", "Bar Line")),
      ElementName("StemSlash",            QT_TRANSLATE_NOOP("elementName", "Stem Slash")),
      ElementName("Line",                 QT_TRANSLATE_NOOP("elementName", "Line")),
      ElementName("Bracket",              QT_TRANSLATE_NOOP("elementName", "Bracket")),

      ElementName("Arpeggio",             QT_TRANSLATE_NOOP("elementName", "Arpeggio")),
      ElementName("Accidental",           QT_TRANSLATE_NOOP("elementName", "Accidental")),
      ElementName("Stem",                 QT_TRANSLATE_NOOP("elementName", "Stem")),
      ElementName("Note",                 QT_TRANSLATE_NOOP("elementName", "Note")),
      ElementName("Clef",                 QT_TRANSLATE_NOOP("elementName", "Clef")),
      ElementName("KeySig",               QT_TRANSLATE_NOOP("elementName", "Key Signature")),
      ElementName("Ambitus",              QT_TRANSLATE_NOOP("elementName", "Ambitus")),
      ElementName("TimeSig",              QT_TRANSLATE_NOOP("elementName", "Time Signature")),
      ElementName("Rest",                 QT_TRANSLATE_NOOP("elementName", "Rest")),
      ElementName("Breath",               QT_TRANSLATE_NOOP("elementName", "Breath")),
      ElementName("Glissando",            QT_TRANSLATE_NOOP("elementName", "Glissando")),
      ElementName("RepeatMeasure",        QT_TRANSLATE_NOOP("elementName", "Repeat Measure")),
      ElementName("Image",                QT_TRANSLATE_NOOP("elementName", "Image")),
      ElementName("Tie",                  QT_TRANSLATE_NOOP("elementName", "Tie")),
      ElementName("Articulation",         QT_TRANSLATE_NOOP("elementName", "Articulation")),
      ElementName("ChordLine",            QT_TRANSLATE_NOOP("elementName", "Chord Line")),
      ElementName("Dynamic",              QT_TRANSLATE_NOOP("elementName", "Dynamic")),
      ElementName("Beam",                 QT_TRANSLATE_NOOP("elementName", "Beam")),
      ElementName("Hook",                 QT_TRANSLATE_NOOP("elementName", "Hook")),
      ElementName("Lyrics",               QT_TRANSLATE_NOOP("elementName", "Lyrics")),
      ElementName("FiguredBass",          QT_TRANSLATE_NOOP("elementName", "Figured Bass")),
      ElementName("Marker",               QT_TRANSLATE_NOOP("elementName", "Marker")),
      ElementName("Jump",                 QT_TRANSLATE_NOOP("elementName", "Jump")),
      ElementName("Fingering",            QT_TRANSLATE_NOOP("elementName", "Fingering")),
      ElementName("Tuplet",               QT_TRANSLATE_NOOP("elementName", "Tuplet")),
      ElementName("Tempo",                QT_TRANSLATE_NOOP("elementName", "Tempo")),
      ElementName("StaffText",            QT_TRANSLATE_NOOP("elementName", "Staff Text")),
      ElementName("RehearsalMark",        QT_TRANSLATE_NOOP("elementName", "Rehearsal Mark")),
      ElementName("InstrumentChange",     QT_TRANSLATE_NOOP("elementName", "Instrument Change")),
      ElementName("Harmony",              QT_TRANSLATE_NOOP("elementName", "Chord Symbol")),
      ElementName("FretDiagram",          QT_TRANSLATE_NOOP("elementName", "Fretboard Diagram")),
      ElementName("Bend",                 QT_TRANSLATE_NOOP("elementName", "Bend")),
      ElementName("TremoloBar",           QT_TRANSLATE_NOOP("elementName", "Tremolo Bar")),
      ElementName("Volta",                QT_TRANSLATE_NOOP("elementName", "Volta")),
      ElementName("HairpinSegment",       QT_TRANSLATE_NOOP("elementName", "Hairpin Segment")),
      ElementName("OttavaSegment",        QT_TRANSLATE_NOOP("elementName", "Ottava Segment")),
      ElementName("TrillSegment",         QT_TRANSLATE_NOOP("elementName", "Trill Segment")),
      ElementName("TextLineSegment",      QT_TRANSLATE_NOOP("elementName", "Text Line Segment")),
      ElementName("VoltaSegment",         QT_TRANSLATE_NOOP("elementName", "Volta Segment")),
      ElementName("PedalSegment",         QT_TRANSLATE_NOOP("elementName", "Pedal Segment")),
      ElementName("LayoutBreak",          QT_TRANSLATE_NOOP("elementName", "Layout Break")),
      ElementName("Spacer",               QT_TRANSLATE_NOOP("elementName", "Spacer")),
      ElementName("StaffState",           QT_TRANSLATE_NOOP("elementName", "Staff State")),
      ElementName("LedgerLine",           QT_TRANSLATE_NOOP("elementName", "Ledger Line")),
      ElementName("NoteHead",             QT_TRANSLATE_NOOP("elementName", "Note Head")),
      ElementName("NoteDot",              QT_TRANSLATE_NOOP("elementName", "Note Dot")),
      ElementName("Tremolo",              QT_TRANSLATE_NOOP("elementName", "Tremolo")),
      ElementName("Measure",              QT_TRANSLATE_NOOP("elementName", "Measure")),
      ElementName("Selection",            QT_TRANSLATE_NOOP("elementName", "Selection")),
      ElementName("Lasso",                QT_TRANSLATE_NOOP("elementName", "Lasso")),
      ElementName("ShadowNote",           QT_TRANSLATE_NOOP("elementName", "Shadow Note")),
      ElementName("RubberBand",           QT_TRANSLATE_NOOP("elementName", "Rubber Band")),
      ElementName("TabDurationSymbol",    QT_TRANSLATE_NOOP("elementName", "Tab Duration Symbol")),
      ElementName("FSymbol",              QT_TRANSLATE_NOOP("elementName", "Font Symbol")),
      ElementName("Page",                 QT_TRANSLATE_NOOP("elementName", "Page")),
      ElementName("HairPin",              QT_TRANSLATE_NOOP("elementName", "Hairpin")),
      ElementName("Ottava",               QT_TRANSLATE_NOOP("elementName", "Ottava")),
      ElementName("Pedal",                QT_TRANSLATE_NOOP("elementName", "Pedal")),
      ElementName("Trill",                QT_TRANSLATE_NOOP("elementName", "Trill")),
      ElementName("TextLine",             QT_TRANSLATE_NOOP("elementName", "Text Line")),
      ElementName("NoteLine",             QT_TRANSLATE_NOOP("elementName", "Note Line")),
      ElementName("Segment",              QT_TRANSLATE_NOOP("elementName", "Segment")),
      ElementName("System",               QT_TRANSLATE_NOOP("elementName", "System")),
      ElementName("Compound",             QT_TRANSLATE_NOOP("elementName", "Compound")),
      ElementName("Chord",                QT_TRANSLATE_NOOP("elementName", "Chord")),
      ElementName("Slur",                 QT_TRANSLATE_NOOP("elementName", "Slur")),
      ElementName("Element",              QT_TRANSLATE_NOOP("elementName", "Element")),
      ElementName("ElementList",          QT_TRANSLATE_NOOP("elementName", "Element List")),
      ElementName("StaffList",            QT_TRANSLATE_NOOP("elementName", "Staff List")),
      ElementName("MeasureList",          QT_TRANSLATE_NOOP("elementName", "Measure List")),
      ElementName("Layout",               QT_TRANSLATE_NOOP("elementName", "Layout")),
      ElementName("HBox",                 QT_TRANSLATE_NOOP("elementName", "Horizontal Frame")),
      ElementName("VBox",                 QT_TRANSLATE_NOOP("elementName", "Vertical Frame")),
      ElementName("TBox",                 QT_TRANSLATE_NOOP("elementName", "Text Frame")),
      ElementName("FBox",                 QT_TRANSLATE_NOOP("elementName", "Fretboard Diagram Frame")),
      ElementName("AccidentalBracket",    QT_TRANSLATE_NOOP("elementName", "Accidental Bracket")),
      ElementName("Icon",                 QT_TRANSLATE_NOOP("elementName", "Icon")),
      ElementName("Ossia",                QT_TRANSLATE_NOOP("elementName", "Ossia")),
      ElementName("BagpipeEmbellishment", QT_TRANSLATE_NOOP("elementName", "Bagpipe Embellishment"))
      };

//---------------------------------------------------------
//   DropData
//---------------------------------------------------------

DropData::DropData()
      {
      view = 0;
      element = 0;
      duration = Fraction(1,4);
      modifiers = 0;
      }

//---------------------------------------------------------
//   LinkedElements
//---------------------------------------------------------

LinkedElements::LinkedElements(Score* score)
      {
      _lid = score->linkId(); // create new unique id
      }

LinkedElements::LinkedElements(Score* score, int id)
      {
      _lid = id;
      score->linkId(id);      // remember used id
      }

//---------------------------------------------------------
//   setLid
//---------------------------------------------------------

void LinkedElements::setLid(Score* score, int id)
      {
      _lid = id;
      score->linkId(id);
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Element::spatiumChanged(qreal oldValue, qreal newValue)
      {
      _userOff *= (newValue / oldValue);
      _readPos *= (newValue / oldValue);
      }

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

qreal Element::spatium() const
      {
      Staff* s = staff();
      return s ? s->spatium() : _score->spatium();
      }

//---------------------------------------------------------
//   magS
//---------------------------------------------------------

qreal Element::magS() const
      {
      return mag() * (_score->spatium() /(MScore::DPI * SPATIUM20));
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* Element::name() const
      {
      return name(type());
      }

//---------------------------------------------------------
//   userName
//---------------------------------------------------------

QString Element::userName() const
      {
      return qApp->translate("elementName", elementNames[type()].userName);
      }

//---------------------------------------------------------
//   ~Element
//---------------------------------------------------------

Element::~Element()
      {
      if (_links) {
            _links->removeOne(this);
            if (_links->isEmpty()) {
                  //DEBUG:
                  score()->links().remove(_links->lid());
                  //
                  delete _links;
                  }
            }
      }

//---------------------------------------------------------
//   Element
//---------------------------------------------------------

Element::Element(Score* s) :
   QObject(0)
      {
      _selected      = false;
      _generated     = false;
      _visible       = true;
      _placement     = Placement::BELOW;
      _flags         = ElementFlag::SELECTABLE;
      _track         = -1;
      _color         = MScore::defaultColor;
      _mag           = 1.0;
      _tag           = 1;
      _score         = s;
      itemDiscovered = false;
      }

Element::Element(const Element& e)
   : QObject(0)
      {
      _parent     = e._parent;
      _selected   = e._selected;
      _generated  = e._generated;
      _visible    = e._visible;
      _placement  = e._placement;
      _flags      = e._flags;
      _track      = e._track;
      _color      = e._color;
      _mag        = e._mag;
      _pos        = e._pos;
      _userOff    = e._userOff;
      _readPos    = e._readPos;
      _score      = e._score;
      _bbox       = e._bbox;
      _tag        = e._tag;
      itemDiscovered = false;
      }

//---------------------------------------------------------
//   linkTo
//---------------------------------------------------------

void Element::linkTo(Element* element)
      {
      Q_ASSERT(element != this);
      if (!_links) {
            if (element->links()) {
                  _links = element->_links;
                  Q_ASSERT(_links->contains(element));
                  }
            else {
                  _links = new LinkedElements(score());
                  _links->append(element);
                  element->_links = _links;
                  }
            Q_ASSERT(!_links->contains(this));
            _links->append(this);
            }
      else {
            _links->append(element);
            element->_links = _links;
            }
      }

//---------------------------------------------------------
//   unlink
//---------------------------------------------------------

void Element::unlink()
      {
      if (_links) {
            Q_ASSERT(_links->contains(this));
            _links->removeOne(this);
            _links = 0;
            }
      }

//---------------------------------------------------------
//   undoUnlink
//---------------------------------------------------------

void Element::undoUnlink()
      {
      if (_links)
            _score->undo(new Unlink(this));
      }

//---------------------------------------------------------
//   linkedClone
//---------------------------------------------------------

Element* Element::linkedClone()
      {
      Element* e = clone();
      linkTo(e);
      return e;
      }

//---------------------------------------------------------
//   linkList
//---------------------------------------------------------

QList<Element*> Element::linkList() const
      {
      QList<Element*> el;
      if (links())
            el.append(*links());
      else
            el.append((Element*)this);
      return el;
      }

//---------------------------------------------------------
//   adjustReadPos
//---------------------------------------------------------

void Element::adjustReadPos()
      {
      if (!_readPos.isNull()) {
            _userOff = _readPos - _pos;
            _readPos = QPointF();
            }
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Element::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (all || _visible || score()->showInvisible())
            func(data, this);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Element::reset()
      {
      if (!_userOff.isNull())
            score()->undoChangeProperty(this, P_USER_OFF, QPointF());
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void Element::change(Element* o, Element* n)
      {
      remove(o);
      add(n);
      }

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

Staff* Element::staff() const
      {
      if (_track == -1 || score()->staves().isEmpty())
            return 0;

      return score()->staff(staffIdx());
      }

//---------------------------------------------------------
//   curColor
//---------------------------------------------------------

QColor Element::curColor() const
      {
      return curColor(this);
      }

//---------------------------------------------------------
//   curColor
//---------------------------------------------------------

QColor Element::curColor(const Element* proxy) const
      {
      // the default element color is always interpreted as black in
      // printing
      if (score() && score()->printing())
            return (proxy->color() == MScore::defaultColor) ? Qt::black : proxy->color();

      if (flag(ElementFlag::DROP_TARGET))
            return MScore::dropColor;
      bool marked = false;
      if (type() == Element::NOTE) {
            const Note* note = static_cast<const Note*>(this);
            marked = note->mark();
            }
      if (proxy->selected() || marked ) {
            if (track() == -1)
                  return MScore::selectColor[0];
            else
                  return MScore::selectColor[voice()];
            }
      if (!proxy->visible())
            return Qt::gray;
      return proxy->color();
      }

//---------------------------------------------------------
//   drag
///   Return update Rect relative to canvas.
//---------------------------------------------------------

QRectF Element::drag(EditData* data)
      {
      QRectF r(canvasBoundingRect());

      qreal x = data->delta.x();
      qreal y = data->delta.y();

      qreal _spatium = spatium();
      if (data->hRaster) {
            qreal hRaster = _spatium / MScore::hRaster();
            int n = lrint(x / hRaster);
            x = hRaster * n;
            }
      if (data->vRaster) {
            qreal vRaster = _spatium / MScore::vRaster();
            int n = lrint(y / vRaster);
            y = vRaster * n;
            }
      setUserOff(QPointF(x, y));
      setGenerated(false);
      return canvasBoundingRect() | r;
      }

//---------------------------------------------------------
//   pagePos
//    return position in canvas coordinates
//---------------------------------------------------------

QPointF Element::pagePos() const
      {
      QPointF p(pos());
      if (parent() == 0)
            return p;

      if (_flags & ElementFlag::ON_STAFF) {
            System* system;
            if (parent()->type() == SEGMENT)
                  system = static_cast<Segment*>(parent())->measure()->system();
            else if (parent()->type() == MEASURE)     // used in measure number
                  system = static_cast<Measure*>(parent())->system();
            else if (parent()->type() == SYSTEM)
                  system = static_cast<System*>(parent());
            else
                  Q_ASSERT(false);
            if (system) {
                  int si = staffIdx();
                  if (type() == CHORD || type() == REST)
                        si += static_cast<const ChordRest*>(this)->staffMove();
                  p.ry() += system->staffYpage(si); // system->staff(si)->y() + system->y();
                  }
            p.rx() = pageX();
            }
      else {
            if (parent()->parent())
                  p += parent()->pagePos();
            }
      return p;
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Element::canvasPos() const
      {
      QPointF p(pos());
      if (parent() == 0)
            return p;

      if (_flags & ElementFlag::ON_STAFF) {
            System* system;
            if (parent()->type() == SEGMENT)
                  system = static_cast<Segment*>(parent())->measure()->system();
            else if (parent()->type() == MEASURE)     // used in measure number
                  system = static_cast<Measure*>(parent())->system();
            else if (parent()->type() == SYSTEM)
                  system = static_cast<System*>(parent());
            else
                  Q_ASSERT(false);
            if (system) {
                  int si = staffIdx();
                  if (type() == CHORD || type() == REST)
                        si += static_cast<const ChordRest*>(this)->staffMove();
                  p.ry() += system->staffYpage(si); // system->staff(si)->y() + system->y();
                  Page* page = system->page();
                  if (page)
                        p.ry() += page->y();
                  }
            p.rx() = canvasX();
            }
      else {
            p += parent()->canvasPos();
            }
      return p;
      }

//---------------------------------------------------------
//   pageX
//---------------------------------------------------------

qreal Element::pageX() const
      {
      qreal xp = x();
      for (Element* e = parent(); e && e->parent(); e = e->parent())
            xp += e->x();
      return xp;
      }

//---------------------------------------------------------
//    canvasX
//---------------------------------------------------------

qreal Element::canvasX() const
      {
      qreal xp = x();
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      return xp;
      }

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

/**
 Return true if \a p is inside the shape of the object.

 Note: \a p is in page coordinates
*/

bool Element::contains(const QPointF& p) const
      {
      return shape().contains(p - pagePos());
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

/**
  Returns the shape of this element as a QPainterPath in local
  coordinates. The shape is used for collision detection and
  hit tests (contains())

  The default implementation calls bbox() to return a simple rectangular
  shape, but subclasses can reimplement this function to return a more
  accurate shape for non-rectangular elements.
*/

QPainterPath Element::shape() const
      {
      QPainterPath pp;
      pp.addRect(bbox());
      return pp;
      }

//---------------------------------------------------------
//  intersects
//---------------------------------------------------------

/**
 Return true if \a rr intersects bounding box of object.

 Note: \a rr is in page coordinates
*/

bool Element::intersects(const QRectF& rr) const
      {
      return shape().intersects(rr.translated(-pagePos()));
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Element::writeProperties(Xml& xml) const
      {
      //copy paste should not keep links
      if (_links && (_links->size() > 1) && !xml.clipboardmode)
            xml.tag("lid", _links->lid());
      if (!userOff().isNull()) {
            if (type() == VOLTA_SEGMENT || isChordRest())
                  xml.tag("offset", userOff() / spatium());
            else
                  xml.tag("pos", pos() / spatium());
            }
      if (((track() != xml.curTrack) || (type() == SLUR)) && (track() != -1)) {
            int t;
            t = track() + xml.trackDiff;
            xml.tag("track", t);
            }
      if (_tag != 0x1) {
            for (int i = 1; i < MAX_TAGS; i++) {
                  if (_tag == ((unsigned)1 << i)) {
                        xml.tag("tag", score()->layerTags()[i]);
                        break;
                        }
                  }
            }
      if (color() != Qt::black)
            xml.tag("color", color());
      if (!visible())
            xml.tag("visible", visible());
      writeProperty(xml, P_PLACEMENT);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Element::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "track")
            setTrack(e.readInt());
      else if (tag == "color")
            setColor(e.readColor());
      else if (tag == "visible")
            setVisible(e.readInt());
      else if (tag == "selected") // obsolete
            e.readInt();
      else if (tag == "userOff")
            _userOff = e.readPoint();
      else if (tag == "lid") {
            int id = e.readInt();
            _links = score()->links().value(id);
            if (!_links) {
                  if (score()->parentScore())   // DEBUG
                        qDebug("---link %d not found (%d)", id, score()->links().size());
                  _links = new LinkedElements(score(), id);
                  score()->links().insert(id, _links);
                  }
#ifndef NDEBUG
            else {
                  foreach(Element* ee, *_links) {
                        if (ee->type() != type()) {
                              qFatal("link %s(%d) type mismatch %s linked to %s",
                                 ee->name(), id, ee->name(), name());
                              }
                        }
                  }
#endif
            Q_ASSERT(!_links->contains(this));
            _links->append(this);
            }
      else if (tag == "tick") {
            int val = e.readInt();
            if (val >= 0 && type() != SYMBOL  && ((type() != GLISSANDO && type() != FINGERING && type() != STAFF_TEXT) || score()->mscVersion() > 114))   // hack for 1.2
                  e.setTick(score()->fileDivision(val));
            }
      else if (tag == "offset")
            setUserOff(e.readPoint() * spatium());
      else if (tag == "pos") {
            QPointF pt = e.readPoint();
            if (score()->mscVersion() > 114)
                  _readPos = pt * spatium();
            }
      else if (tag == "voice")
            setTrack((_track/VOICES)*VOICES + e.readInt());
      else if (tag == "tag") {
            QString val(e.readElementText());
            for (int i = 1; i < MAX_TAGS; i++) {
                  if (score()->layerTags()[i] == val) {
                        _tag = 1 << i;
                        break;
                        }
                  }
            }
      else if (tag == "placement")
            _placement = Placement(Ms::getProperty(P_PLACEMENT, e).toInt());
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   writeProperty
//---------------------------------------------------------

void Element::writeProperty(Xml& xml, P_ID id) const
      {
      xml.tag(id, getProperty(id), propertyDefault(id));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Element::write(Xml& xml) const
      {
      xml.stag(name());
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Element::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Element::startEdit(MuseScoreView*, const QPointF&)
      {
      undoPushProperty(P_USER_OFF);
      }

//---------------------------------------------------------
//   remove
///   Remove \a el from the list. Return true on success.
//---------------------------------------------------------

bool ElementList::remove(Element* el)
      {
      auto i = find(begin(), end(), el);
      if (i == end())
            return false;
      erase(i);
      return true;
      }

//---------------------------------------------------------
//   replace
//---------------------------------------------------------

void ElementList::replace(Element* o, Element* n)
      {
      auto i = find(begin(), end(), o);
      if (i == end()) {
            qDebug("ElementList::replace: element not found");
            return;
            }
      *i = n;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ElementList::write(Xml& xml) const
      {
      for (const Element* e : *this)
            e->write(xml);
      }

//---------------------------------------------------------
//   StaffLines
//---------------------------------------------------------

StaffLines::StaffLines(Score* s)
   : Element(s)
      {
      setWidth(1.0);      // dummy
      lines = 5;
      setSelectable(false);
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF StaffLines::pagePos() const
      {
      System* system = measure()->system();
      return QPointF(measure()->x() + system->x(),
         system->staff(staffIdx())->y() + system->y());
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF StaffLines::canvasPos() const
      {
      QPointF p(pagePos());
      Element* e = parent();
      while (e) {
            if (e->type() == PAGE) {
                  p += e->pos();
                  break;
                  }
            e = e->parent();
            }
      return p;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffLines::layout()
      {
      StaffType* st = staff() ? staff()->staffType() : 0;
      qreal _spatium = spatium();
      if (st) {
            dist  = st->lineDistance().val() * _spatium;
            lines = st->lines();
            }
      else {
            dist  = _spatium;
            lines = 5;
            }

//      qDebug("StaffLines::layout:: dist %f st %p", dist, st);

      setColor(staff() ? staff()->color() : MScore::defaultColor);

      lw = score()->styleS(ST_staffLineWidth).val() * _spatium;
      bbox().setRect(0.0, -lw*.5, width(), lines * dist + lw);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffLines::draw(QPainter* painter) const
      {
      QPointF _pos(0.0, 0.0);

      qreal x1 = _pos.x();
      qreal x2 = x1 + width();

      QVector<QLineF> ll(lines);
      qreal y = _pos.y();
      for (int i = 0; i < lines; ++i) {
            ll[i].setLine(x1, y, x2, y);
            y += dist;
            }
      if (MScore::debugMode) {
            painter->setPen(QPen(Qt::lightGray, lw, Qt::SolidLine, Qt::FlatCap));
            y = _pos.y() - 3 * dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            y = _pos.y() - 2 * dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            y = _pos.y() - dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            y = _pos.y() + lines * dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            y = _pos.y() + (lines+1) * dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            y = _pos.y() + (lines+2) * dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            y = _pos.y() + (lines+3) * dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            y = _pos.y() + (lines+4) * dist;
            painter->drawLine(QLineF(x1, y, x2, y));
            }

      painter->setPen(QPen(curColor(), lw, Qt::SolidLine, Qt::FlatCap));
      painter->drawLines(ll);
      }

//---------------------------------------------------------
//   y1
//---------------------------------------------------------

qreal StaffLines::y1() const
      {
      System* system = measure()->system();
      if (system == 0)
            return 0.0;

      return system->staff(staffIdx())->y() + ipos().y();
      }

//---------------------------------------------------------
//   Line
//---------------------------------------------------------

Line::Line(Score* s, bool v)
   : Element(s)
      {
      vertical = v;
      _z = LINE * 100;
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Line::dump() const
      {
      qDebug("  width:%g height:%g vert:%d", point(_width), point(_len), vertical);
      }

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void Line::setLen(Spatium l)
      {
      _len = l;
      }

//---------------------------------------------------------
//   setLineWidth
//---------------------------------------------------------

void Line::setLineWidth(Spatium w)
      {
      _width = w;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Line::layout()
      {
      qreal sp = spatium();
      qreal w  = _width.val() * sp;
      qreal l  = _len.val() * sp;
      qreal w2 = w * .5;
      if (vertical)
            bbox().setRect(-w2, -w2, w, l + w);
      else
            bbox().setRect(-w2, -w2, l + w, w);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Line::draw(QPainter* painter) const
      {
      qreal sp = spatium();
      painter->setPen(QPen(curColor(), _width.val() * sp));

      qreal l = _len.val() * sp;
      if (vertical)
            painter->drawLine(QLineF(0.0, 0.0, 0.0, l));
      else
            painter->drawLine(QLineF(0.0, 0.0, l, 0.0));
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Line::writeProperties(Xml& xml) const
      {
      xml.tag("lineWidth", _width.val());
      xml.tag("lineLen", _len.val());
      if (!vertical)
            xml.tag("vertical", vertical);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Line::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "lineWidth")
            _width = Spatium(e.readDouble());
      else if (tag == "lineLen")
            _len = Spatium(e.readDouble());
      else if (tag == "vertical")
            vertical = e.readInt();
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   Compound
//---------------------------------------------------------

Compound::Compound(Score* s)
   : Element(s)
      {
      }

Compound::Compound(const Compound& c)
   : Element(c)
      {
      elements.clear();
      foreach(Element* e, c.elements)
            elements.append(e->clone());
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Compound::draw(QPainter* painter) const
      {
      foreach(Element* e, elements) {
            QPointF pt(e->pos());
            painter->translate(pt);
            e->draw(painter);
            painter->translate(-pt);
            }
      }

//---------------------------------------------------------
//   addElement
//---------------------------------------------------------

/**
 offset \a x and \a y are in Point units
*/

void Compound::addElement(Element* e, qreal x, qreal y)
      {
      e->setPos(x, y);
      e->setParent(this);
      elements.push_back(e);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Compound::layout()
      {
      setbbox(QRectF());
      for (auto i = elements.begin(); i != elements.end(); ++i) {
            Element* e = *i;
            e->layout();
            addbbox(e->bbox().translated(e->pos()));
            }
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Compound::setSelected(bool f)
      {
      Element::setSelected(f);
      for (auto i = elements.begin(); i != elements.end(); ++i)
            (*i)->setSelected(f);
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void Compound::setVisible(bool f)
      {
      Element::setVisible(f);
      for (auto i = elements.begin(); i != elements.end(); ++i)
            (*i)->setVisible(f);
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Compound::clear()
      {
      foreach(Element* e, elements) {
            if (e->selected())
                  score()->deselect(e);
            delete e;
            }
      elements.clear();
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Element::dump() const
      {
      qDebug("---Element: %s, pos(%4.2f,%4.2f)"
         "\n   bbox(%g,%g,%g,%g)"
         "\n   abox(%g,%g,%g,%g)"
         "\n  parent: %p",
         name(), ipos().x(), ipos().y(),
         _bbox.x(), _bbox.y(), _bbox.width(), _bbox.height(),
         abbox().x(), abbox().y(), abbox().width(), abbox().height(),
         parent());
      }

//---------------------------------------------------------
//   RubberBand::draw
//---------------------------------------------------------

void RubberBand::draw(QPainter* painter) const
      {
      if (!showRubberBand)
            return;
      painter->setPen(Qt::red);
      painter->drawLine(QLineF(_p1.x(), _p1.y(), _p2.x(), _p2.y()));
      }

//---------------------------------------------------------
//   mimeData
//---------------------------------------------------------

QByteArray Element::mimeData(const QPointF& dragOffset) const
      {
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      Xml xml(&buffer);
      xml.clipboardmode = true;
      xml.stag("Element");
      if (type() == NOTE)
            xml.fTag("duration", static_cast<const Note*>(this)->chord()->duration());
      if (!dragOffset.isNull())
            xml.tag("dragOffset", dragOffset);
      write(xml);
      xml.etag();
      buffer.close();
      return buffer.buffer();
      }

//---------------------------------------------------------
//   readType
//    return new position of QDomElement in e
//---------------------------------------------------------

Element::ElementType Element::readType(XmlReader& e, QPointF* dragOffset,
   Fraction* duration)
      {
      while (e.readNextStartElement()) {
            if (e.name() == "Element")
                  while (e.readNextStartElement()) {
                        const QStringRef& tag = e.name();
                        if (tag == "dragOffset")
                              *dragOffset = e.readPoint();
                        else if (tag == "duration")
                              *duration = e.readFraction();
                        else {
                              ElementType type = name2type(tag);
                              if (type == INVALID)
                                    break;
                              return type;
                        }
                  }
            else
                  e.unknown();
            }
      return INVALID;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Element::editDrag(const EditData& ed)
      {
      score()->addRefresh(abbox());
      setUserOff(userOff() + ed.delta);
      score()->addRefresh(abbox());
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Element::edit(MuseScoreView*, int, int key, Qt::KeyboardModifiers, const QString&)
      {
      if (key ==  Qt::Key_Home) {
            setUserOff(QPoint());
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Element::add(Element* e)
      {
      qDebug("Element: cannot add %s to %s", e->name(), name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Element::remove(Element* e)
      {
      qFatal("Element: cannot remove %s from %s", e->name(), name());
      }

//---------------------------------------------------------
//   create
//    Element factory
//---------------------------------------------------------

Element* Element::create(ElementType type, Score* score)
      {
      switch(type) {
            case VOLTA:             return new Volta(score);
            case OTTAVA:            return new Ottava(score);
            case TEXTLINE:          return new TextLine(score);
            case NOTELINE:          return new NoteLine(score);
            case TRILL:             return new Trill(score);
            case PEDAL:             return new Pedal(score);
            case HAIRPIN:           return new Hairpin(score);
            case CLEF:              return new Clef(score);
            case KEYSIG:            return new KeySig(score);
            case TIMESIG:           return new TimeSig(score);
            case BAR_LINE:          return new BarLine(score);
            case ARPEGGIO:          return new Arpeggio(score);
            case BREATH:            return new Breath(score);
            case GLISSANDO:         return new Glissando(score);
            case BRACKET:           return new Bracket(score);
            case ARTICULATION:      return new Articulation(score);
            case CHORDLINE:         return new ChordLine(score);
            case ACCIDENTAL:        return new Accidental(score);
            case DYNAMIC:           return new Dynamic(score);
            case TEXT:              return new Text(score);
            case INSTRUMENT_NAME:   return new InstrumentName(score);
            case STAFF_TEXT:        return new StaffText(score);
            case REHEARSAL_MARK:    return new RehearsalMark(score);
            case INSTRUMENT_CHANGE: return new InstrumentChange(score);
            case NOTEHEAD:          return new NoteHead(score);
            case NOTEDOT:           return new NoteDot(score);
            case TREMOLO:           return new Tremolo(score);
            case LAYOUT_BREAK:      return new LayoutBreak(score);
            case MARKER:            return new Marker(score);
            case JUMP:              return new Jump(score);
            case REPEAT_MEASURE:    return new RepeatMeasure(score);
            case ICON:              return new Icon(score);
            case NOTE:              return new Note(score);
            case SYMBOL:            return new Symbol(score);
            case FSYMBOL:           return new FSymbol(score);
            case CHORD:             return new Chord(score);
            case REST:              return new Rest(score);
            case SPACER:            return new Spacer(score);
            case STAFF_STATE:       return new StaffState(score);
            case TEMPO_TEXT:        return new TempoText(score);
            case HARMONY:           return new Harmony(score);
            case FRET_DIAGRAM:      return new FretDiagram(score);
            case BEND:              return new Bend(score);
            case TREMOLOBAR:        return new TremoloBar(score);
            case LYRICS:            return new Lyrics(score);
            case FIGURED_BASS:      return new FiguredBass(score);
            case STEM:              return new Stem(score);
            case SLUR:              return new Slur(score);
            case ACCIDENTAL_BRACKET: return new AccidentalBracket(score);
            case FINGERING:          return new Fingering(score);
            case HBOX:              return new HBox(score);
            case VBOX:              return new VBox(score);
            case TBOX:              return new TBox(score);
            case FBOX:              return new FBox(score);
            case MEASURE:           return new Measure(score);
            case TAB_DURATION_SYMBOL: return new TabDurationSymbol(score);
            case OSSIA:               return new Ossia(score);
            case IMAGE:             return new Image(score);
            case BAGPIPE_EMBELLISHMENT: return new BagpipeEmbellishment(score);
            case AMBITUS:           return new Ambitus(score);

            case TEXTLINE_SEGMENT:    // return new TextLineSegment(score);

            case SLUR_SEGMENT:
            case STEM_SLASH:
            case LINE:
            case TIE:
            case PAGE:
            case BEAM:
            case HOOK:
            case TUPLET:
            case HAIRPIN_SEGMENT:
            case OTTAVA_SEGMENT:
            case TRILL_SEGMENT:
            case VOLTA_SEGMENT:
            case PEDAL_SEGMENT:
            case LEDGER_LINE:
            case STAFF_LINES:
            case SELECTION:
            case LASSO:
            case SHADOW_NOTE:
            case RUBBERBAND:
            case SEGMENT:
            case SYSTEM:
            case COMPOUND:
            case ELEMENT:
            case ELEMENT_LIST:
            case STAFF_LIST:
            case MEASURE_LIST:
            case LAYOUT:
            case MAXTYPE:
            case INVALID:  break;
            }
      qDebug("cannot create type %d <%s>", int(type), Element::name(type));
      return 0;
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* Element::name(ElementType type)
      {
      return elementNames[type].name;
      }

//---------------------------------------------------------
//   name2type
//---------------------------------------------------------

Element::ElementType Element::name2type(const QStringRef& s)
      {
      for (int i = 0; i < MAXTYPE; ++i) {
            if (s == elementNames[i].name)
                  return ElementType(i);
            }
qDebug("name2type: invalid type <%s>", s.toUtf8().data());
      return INVALID;
      }

//---------------------------------------------------------
//   name2Element
//---------------------------------------------------------

Element* Element::name2Element(const QStringRef& s, Score* sc)
      {
      ElementType type = Element::name2type(s);
      if (type == INVALID)
            return 0;
      return Element::create(type, sc);
      }

//---------------------------------------------------------
//   elementLessThan
//---------------------------------------------------------

bool elementLessThan(const Element* const e1, const Element* const e2)
      {
      return e1->z() <= e2->z();
      }

//---------------------------------------------------------
//   getGrip
//---------------------------------------------------------

QPointF Element::getGrip(int) const
      {
      qreal _spatium = score()->spatium();
      return QPointF(userOff().x() / _spatium, userOff().y() / _spatium);
      }

//---------------------------------------------------------
//   setGrip
//---------------------------------------------------------

void Element::setGrip(int, const QPointF& pt)
      {
      qreal _spatium = score()->spatium();
      setUserOff(QPointF(pt.x() * _spatium, pt.y() * _spatium));
      }

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void collectElements(void* data, Element* e)
      {
      QList<Element*>* el = static_cast<QList<Element*>*>(data);
      el->append(e);
      }

//---------------------------------------------------------
//   Space::operator+=
//---------------------------------------------------------

Space& Space::operator+=(const Space& s)
      {
      _lw += s._lw;
      _rw += s._rw;
      return *this;
      }

//---------------------------------------------------------
//   undoSetPlacement
//---------------------------------------------------------

void Element::undoSetPlacement(Placement v)
      {
      score()->undoChangeProperty(this, P_PLACEMENT, int(v));
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Element::getProperty(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_COLOR:     return _color;
            case P_VISIBLE:   return _visible;
            case P_SELECTED:  return _selected;
            case P_USER_OFF:  return _userOff;
            case P_PLACEMENT: return int(_placement);
            default:
                  return QVariant();
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Element::setProperty(P_ID propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case P_COLOR:
                  _color = v.value<QColor>();
                  break;
            case P_VISIBLE:
                  setVisible(v.toBool());
                  break;
            case P_SELECTED:
                  setSelected(v.toBool());
                  break;
            case P_USER_OFF:
                  _userOff = v.toPointF();
                  break;
            case P_PLACEMENT:
                  _placement = Placement(v.toInt());
                  break;
            default:
                  qFatal("Element::setProperty: unknown <%s>(%d), data <%s>",
                     propertyName(propertyId), propertyId, qPrintable(v.toString()));
                  return false;
            }
      setGenerated(false);
      score()->addRefresh(canvasBoundingRect());
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Element::propertyDefault(P_ID id) const
      {
      switch(id) {
            case P_VISIBLE:
                  return true;
            case P_COLOR:
                  return MScore::defaultColor;
            case P_PLACEMENT:
                  return int(Placement::BELOW);
            case P_SELECTED:
                  return false;
            case P_USER_OFF:
                  return QPointF();
            default:    // not all properties have a default
                  break;
            }
      return QVariant();
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void Element::undoChangeProperty(P_ID id, const QVariant& val)
      {
      score()->undoChangeProperty(this, id, val);
      }

//---------------------------------------------------------
//   undoPushProperty
//---------------------------------------------------------

void Element::undoPushProperty(P_ID id)
      {
      QVariant val = getProperty(id);
      score()->undo()->push1(new ChangeProperty(this, id, val));
      }

//---------------------------------------------------------
//   isChordRest
//---------------------------------------------------------

bool Element::isChordRest() const
      {
      return type() == REST || type() == CHORD || type() == REPEAT_MEASURE;
      }

//---------------------------------------------------------
//   isDurationElement
//---------------------------------------------------------

bool Element::isDurationElement() const
      {
      return isChordRest() || (type() == TUPLET);
      }

//---------------------------------------------------------
//   isSLine
//---------------------------------------------------------

bool Element::isSLine() const
      {
      return type() == HAIRPIN || type() == OTTAVA || type() == PEDAL
         || type() == TRILL || type() == VOLTA || type() == TEXTLINE || type() == NOTELINE;
      }

//---------------------------------------------------------
//   isText
//---------------------------------------------------------

bool Element::isText() const
      {
      return type()  == TEXT
         || type() == LYRICS
         || type() == DYNAMIC
         || type() == FINGERING
         || type() == HARMONY
         || type() == MARKER
         || type() == JUMP
         || type() == STAFF_TEXT
         || type() == REHEARSAL_MARK
         || type() == INSTRUMENT_CHANGE
         || type() == FIGURED_BASS
         || type() == TEMPO_TEXT;
      }

//---------------------------------------------------------
//   findMeasure
//---------------------------------------------------------

Element* Element::findMeasure()
      {
      if (type() == MEASURE)
            return this;
      else if (_parent)
            return _parent->findMeasure();
      else
            return 0;
      }

//---------------------------------------------------------
//   undoSetColor
//---------------------------------------------------------

void Element::undoSetColor(const QColor& c)
      {
      score()->undoChangeProperty(this, P_COLOR, c);
      }

//---------------------------------------------------------
//   positioning functions for scripts
//
//    use spatium units rather than raster units
//    are undoable
//    route pos changes to usefOff
//---------------------------------------------------------

QPointF Element::scriptPos() const
      {
      return (_pos + _userOff) / spatium();
      }

void Element::scriptSetPos(const QPointF& p)
      {
      score()->undoChangeProperty(this, P_USER_OFF, p*spatium() - ipos());
      }

QPointF Element::scriptUserOff() const
      {
      return _userOff / spatium();
      }

void Element::scriptSetUserOff(const QPointF& o)
      {
      score()->undoChangeProperty(this, P_USER_OFF, o * spatium());
      }

//void Element::draw(SymId id, QPainter* p) const { score()->scoreFont()->draw(id, p, magS()); }

//---------------------------------------------------------
//   drawSymbol
//---------------------------------------------------------

void Element::drawSymbol(SymId id, QPainter* p, const QPointF& o) const
      {
      score()->scoreFont()->draw(id, p, magS(), o);
      }

void Element::drawSymbol(SymId id, QPainter* p, const QPointF& o, int n) const
      {
      score()->scoreFont()->draw(id, p, magS(), o, n);
      }

void Element::drawSymbols(const QString& s, QPainter* p, const QPointF& o) const
      {
      score()->scoreFont()->draw(s, p, magS(), o);
      }

//---------------------------------------------------------
//   symHeight
//---------------------------------------------------------

qreal Element::symHeight(SymId id) const
      {
      return score()->scoreFont()->height(id, magS());
      }

//---------------------------------------------------------
//   symWidth
//---------------------------------------------------------

qreal Element::symWidth(SymId id) const
      {
      return score()->scoreFont()->width(id, magS());
      }
qreal Element::symWidth(const QString& s) const
      {
      return score()->scoreFont()->width(s, magS());
      }

//---------------------------------------------------------
//   symBbox
//---------------------------------------------------------

QRectF Element::symBbox(SymId id) const
      {
      return score()->scoreFont()->bbox(id, magS());
      }

QRectF Element::symBbox(const QString& s) const
      {
      return score()->scoreFont()->bbox(s, magS());
      }

//---------------------------------------------------------
//   symAttach
//---------------------------------------------------------

QPointF Element::symAttach(SymId id) const
      {
      return score()->scoreFont()->attach(id, magS());
      }

//---------------------------------------------------------
//   symCutOutNE / symCutOutNW / symCutOutSE / symCutOutNW
//---------------------------------------------------------

QPointF Element::symCutOutNE(SymId id) const
      {
      return score()->scoreFont()->cutOutNE(id, magS());
      }

QPointF Element::symCutOutNW(SymId id) const
      {
      return score()->scoreFont()->cutOutNW(id, magS());
      }

QPointF Element::symCutOutSE(SymId id) const
      {
      return score()->scoreFont()->cutOutSE(id, magS());
      }

QPointF Element::symCutOutSW(SymId id) const
      {
      return score()->scoreFont()->cutOutSW(id, magS());
      }

//---------------------------------------------------------
//   symIsValid
//---------------------------------------------------------

bool Element::symIsValid(SymId id) const
      {
      return score()->scoreFont()->isValid(id);
      }

//---------------------------------------------------------
//   toTimeSigString
//---------------------------------------------------------

QString Element::toTimeSigString(const QString& s) const
      {
      QString d;
      ScoreFont* f = score()->scoreFont();
      for (int i = 0; i < s.size(); ++i) {
            switch (s[i].toLatin1()) {
                  case '+': d += f->toString(SymId::timeSigPlusSmall); break;
                  case '0': d += f->toString(SymId::timeSig0); break;
                  case '1': d += f->toString(SymId::timeSig1); break;
                  case '2': d += f->toString(SymId::timeSig2); break;
                  case '3': d += f->toString(SymId::timeSig3); break;
                  case '4': d += f->toString(SymId::timeSig4); break;
                  case '5': d += f->toString(SymId::timeSig5); break;
                  case '6': d += f->toString(SymId::timeSig6); break;
                  case '7': d += f->toString(SymId::timeSig7); break;
                  case '8': d += f->toString(SymId::timeSig8); break;
                  case '9': d += f->toString(SymId::timeSig9); break;
                  case 'C': d += f->toString(SymId::timeSigCommon); break;
                  case 'O': d += f->toString(SymId::mensuralProlation2); break;
                  case '(': d += f->toString(SymId::timeSigParensLeftSmall); break;
                  case ')': d += f->toString(SymId::timeSigParensRightSmall); break;
                  case '\xA2': d += f->toString(SymId::timeSigCutCommon); break;    // '¢'
                  case '\xD8': d += f->toString(SymId::mensuralProlation3); break;  // 'Ø'
                  default:  d += s[i]; break;
                  }
            }
      return d;
      }

//---------------------------------------------------------
//   concertPitch
//---------------------------------------------------------

bool Element::concertPitch() const
      {
      return score()->styleB(ST_concertPitch);
      }
}

