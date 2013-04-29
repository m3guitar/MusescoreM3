//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "groups.h"
#include "durationtype.h"
#include "chordrest.h"
#include "staff.h"
#include "tuplet.h"

//---------------------------------------------------------
//   noteGroups
//---------------------------------------------------------

static const std::vector<NoteGroup> noteGroups {
      { Fraction(4,4),
            Groups( { { 4, 768}, { 8, 272}, {12, 768}, {16, 273}, {20, 768}, {24, 272}, {28, 768} })
            },
      { Fraction(2,4),
            Groups( { { 8, 273}, {0, 0} })
            },
      };

//---------------------------------------------------------
//   endBeam
//---------------------------------------------------------

BeamMode Groups::endBeam(ChordRest* cr)
      {
      Q_ASSERT(cr->staff());

      TDuration d = cr->durationType();
      if (cr->tuplet() && !cr->tuplet()->elements().isEmpty()) {
            if (cr->tuplet()->elements().front() == cr)     // end beam at tuplet
                  return BeamMode::BEGIN;
            return BeamMode::AUTO;
            }

      int type  = int(d.type()) - int(TDuration::DurationType::V_EIGHT);
      if (type < 0 || type > 3)
            return BeamMode::AUTO;

      return cr->staff()->group(cr->tick()).beamMode(cr->rtick(), d.type());
      }

//---------------------------------------------------------
//   beamMode
//---------------------------------------------------------

BeamMode Groups::beamMode(int tick, TDuration::DurationType d) const
      {
      int shift;
      switch (d) {
            case TDuration::DurationType::V_EIGHT: shift = 0; break;
            case TDuration::DurationType::V_16TH:  shift = 4; break;
            case TDuration::DurationType::V_32ND:  shift = 8; break;
            default:
                  return BeamMode::AUTO;
            }
      for (const GroupNode& e : *this) {
            if (e.pos * 60 < tick)
                  continue;
            if (e.pos * 60 > tick)
                  break;

            int action = (e.action >> shift) & 0xf;
            switch (action) {
                  case 0: return BeamMode::AUTO;
                  case 1: return BeamMode::BEGIN;
                  case 2: return BeamMode::BEGIN32;
                  case 3: return BeamMode::BEGIN64;
                  default:
                        qDebug("   Groups::beamMode: bad action %d", action);
                        return BeamMode::AUTO;
                  }
            }
      return BeamMode::AUTO;
      }

//---------------------------------------------------------
//   endings
//---------------------------------------------------------

const Groups& Groups::endings(const Fraction& f)
      {
      for (const NoteGroup& g : noteGroups) {
            if (g.timeSig.identical(f)) {
                  return g.endings;
                  }
            }
      // TODO: construct default list
      return noteGroups[0].endings;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Groups::write(Xml& xml) const
      {
      xml.stag("Groups");
      for (const GroupNode& n : *this)
            xml.tagE(QString("Node pos=\"%1\" action=\"%3\"")
               .arg(n.pos).arg(n.action));
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Groups::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Node") {
                  GroupNode n;
                  n.pos    = e.intAttribute("pos");
                  n.action = e.intAttribute("action");
                  push_back(n);
                  e.skipCurrentElement();
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   addStop
//---------------------------------------------------------

void Groups::addStop(int pos, TDuration::DurationType d, BeamMode bm)
      {
      int shift;
      switch (d) {
            case TDuration::DurationType::V_EIGHT: shift = 0; break;
            case TDuration::DurationType::V_16TH:  shift = 4; break;
            case TDuration::DurationType::V_32ND:  shift = 8; break;
            default:
                  return;
            }
      int action;
      if (bm == BeamMode::BEGIN)
            action = 1;
      else if (bm == BeamMode::BEGIN32)
            action = 2;
      else if (bm == BeamMode::BEGIN64)
            action = 3;
      else
            return;

      pos    /= 60;
      action <<= shift;

      auto i = begin();
      for (; i != end(); ++i) {
            if (i->pos == pos) {
                  i->action = (i->action & ~(0xf << shift)) | action;
                  return;
                  }
            if (i->pos > pos)
                  break;
            }
      insert(i, GroupNode({ pos, action }));
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Groups::dump(const char* m) const
      {
      printf("%s\n", m);
      for (const GroupNode& n : *this) {
            printf("  group tick %d action 0x%02x\n", n.pos * 60, n.action);
            }
      }

