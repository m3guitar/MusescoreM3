//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: menus.cpp 5651 2012-05-19 15:57:26Z lasconic $
//
//  Copyright (C) 2002-2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "libmscore/score.h"
#include "palette.h"
#include "palettebox.h"
#include "libmscore/note.h"
#include "libmscore/chordrest.h"
#include "libmscore/dynamic.h"
#include "libmscore/slur.h"
#include "libmscore/sym.h"
#include "libmscore/hairpin.h"
#include "scoreview.h"
#include "musescore.h"
#include "libmscore/select.h"
#include "libmscore/tempo.h"
#include "libmscore/segment.h"
#include "libmscore/undo.h"
#include "icons.h"
#include "libmscore/bracket.h"
#include "libmscore/ottava.h"
#include "libmscore/textline.h"
#include "libmscore/trill.h"
#include "libmscore/pedal.h"
#include "libmscore/clef.h"
#include "libmscore/timesig.h"
#include "libmscore/barline.h"
#include "libmscore/layoutbreak.h"
#include "symboldialog.h"
#include "libmscore/volta.h"
#include "libmscore/keysig.h"
#include "libmscore/breath.h"
#include "libmscore/arpeggio.h"
#include "libmscore/tremolo.h"
#include "libmscore/repeat.h"
#include "libmscore/tempotext.h"
#include "libmscore/glissando.h"
#include "libmscore/articulation.h"
#include "libmscore/chord.h"
#include "libmscore/drumset.h"
#include "libmscore/spacer.h"
#include "libmscore/measure.h"
#include "libmscore/fret.h"
#include "libmscore/staffstate.h"
#include "libmscore/fingering.h"
#include "libmscore/bend.h"
#include "libmscore/tremolobar.h"
#include "libmscore/chordline.h"
#include "libmscore/stafftext.h"
#include "libmscore/instrchange.h"
#include "workspace.h"
#include "libmscore/icon.h"
#include "libmscore/accidental.h"
#include "libmscore/harmony.h"
#include "libmscore/rehearsalmark.h"
#include "shortcut.h"
#include "libmscore/marker.h"
#include "libmscore/jump.h"
#include "libmscore/bagpembell.h"
#include "libmscore/ambitus.h"

namespace Ms {

extern bool useFactorySettings;

//---------------------------------------------------------
//   populateIconPalette
//---------------------------------------------------------

void populateIconPalette(Palette* p, const IconAction* a)
      {
      while (a->subtype != -1) {
            Icon* ik = new Icon(gscore);
            ik->setIconType(a->subtype);
            Shortcut* s = Shortcut::getShortcut(a->action);
            QAction* action = s->action();
            QIcon icon(action->icon());
            ik->setAction(a->action, icon);
            p->append(ik, s->help());
            ++a;
            }
      }

//---------------------------------------------------------
//   newBeamPalette
//---------------------------------------------------------

Palette* MuseScore::newBeamPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Beam Properties"));
      sp->setGrid(27, 40);
      sp->setDrawGrid(true);

      static const IconAction bpa[] = {
            { ICON_SBEAM,    "beam-start" },
            { ICON_MBEAM,    "beam-mid" },
            { ICON_NBEAM,    "no-beam" },
            { ICON_BEAM32,   "beam32" },
            { ICON_BEAM64,   "beam64" },
            { ICON_AUTOBEAM, "auto-beam" },
            { ICON_FBEAM1,   "fbeam1" },
            { ICON_FBEAM2,   "fbeam2" },
            { -1, ""}
            };

      populateIconPalette(sp, bpa);
      return sp;
      }

//---------------------------------------------------------
//   newFramePalette
//---------------------------------------------------------

Palette* MuseScore::newFramePalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Frames && Measures"));
      sp->setGrid(27, 40);
      sp->setDrawGrid(true);

      if(enableExperimental) {
            static const IconAction bpa[] = {
                  { ICON_VFRAME,   "insert-vbox" },
                  { ICON_HFRAME,   "insert-hbox" },
                  { ICON_TFRAME,   "insert-textframe" },
                  { ICON_FFRAME,   "insert-fretframe" },
                  { ICON_MEASURE,  "insert-measure" },
                  { -1, ""}
                  };
            populateIconPalette(sp, bpa);
            }
      else {
            static const IconAction bpa[] = {
                  { ICON_VFRAME,   "insert-vbox" },
                  { ICON_HFRAME,   "insert-hbox" },
                  { ICON_TFRAME,   "insert-textframe" },
                  { ICON_MEASURE,  "insert-measure" },
                  { -1, ""}
                };
            populateIconPalette(sp, bpa);
            }

      return sp;
      }

//---------------------------------------------------------
//   newDynamicsPalette
//---------------------------------------------------------

Palette* MuseScore::newDynamicsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Dynamics"));
      sp->setMag(.8);
      sp->setGrid(42, 28);
      sp->setDrawGrid(true);

      const char* array[] = {
            "ppp", "pp", "p", "mp", "mf", "f", "ff", "fff",
            "fp", "sf", "sfz", "sff", "sffz", "sfp", "sfpp",
            "rfz", "rf", "fz", "m", "r", "s", "z", "n"
            };
      for (const char* c : array) {
            Dynamic* dynamic = new Dynamic(gscore);
            dynamic->setDynamicType(c);
            sp->append(dynamic, dynamic->dynamicTypeName());
            }
      return sp;
      }

//---------------------------------------------------------
//   newKeySigPalette
//---------------------------------------------------------

Palette* MuseScore::newKeySigPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Key Signatures"));
      sp->setMag(1.0);
      sp->setGrid(56, 45);
      sp->setYOffset(1.0);

      for (int i = 0; i < 7; ++i) {
            KeySig* k = new KeySig(gscore);
            k->setKeySigEvent(KeySigEvent(i+1));
            sp->append(k, keyNames[i*2]);
            }
      for (int i = -7; i < 0; ++i) {
            KeySig* k = new KeySig(gscore);
            k->setKeySigEvent(KeySigEvent(i));
            sp->append(k, keyNames[(7 + i) * 2 + 1]);
            }
      KeySig* k = new KeySig(gscore);
      k->setKeySigEvent(KeySigEvent(0));
      sp->append(k, keyNames[14]);
      return sp;
      }

//---------------------------------------------------------
//   newAccidentalsPalette
//---------------------------------------------------------

Palette* MuseScore::newAccidentalsPalette(bool basic)
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Accidentals"));
      sp->setGrid(33, 36);
      sp->setDrawGrid(true);

      if (basic) {
            static Accidental::AccidentalType types[] = {
                  Accidental::AccidentalType::NONE,
                  Accidental::AccidentalType::SHARP,
                  Accidental::AccidentalType::FLAT,
                  Accidental::AccidentalType::SHARP2,
                  Accidental::AccidentalType::FLAT2,
                  Accidental::AccidentalType::NATURAL
                  };
            for (auto i : types) {
                  Accidental* s = new Accidental(gscore);
                  s->setAccidentalType(Accidental::AccidentalType(i));
                  sp->append(s, s->subtypeUserName());
                  }
            }
      else {
            for (int i = int(Accidental::AccidentalType::SHARP); i < int(Accidental::AccidentalType::END); ++i) {
                  Accidental* s = new Accidental(gscore);
                  s->setAccidentalType(Accidental::AccidentalType(i));
                  if (s->symbol() != SymId::noSym)
                        sp->append(s, s->subtypeUserName());
                  else
                        delete s;
                  }
            }
      AccidentalBracket* ab = new AccidentalBracket(gscore);
      sp->append(ab, QT_TRANSLATE_NOOP("Palette", "round brackets"));
      return sp;
      }

//---------------------------------------------------------
//   newBarLinePalette
//---------------------------------------------------------

Palette* MuseScore::newBarLinePalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Barlines"));
      sp->setMag(0.8);
      sp->setGrid(42, 38);

      // bar line styles
      struct {
            BarLineType type;
            const char* name;
            } t[] = {
            { NORMAL_BAR,       QT_TRANSLATE_NOOP("Palette", "Normal") },
            { BROKEN_BAR,       QT_TRANSLATE_NOOP("Palette", "Dashed") },
            { DOTTED_BAR,       QT_TRANSLATE_NOOP("Palette", "Dotted") },
            { END_BAR,          QT_TRANSLATE_NOOP("Palette", "End Bar") },
            { DOUBLE_BAR,       QT_TRANSLATE_NOOP("Palette", "Double Bar") },
            { START_REPEAT,     QT_TRANSLATE_NOOP("Palette", "Start Repeat") },
            { END_REPEAT,       QT_TRANSLATE_NOOP("Palette", "End Repeat") },
            { END_START_REPEAT, QT_TRANSLATE_NOOP("Palette", "End-Start Repeat") },
            };
      for (unsigned i = 0; i < sizeof(t)/sizeof(*t); ++i) {
            BarLine* b  = new BarLine(gscore);
            b->setBarLineType(t[i].type);
            sp->append(b, t[i].name);
            }

      // bar line spans
      struct {
            int         from, to;
            const char* name;
            } span[] = {
            { -1, 1, QT_TRANSLATE_NOOP("Palette", "Tick 1") },
            { -2, 2, QT_TRANSLATE_NOOP("Palette", "Tick 2") },
            { 2,  6, QT_TRANSLATE_NOOP("Palette", "Short 1") },
            { 1,  7, QT_TRANSLATE_NOOP("Palette", "Short 2") },
            };
      for (unsigned i = 0; i < sizeof(span)/sizeof(*span); ++i) {
            BarLine* b  = new BarLine(gscore);
            b->setBarLineType(NORMAL_BAR);
            b->setSpanFrom(span[i].from);
            b->setSpanTo(span[i].to);
            sp->append(b, span[i].name);
            }
      return sp;
      }

//---------------------------------------------------------
//   newRepeatsPalette
//---------------------------------------------------------

Palette* MuseScore::newRepeatsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Repeats"));
      sp->setMag(0.65);
      sp->setGrid(84, 28);
      sp->setDrawGrid(true);

      RepeatMeasure* rm = new RepeatMeasure(gscore);
      sp->append(rm, tr("Repeat measure sign"));

      Marker* mk = new Marker(gscore);
      mk->setMarkerType(MarkerType::SEGNO);
      sp->append(mk, tr("Segno"));

      mk = new Marker(gscore);
      mk->setMarkerType(MarkerType::VARSEGNO);
      sp->append(mk, tr("Segno Variation"));

      mk = new Marker(gscore);
      mk->setMarkerType(MarkerType::CODA);
      sp->append(mk, tr("Coda"));

      mk = new Marker(gscore);
      mk->setMarkerType(MarkerType::VARCODA);
      sp->append(mk, tr("Varied coda"));

/*      mk = new Marker(gscore);                      // not in smufl
      mk->setMarkerType(MarkerType::CODETTA);
      sp->append(mk, tr("Codetta"));
  */
      mk = new Marker(gscore);
      mk->setMarkerType(MarkerType::FINE);
      sp->append(mk, tr("Fine"));

      Jump* jp = new Jump(gscore);
      jp->setJumpType(JumpType::DC);
      sp->append(jp, tr("Da Capo"));

      jp = new Jump(gscore);
      jp->setJumpType(JumpType::DC_AL_FINE);
      sp->append(jp, tr("Da Capo al Fine"));

      jp = new Jump(gscore);
      jp->setJumpType(JumpType::DC_AL_CODA);
      sp->append(jp, tr("Da Capo al Coda"));

      jp = new Jump(gscore);
      jp->setJumpType(JumpType::DS_AL_CODA);
      sp->append(jp, tr("D.S. al Coda"));

      jp = new Jump(gscore);
      jp->setJumpType(JumpType::DS_AL_FINE);
      sp->append(jp, tr("D.S. al Fine"));

      jp = new Jump(gscore);
      jp->setJumpType(JumpType::DS);
      sp->append(jp, tr("D.S."));

      mk = new Marker(gscore);
      mk->setMarkerType(MarkerType::TOCODA);
      sp->append(mk, tr("To Coda"));
      return sp;
      }

//---------------------------------------------------------
//   newBreaksPalette
//---------------------------------------------------------

Palette* MuseScore::newBreaksPalette()
      {
      qreal _spatium = gscore->spatium();
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Breaks && Spacers"));
      sp->setMag(1.0);
      sp->setGrid(42, 36);
      sp->setDrawGrid(true);

      LayoutBreak* lb = new LayoutBreak(gscore);
      lb->setLayoutBreakType(LayoutBreak::LayoutBreakType::LINE);
      PaletteCell* cell = sp->append(lb, tr("Line break"));
      cell->mag = 1.2;

      lb = new LayoutBreak(gscore);
      lb->setLayoutBreakType(LayoutBreak::LayoutBreakType::PAGE);
      cell = sp->append(lb, tr("Page break"));
      cell->mag = 1.2;

      lb = new LayoutBreak(gscore);
      lb->setLayoutBreakType(LayoutBreak::LayoutBreakType::SECTION);
      cell = sp->append(lb, tr("Section break"));
      cell->mag = 1.2;

      Spacer* spacer = new Spacer(gscore);
      spacer->setSpacerType(SPACER_DOWN);
      spacer->setGap(3 * _spatium);
      cell = sp->append(spacer, tr("Staff spacer down"));
      cell->mag = .7;

      spacer = new Spacer(gscore);
      spacer->setSpacerType(SPACER_UP);
      spacer->setGap(3 * _spatium);
      cell = sp->append(spacer, tr("Staff spacer up"));
      cell->mag = .7;
      return sp;
      }

//---------------------------------------------------------
//   newFingeringPalette
//---------------------------------------------------------

Palette* MuseScore::newFingeringPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Fingering"));
      sp->setMag(1.5);
      sp->setGrid(28, 30);
      sp->setDrawGrid(true);

      const char finger[] = "012345pimac";
      for (unsigned i = 0; i < strlen(finger); ++i) {
            Fingering* f = new Fingering(gscore);
            f->setText(QString(finger[i]));
            sp->append(f, tr("Fingering %1").arg(finger[i]));
            }
      const char stringnumber[] = "0123456";
      for (unsigned i = 0; i < strlen(stringnumber); ++i) {
            Fingering* f = new Fingering(gscore);
            f->setTextStyleType(TEXT_STYLE_STRING_NUMBER);
            f->setText(QString(stringnumber[i]));
            sp->append(f, tr("String number %1").arg(stringnumber[i]));
            }
      Symbol* symbol = new Symbol(gscore);
      symbol->setSym(SymId::stringsThumbPosition);
      sp->append(symbol, tr("Thumb"));
      return sp;
      }

//---------------------------------------------------------
//   newTremoloPalette
//---------------------------------------------------------

Palette* MuseScore::newTremoloPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Tremolo"));
      sp->setGrid(27, 40);
      sp->setDrawGrid(true);
      const char* tremoloName[] = {
            QT_TR_NOOP("Eighth through stem"),
            QT_TR_NOOP("16th through stem"),
            QT_TR_NOOP("32nd through stem"),
            QT_TR_NOOP("64th through stem"),
            QT_TR_NOOP("Eighth between notes"),
            QT_TR_NOOP("16th between notes"),
            QT_TR_NOOP("32nd between notes"),
            QT_TR_NOOP("64th between notes")
            };

      for (int i = int(TremoloType::R8); i <= int(TremoloType::C64); ++i) {
            Tremolo* tremolo = new Tremolo(gscore);
            tremolo->setTremoloType(TremoloType(i));
            sp->append(tremolo, tr(tremoloName[i - int(TremoloType::R8)]));
            }
      return sp;
      }

//---------------------------------------------------------
//   newNoteHeadsPalette
//---------------------------------------------------------

Palette* MuseScore::newNoteHeadsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Note Heads"));
      sp->setMag(1.3);
      sp->setGrid(33, 36);
      sp->setDrawGrid(true);

      for (int i = 0; i < int(NoteHeadGroup::HEAD_GROUPS); ++i) {
            SymId sym = Note::noteHead(0, NoteHeadGroup(i), NoteHeadType::HEAD_HALF);
            // HEAD_BREVIS_ALT shows up only for brevis value
            if (i == int(NoteHeadGroup::HEAD_BREVIS_ALT) )
                  sym = Note::noteHead(0, NoteHeadGroup(i), NoteHeadType::HEAD_BREVIS);
            NoteHead* nh = new NoteHead(gscore);
            nh->setSym(sym);
            sp->append(nh, Sym::id2userName(sym));
            }
      Icon* ik = new Icon(gscore);
      ik->setIconType(ICON_BRACKETS);
      Shortcut* s = Shortcut::getShortcut("add-brackets");
      QAction* action = s->action();
      QIcon icon(action->icon());
      ik->setAction("add-brackets", icon);
      sp->append(ik, s->help());
      return sp;
      }

//---------------------------------------------------------
//   newArticulationsPalette
//---------------------------------------------------------

Palette* MuseScore::newArticulationsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Articulations && Ornaments"));
      sp->setGrid(42, 25);
      sp->setDrawGrid(true);

      for (int i = 0; i < ARTICULATIONS; ++i) {
            Articulation* s = new Articulation(gscore);
            s->setArticulationType(ArticulationType(i));
            sp->append(s, qApp->translate("articulation", qPrintable(s->subtypeUserName())));
            }
      Bend* bend = new Bend(gscore);
      bend->points().append(PitchValue(0,    0, false));
      bend->points().append(PitchValue(15, 100, false));
      bend->points().append(PitchValue(60, 100, false));
      sp->append(bend, qApp->translate("articulation", "Bend"));

      TremoloBar* tb = new TremoloBar(gscore);
      tb->points().append(PitchValue(0,     0, false));     // "Dip"
      tb->points().append(PitchValue(30, -100, false));
      tb->points().append(PitchValue(60,    0, false));
      sp->append(tb, qApp->translate("articulation", "Tremolo Bar"));
      return sp;
      }

//---------------------------------------------------------
//   newBracketsPalette
//---------------------------------------------------------

Palette* MuseScore::newBracketsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Brackets"));
      sp->setMag(0.7);
      sp->setGrid(42, 60);
      sp->setDrawGrid(true);

      Bracket* b1 = new Bracket(gscore);
      b1->setBracketType(BRACKET_NORMAL);
      Bracket* b2 = new Bracket(gscore);
      b2->setBracketType(BRACKET_BRACE);
      Bracket* b3 = new Bracket(gscore);
      b3->setBracketType(BRACKET_SQUARE);
      Bracket* b4 = new Bracket(gscore);
      b4->setBracketType(BRACKET_LINE);

      sp->append(b1, tr("Bracket"));
      sp->append(b2, tr("Brace"));
      sp->append(b3, tr("Square"));
      sp->append(b4, tr("Line"));

      return sp;
      }

//---------------------------------------------------------
//   newBreathPalette
//---------------------------------------------------------

Palette* MuseScore::newBreathPalette()
      {
      Palette* sp = new Palette();
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Breaths && Pauses"));
      sp->setGrid(42, 40);
      sp->setDrawGrid(true);
      sp->setDrawGrid(true);

      for (int i = 0; i < 4; ++i) {
            Breath* a = new Breath(gscore);
            a->setBreathType(i);
            if (i < 2)
                  sp->append(a, tr("Breath"));
            else
                  sp->append(a, tr("Caesura"));
            }
      return sp;
      }

//---------------------------------------------------------
//   newArpeggioPalette
//---------------------------------------------------------

Palette* MuseScore::newArpeggioPalette()
      {
      Palette* sp = new Palette();
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Arpeggios && Glissandi"));
      sp->setGrid(27, 60);
      sp->setDrawGrid(true);

      for (int i = 0; i < 6; ++i) {
            Arpeggio* a = new Arpeggio(gscore);
            a->setArpeggioType(ArpeggioType(i));
            sp->append(a, tr("Arpeggio"));
            }
      for (int i = 0; i < 2; ++i) {
            Glissando* a = new Glissando(gscore);
            a->setGlissandoType(GlissandoType(i));
            sp->append(a, tr("Glissando"));
            }

      //fall and doits
      const char* scorelineNames[] = {
            QT_TR_NOOP("fall"),
            QT_TR_NOOP("doit"),
            QT_TR_NOOP("plop"),
            QT_TR_NOOP("scoop"),
            };

      ChordLine* cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::FALL);
      sp->append(cl, tr(scorelineNames[0]));

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::DOIT);
      sp->append(cl, tr(scorelineNames[1]));

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::PLOP);
      sp->append(cl, tr(scorelineNames[2]));

      cl = new ChordLine(gscore);
      cl->setChordLineType(ChordLineType::SCOOP);
      sp->append(cl, tr(scorelineNames[3]));

      return sp;
      }

//---------------------------------------------------------
//   newClefsPalette
//---------------------------------------------------------

Palette* MuseScore::newClefsPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Clefs"));
      sp->setMag(0.8);
      sp->setGrid(33, 60);
      sp->setYOffset(1.0);
      // Up to ClefType::MAX-1, because ClefType::PERC2 is no longer supported
      static const ClefType clefs[int(ClefType::MAX)-1] = {
            ClefType::G, ClefType::G1, ClefType::G2, ClefType::G3, ClefType::G4,
            ClefType::C1, ClefType::C2, ClefType::C3, ClefType::C4, ClefType::C5,
            ClefType::F, ClefType::F_8VA, ClefType::F_15MA, ClefType::F8, ClefType::F15, ClefType::F_B, ClefType::F_C,
            ClefType::PERC, ClefType::TAB, ClefType::TAB2
            };
      for (int i = 0; i < int(ClefType::MAX)-1; ++i) {
            ClefType j = clefs[i];
            Clef* k = new Ms::Clef(gscore);
            k->setClefType(ClefTypeList(j, j));
            sp->append(k, qApp->translate("clefTable", ClefInfo::name(j)));
            }
      return sp;
      }

//---------------------------------------------------------
//   newGraceNotePalette
//---------------------------------------------------------

Palette* MuseScore::newGraceNotePalette()
      {
      Palette* notePalette = new Palette;
      notePalette->setName(QT_TRANSLATE_NOOP("Palette", "Grace Notes"));
      notePalette->setGrid(32, 40);
      notePalette->setDrawGrid(true);

      static const IconAction gna[] = {
            { ICON_ACCIACCATURA, "acciaccatura" },
            { ICON_APPOGGIATURA, "appoggiatura" },
            { ICON_GRACE4,       "grace4" },
            { ICON_GRACE16,      "grace16" },
            { ICON_GRACE32,      "grace32" },
            { ICON_GRACE8_AFTER,  "grace8after" },
            { ICON_GRACE16_AFTER, "grace16after" },
            { ICON_GRACE32_AFTER, "grace32after" },
            { -1, "" }
            };
      populateIconPalette(notePalette, gna);
      return notePalette;
      }

//---------------------------------------------------------
//   newBagpipeEmbellishmentPalette
//---------------------------------------------------------

Palette* MuseScore::newBagpipeEmbellishmentPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Bagpipe Embellishments"));
      sp->setMag(0.8);
      sp->setGrid(60, 80);

      for (int i = 0; i < BagpipeEmbellishment::nEmbellishments(); ++i) {
            BagpipeEmbellishment* b  = new BagpipeEmbellishment(gscore);
            b->setEmbelType(i);
            sp->append(b, BagpipeEmbellishment::BagpipeEmbellishmentList[i].name);
            }

      return sp;
      }

//---------------------------------------------------------
//   newLinesPalette
//---------------------------------------------------------

Palette* MuseScore::newLinesPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Lines"));
      sp->setMag(.8);
      sp->setGrid(82, 35);
      sp->setDrawGrid(true);

      qreal w = gscore->spatium() * 8;

      Slur* slur = new Slur(gscore);
      slur->setId(0);
      sp->append(slur, qApp->translate("lines", "Slur"));

      Hairpin* gabel0 = new Hairpin(gscore);
      gabel0->setHairpinType(Hairpin::HairpinType::CRESCENDO);
      gabel0->setLen(w);
      sp->append(gabel0, qApp->translate("lines", "Crescendo"));

      Hairpin* gabel1 = new Hairpin(gscore);
      gabel1->setHairpinType(Hairpin::HairpinType::DECRESCENDO);
      gabel1->setLen(w);
      sp->append(gabel1, QT_TRANSLATE_NOOP("Palette", "Diminuendo"));

      Volta* volta = new Volta(gscore);
      volta->setVoltaType(VoltaType::CLOSED);
      volta->setLen(w);
      volta->setText("1.");
      QList<int> il;
      il.append(1);
      volta->setEndings(il);
      sp->append(volta, QT_TRANSLATE_NOOP("Palette", "Prima volta"));

      volta = new Volta(gscore);
      volta->setVoltaType(VoltaType::CLOSED);
      volta->setLen(w);
      volta->setText("2.");
      il.clear();
      il.append(2);
      volta->setEndings(il);
      sp->append(volta, QT_TRANSLATE_NOOP("Palette", "Seconda volta"));

      volta = new Volta(gscore);
      volta->setVoltaType(VoltaType::CLOSED);
      volta->setLen(w);
      volta->setText("3.");
      il.clear();
      il.append(3);
      volta->setEndings(il);
      sp->append(volta, QT_TRANSLATE_NOOP("Palette", "Terza volta"));

      volta = new Volta(gscore);
      volta->setVoltaType(VoltaType::OPEN);
      volta->setLen(w);
      volta->setText("2.");
      il.clear();
      il.append(2);
      volta->setEndings(il);
      sp->append(volta, QT_TRANSLATE_NOOP("Palette", "Seconda volta 2"));

      Ottava* ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_8VA);
      ottava->setLen(w);
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "8va"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_8VB);
      ottava->setLen(w);
      ottava->setPlacement(Element::Placement::BELOW);
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "8vb"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_15MA);
      ottava->setLen(w);
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "15ma"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_15MB);
      ottava->setLen(w);
      ottava->setPlacement(Element::Placement::BELOW);
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "15mb"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_22MA);
      ottava->setLen(w);
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "22ma"));

      ottava = new Ottava(gscore);
      ottava->setOttavaType(OttavaType::OTTAVA_22MB);
      ottava->setLen(w);
      sp->append(ottava, QT_TRANSLATE_NOOP("Palette", "22mb"));


      Pedal* pedal = new Pedal(gscore);
      pedal->setLen(w);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setEndHookType(HOOK_45);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginHook(true);
      pedal->setBeginHookType(HOOK_45);
      pedal->setEndHookType(HOOK_45);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal"));

      pedal = new Pedal(gscore);
      pedal->setLen(w);
      pedal->setBeginHook(true);
      pedal->setBeginHookType(HOOK_45);
      sp->append(pedal, QT_TRANSLATE_NOOP("Palette", "Pedal"));

      Trill* trill = new Trill(gscore);
      trill->setLen(w);
      sp->append(trill, QT_TRANSLATE_NOOP("Palette", "Trill line"));

      trill = new Trill(gscore);
      trill->setTrillType("upprall");
      trill->setLen(w);
      sp->append(trill, QT_TRANSLATE_NOOP("Palette", "Upprall line"));

      trill = new Trill(gscore);
      trill->setTrillType("downprall");
      trill->setLen(w);
      sp->append(trill, QT_TRANSLATE_NOOP("Palette", "Downprall line"));

      trill = new Trill(gscore);
      trill->setTrillType("prallprall");
      trill->setLen(w);
      sp->append(trill, QT_TRANSLATE_NOOP("Palette", "Prallprall line"));

      trill = new Trill(gscore);
      trill->setTrillType("pure");
      trill->setLen(w);
      sp->append(trill, QT_TRANSLATE_NOOP("Palette", "Wavy line"));

      TextLine* textLine = new TextLine(gscore);
      textLine->setLen(w);
      textLine->setBeginText("VII");
      textLine->setEndHook(true);
      sp->append(textLine, QT_TRANSLATE_NOOP("Palette", "Text line"));

      TextLine* line = new TextLine(gscore);
      line->setLen(w);
      line->setDiagonal(true);
      sp->append(line, QT_TRANSLATE_NOOP("Palette", "Line"));

      Ambitus* a = new Ambitus(gscore);
      sp->append(a, QT_TRANSLATE_NOOP("Palette", "Ambitus"));

      return sp;
      }

//---------------------------------------------------------
//   showPalette
//---------------------------------------------------------

void MuseScore::showPalette(bool visible)
      {
      QAction* a = getAction("toggle-palette");
      if (paletteBox == 0)
            Workspace::currentWorkspace->read();
      if (paletteBox)   // read failed?
            paletteBox->setVisible(visible);
      a->setChecked(visible);
      }

//---------------------------------------------------------
//   TempoPattern
//---------------------------------------------------------

struct TempoPattern {
      QString pattern;
      double f;

      TempoPattern(const QString& s, double v) : pattern(s), f(v) {}
      };

//---------------------------------------------------------
//   newTempoPalette
//---------------------------------------------------------

Palette* MuseScore::newTempoPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Tempo"));
      sp->setMag(0.65);
      sp->setGrid(60, 30);
      sp->setDrawGrid(true);

      static const TempoPattern tp[] = {
            TempoPattern("<sym>noteHalfUp</sym> = 80", 80.0/30.0),                        // 1/2
            TempoPattern("<sym>noteQuarterUp</sym> = 80", 80.0/60.0),                     // 1/4
            TempoPattern("<sym>note8thUp</sym> = 80", 80.0/120.0),                    // 1/8
            TempoPattern("<sym>noteHalfUp</sym><sym>textAugmentationDot</sym> = 80", 120/30.0),       // dotted 1/2
            TempoPattern("<sym>noteQuarterUp</sym><sym>textAugmentationDot</sym> = 80", 120/60.0),    // dotted 1/4
            TempoPattern("<sym>note8thUp</sym><sym>textAugmentationDot</sym> = 80", 120/120.0),   // dotted 1/8
            };
      for (unsigned i = 0; i < sizeof(tp)/sizeof(*tp); ++i) {
            TempoText* tt = new TempoText(gscore);
            tt->setFollowText(true);
            tt->setTrack(0);
            tt->setTempo(tp[i].f);
            tt->setText(tp[i].pattern);
            sp->append(tt, tr("Tempo Text"), QString(), 1.5);
            }
      return sp;
      }

//---------------------------------------------------------
//   newTextPalette
//---------------------------------------------------------

Palette* MuseScore::newTextPalette()
      {
      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Text"));
      sp->setMag(0.65);
      sp->setGrid(84, 28);
      sp->setDrawGrid(true);

      StaffText* st = new StaffText(gscore);
      st->setTextStyleType(TEXT_STYLE_STAFF);
      st->setText(tr("staff-text"));
      sp->append(st, tr("Staff Text"));

      st = new StaffText(gscore);
      st->setTextStyleType(TEXT_STYLE_SYSTEM);
      st->setText(tr("system-text"));
      sp->append(st, tr("System Text"));

      RehearsalMark* rhm = new RehearsalMark(gscore);
      rhm->setTrack(0);
      rhm->setText("B1");
      sp->append(rhm, tr("Rehearsal Mark"));

      InstrumentChange* is = new InstrumentChange(gscore);
      is->setText(tr("Instrument"));
      sp->append(is, tr("Instrument Change"));

      Text* text = new Text(gscore);
      text->setTrack(0);
      text->setTextStyleType(TEXT_STYLE_LYRICS_VERSE_NUMBER);
      text->setText(tr("1."));
      sp->append(text, tr("Lyrics Verse Number"));

      Harmony* harmony = new Harmony(gscore);
      harmony->setText("C7");
      sp->append(harmony, tr("Chord Symbol"));
      return sp;
      }

//---------------------------------------------------------
//   newTimePalette
//    create default time signature palette
//---------------------------------------------------------

Palette* MuseScore::newTimePalette()
      {
      struct TS {
            int numerator;
            int denominator;
            TimeSigType type;
            QString name;
            };

      TS tsList[] = {
            { 2,  4, TimeSigType::NORMAL, "2/4" },
            { 3,  4, TimeSigType::NORMAL, "3/4" },
            { 4,  4, TimeSigType::NORMAL, "4/4" },
            { 5,  4, TimeSigType::NORMAL, "5/4" },
            { 6,  4, TimeSigType::NORMAL, "6/4" },
            { 3,  8, TimeSigType::NORMAL, "3/8" },
            { 6,  8, TimeSigType::NORMAL, "6/8" },
            { 9,  8, TimeSigType::NORMAL, "9/8" },
            { 12, 8, TimeSigType::NORMAL, "12/8" },
            { 4,  4, TimeSigType::FOUR_FOUR,  tr("4/4 common time") },
            { 2,  2, TimeSigType::ALLA_BREVE, tr("2/2 alla breve") }
            };

      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Time Signatures"));
      sp->setMag(.8);
      sp->setGrid(42, 38);

      for (unsigned i = 0; i < sizeof(tsList)/sizeof(*tsList); ++i) {
            TimeSig* ts;
            ts = new TimeSig(gscore);
            ts->setSig(Fraction(tsList[i].numerator, tsList[i].denominator), tsList[i].type);
            sp->append(ts, tsList[i].name);
            }
      return sp;
      }

//---------------------------------------------------------
//   setAdvancedPalette
//---------------------------------------------------------

void MuseScore::setAdvancedPalette()
      {
      mscore->getPaletteBox();
      paletteBox->clear();
      paletteBox->addPalette(newGraceNotePalette());
      paletteBox->addPalette(newClefsPalette());
      paletteBox->addPalette(newKeySigPalette());
      paletteBox->addPalette(newTimePalette());
      paletteBox->addPalette(newBarLinePalette());
      paletteBox->addPalette(newLinesPalette());
      paletteBox->addPalette(newArpeggioPalette());
      paletteBox->addPalette(newBreathPalette());
      paletteBox->addPalette(newBracketsPalette());
      paletteBox->addPalette(newArticulationsPalette());
      paletteBox->addPalette(newAccidentalsPalette());
      paletteBox->addPalette(newDynamicsPalette());
      paletteBox->addPalette(newFingeringPalette());
      paletteBox->addPalette(newNoteHeadsPalette());
      paletteBox->addPalette(newTremoloPalette());
      paletteBox->addPalette(newRepeatsPalette());
      paletteBox->addPalette(newTempoPalette());
      paletteBox->addPalette(newTextPalette());
      paletteBox->addPalette(newBreaksPalette());
      paletteBox->addPalette(newBagpipeEmbellishmentPalette());

      //-----------------------------------
      //    staff state changes
      //-----------------------------------

#if 0
      sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Staff Changes"));
      sp->setMag(.7);
      sp->setGrid(42, 36);
      sp->setDrawGrid(true);

      StaffState* st = new StaffState(gscore);
      st->setSubtype(STAFF_STATE_VISIBLE);
      sp->append(st, tr("set visible"));

      st = new StaffState(gscore);
      st->setSubtype(STAFF_STATE_INVISIBLE);
      sp->append(st, tr("set invisible"));

      st = new StaffState(gscore);
      st->setSubtype(STAFF_STATE_TYPE);
      sp->append(st, tr("change staff type"));

      st = new StaffState(gscore);
      st->setSubtype(STAFF_STATE_INSTRUMENT);
      sp->append(st, tr("change instrument"));

      paletteBox->addPalette(sp);
#endif

      paletteBox->addPalette(newBeamPalette());
      paletteBox->addPalette(newFramePalette());

      //-----------------------------------
      //    Symbols
      //-----------------------------------

      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Symbols"));
      sp->setGrid(42, 45);
      sp->setDrawGrid(true);

/*      sp->append(SymId(accDiscantSym));
      sp->append(SymId(accDotSym));
      sp->append(SymId(accFreebaseSym));
      sp->append(SymId(accStdbaseSym));
      sp->append(SymId(accBayanbaseSym));
      sp->append(SymId(accOldEESym));
      sp->append(SymId(accpushSym));
      sp->append(SymId(accpullSym));
*/

      FretDiagram* fret = new FretDiagram(gscore);
      fret->setDot(5, 1);
      fret->setDot(2, 2);
      fret->setDot(1, 3);
      fret->setMarker(0, 'X');
      fret->setMarker(3, 'O');
      fret->setMarker(4, 'O');
      sp->append(fret, tr("Fretboard Diagram"));

      paletteBox->addPalette(sp);
      }

//---------------------------------------------------------
//   setBasicPalette
//---------------------------------------------------------

void MuseScore::setBasicPalette()
      {
      mscore->getPaletteBox();
      paletteBox->clear();
      paletteBox->addPalette(newGraceNotePalette());
      paletteBox->addPalette(newClefsPalette());
      paletteBox->addPalette(newKeySigPalette());
      paletteBox->addPalette(newTimePalette());
      paletteBox->addPalette(newBarLinePalette());
      paletteBox->addPalette(newLinesPalette());
      paletteBox->addPalette(newArpeggioPalette());
      paletteBox->addPalette(newBreathPalette());
      paletteBox->addPalette(newBracketsPalette());
      paletteBox->addPalette(newArticulationsPalette());
      paletteBox->addPalette(newAccidentalsPalette(true));
      paletteBox->addPalette(newDynamicsPalette());
      paletteBox->addPalette(newFingeringPalette());
      paletteBox->addPalette(newNoteHeadsPalette());
      paletteBox->addPalette(newTremoloPalette());
      paletteBox->addPalette(newRepeatsPalette());
      paletteBox->addPalette(newTempoPalette());
      paletteBox->addPalette(newTextPalette());
      paletteBox->addPalette(newBreaksPalette());
//      paletteBox->addPalette(newBagpipeEmbellishmentPalette());
      paletteBox->addPalette(newBeamPalette());
      paletteBox->addPalette(newFramePalette());

      //-----------------------------------
      //    Symbols
      //-----------------------------------

      Palette* sp = new Palette;
      sp->setName(QT_TRANSLATE_NOOP("Palette", "Symbols"));
      sp->setGrid(42, 45);
      sp->setDrawGrid(true);

      FretDiagram* fret = new FretDiagram(gscore);
      fret->setDot(5, 1);
      fret->setDot(2, 2);
      fret->setDot(1, 3);
      fret->setMarker(0, 'X');
      fret->setMarker(3, 'O');
      fret->setMarker(4, 'O');
      sp->append(fret, tr("Fretboard Diagram"));

      paletteBox->addPalette(sp);
      }

//---------------------------------------------------------
//   genCreateMenu
//---------------------------------------------------------

QMenu* MuseScore::genCreateMenu(QWidget* parent)
      {
      QMenu* popup = new QMenu(tr("&Add"), parent);
      popup->setObjectName("Add");

      popup->addAction(getAction("instruments"));

      QMenu* measures = popup->addMenu(tr("&Measures"));
      measures->addAction(getAction("insert-measure"));
      measures->addAction(getAction("insert-measures"));
      measures->addSeparator();
      measures->addAction(getAction("append-measure"));
      measures->addAction(getAction("append-measures"));

      QMenu* frames = popup->addMenu(tr("&Frames"));
      frames->addAction(getAction("insert-hbox"));
      frames->addAction(getAction("insert-vbox"));
      frames->addAction(getAction("insert-textframe"));
      if(enableExperimental)
            frames->addAction(getAction("insert-fretframe"));
      frames->addSeparator();
      frames->addAction(getAction("append-hbox"));
      frames->addAction(getAction("append-vbox"));
      frames->addAction(getAction("append-textframe"));

      QMenu* text = popup->addMenu(tr("&Text"));
      text->addAction(getAction("title-text"));
      text->addAction(getAction("subtitle-text"));
      text->addAction(getAction("composer-text"));
      text->addAction(getAction("poet-text"));
      text->addSeparator();
      text->addAction(getAction("system-text"));
      text->addAction(getAction("staff-text"));
      text->addAction(getAction("chord-text"));
      text->addAction(getAction("rehearsalmark-text"));
      text->addSeparator();
      text->addAction(getAction("lyrics"));
      text->addAction(getAction("figured-bass"));
      text->addAction(getAction("tempo"));

      QMenu* lines = popup->addMenu(tr("&Lines"));
      lines->addSeparator();
      lines->addAction(getAction("add-slur"));
      lines->addAction(getAction("add-hairpin"));
      lines->addAction(getAction("add-hairpin-reverse"));
      lines->addAction(getAction("add-8va"));
      lines->addAction(getAction("add-8vb"));
      lines->addAction(getAction("add-noteline"));
      return popup;
      }

//---------------------------------------------------------
//   addTempo
//---------------------------------------------------------

void MuseScore::addTempo()
      {
      ChordRest* cr = cs->getSelectedChordRest();
      if (!cr)
            return;
//      double bps = 2.0;

      SigEvent event = cs->sigmap()->timesig(cr->tick());
      Fraction f = event.nominal();
      QString text("<sym>noteQuarterUp</sym> = 80");
      switch (f.denominator()) {
            case 1:
                  text = "<sym>noteWhole</sym> = 80";
                  break;
            case 2:
                  text = "<sym>noteHalfUp</sym> = 80";
                  break;
            case 4:
                  text = "<sym>noteQuarterUp</sym> = 80";
                  break;
            case 8:
                  if(f.numerator() % 3 == 0)
                        text = "<sym>noteQuarterUp</sym><sym>textAugmentationDot</sym> = 80";
                  else
                        text = "<sym>note8thUp</sym> = 80";
                  break;
            case 16:
                  if(f.numerator() % 3 == 0)
                        text = text = "<sym>note8thUp</sym><sym>textAugmentationDot</sym> = 80";
                  else
                        text = "<sym>note16thUp</sym> = 80";
                  break;
            case 32:
                  if(f.numerator() % 3 == 0)
                        text = "<sym>note16thUp</sym><sym>textAugmentationDot</sym> = 80";
                  else
                        text = "<sym>note32thUp</sym> = 80";
                  break;
            case 64:
                  if(f.numerator() % 3 == 0)
                        text = "<sym>note32thUp</sym><sym>textAugmentationDot</sym> = 80";
                  else
                        text = "<sym>note64thUp</sym> = 80";
                  break;
            default:
                  break;
            }

      TempoText* tt = new TempoText(cs);
      tt->setParent(cr->segment());
      tt->setTrack(cr->track());
      tt->setText(text);
      tt->setFollowText(true);
      //tt->setTempo(bps);
      cs->undoAddElement(tt);
      cv->startEdit(tt);
      }
}

