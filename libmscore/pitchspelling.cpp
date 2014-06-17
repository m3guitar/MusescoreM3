//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2007-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

//  This file contains the implementation of an pitch spelling
//  algorithmus from Emilios Cambouropoulos as published in:
//  "Automatic Pitch Spelling: From Numbers to Sharps and Flats"

#include "synthesizer/event.h"
#include "note.h"
#include "key.h"
#include "pitchspelling.h"
#include "staff.h"
#include "chord.h"
#include "score.h"

namespace Ms {

//---------------------------------------------------------
//   tpcIsValid
//---------------------------------------------------------

bool tpcIsValid(int val)
      {
      return val >= Tpc::TPC_MIN && val <= Tpc::TPC_MAX;
      }

//---------------------------------------------------------
//   step2tpc
//---------------------------------------------------------

int step2tpc(int step, AccidentalVal alter)
      {
      //    TPC - tonal pitch classes
      //    "line of fifth's" LOF

      static const int spellings[] = {
      //     bb  b   -   #  ##
             0,  7, 14, 21, 28,  // C
             2,  9, 16, 23, 30,  // D
             4, 11, 18, 25, 32,  // E
            -1,  6, 13, 20, 27,  // F
             1,  8, 15, 22, 29,  // G
             3, 10, 17, 24, 31,  // A
             5, 12, 19, 26, 33,  // B
             };

      int i = step*5 + int(alter)+2;
      Q_ASSERT(i >= 0 && (i < int(sizeof(spellings)/sizeof(*spellings))));
      return spellings[i];
      }

static const int tpcByStepAndKey[int(Key::NUM_OF)][STEP_DELTA_OCTAVE] = {
// step          C             D             E             F             G             A             B        Key
      { Tpc::TPC_C_B, Tpc::TPC_D_B, Tpc::TPC_E_B, Tpc::TPC_F_B, Tpc::TPC_G_B, Tpc::TPC_A_B, Tpc::TPC_B_B}, // Cb
      { Tpc::TPC_C_B, Tpc::TPC_D_B, Tpc::TPC_E_B, Tpc::TPC_F,   Tpc::TPC_G_B, Tpc::TPC_A_B, Tpc::TPC_B_B}, // Gb
      { Tpc::TPC_C,   Tpc::TPC_D_B, Tpc::TPC_E_B, Tpc::TPC_F,   TPC_G_B,      Tpc::TPC_A_B, Tpc::TPC_B_B}, // Db
      { Tpc::TPC_C,   Tpc::TPC_D_B, Tpc::TPC_E_B, Tpc::TPC_F,   Tpc::TPC_G,   Tpc::TPC_A_B, Tpc::TPC_B_B}, // Ab
      { Tpc::TPC_C,   Tpc::TPC_D,   Tpc::TPC_E_B, Tpc::TPC_F,   Tpc::TPC_G,   Tpc::TPC_A_B, Tpc::TPC_B_B}, // Eb
      { Tpc::TPC_C,   Tpc::TPC_D,   Tpc::TPC_E_B, Tpc::TPC_F,   Tpc::TPC_G,   Tpc::TPC_A,   Tpc::TPC_B_B}, // B
      { Tpc::TPC_C,   Tpc::TPC_D,   Tpc::TPC_E,   Tpc::TPC_F,   Tpc::TPC_G,   Tpc::TPC_A,   Tpc::TPC_B_B}, // F
      { Tpc::TPC_C,   Tpc::TPC_D,   Tpc::TPC_E,   Tpc::TPC_F,   Tpc::TPC_G,   Tpc::TPC_A,   Tpc::TPC_B},   // C
      { Tpc::TPC_C,   Tpc::TPC_D,   Tpc::TPC_E,   Tpc::TPC_F_S, Tpc::TPC_G,   Tpc::TPC_A,   Tpc::TPC_B},   // G
      { Tpc::TPC_C_S, Tpc::TPC_D,   Tpc::TPC_E,   Tpc::TPC_F_S, Tpc::TPC_G,   Tpc::TPC_A,   Tpc::TPC_B},   // D
      { Tpc::TPC_C_S, Tpc::TPC_D,   Tpc::TPC_E,   Tpc::TPC_F_S, Tpc::TPC_G_S, Tpc::TPC_A,   Tpc::TPC_B},   // A
      { Tpc::TPC_C_S, Tpc::TPC_D_S, Tpc::TPC_E,   Tpc::TPC_F_S, Tpc::TPC_G_S, Tpc::TPC_A,   Tpc::TPC_B},   // E
      { Tpc::TPC_C_S, Tpc::TPC_D_S, Tpc::TPC_E,   Tpc::TPC_F_S, Tpc::TPC_G_S, Tpc::TPC_A_S, Tpc::TPC_B},   // H
      { Tpc::TPC_C_S, Tpc::TPC_D_S, Tpc::TPC_E_S, Tpc::TPC_F_S, Tpc::TPC_G_S, Tpc::TPC_A_S, Tpc::TPC_B},   // F#
      { Tpc::TPC_C_S, Tpc::TPC_D_S, Tpc::TPC_E_S, Tpc::TPC_F_S, Tpc::TPC_G_S, Tpc::TPC_A_S, Tpc::TPC_B_S}, // C#
};

int step2tpcByKey(int step, int key)
      {
      while (step < 0)            step += STEP_DELTA_OCTAVE;
      while (key < int(Key::MIN)) key  += KEY_DELTA_ENHARMONIC;
      while (key > int(Key::MAX)) key  -= KEY_DELTA_ENHARMONIC;
      return tpcByStepAndKey[key-int(Key::MIN)][step % STEP_DELTA_OCTAVE];
      }

//---------------------------------------------------------
//   tpc2step
//---------------------------------------------------------

int tpc2step(int tpc)
      {
      // 14 - C
      // 15 % 7 = 1
      //                                            f  c  g  d  a  e  b
      static const int steps[STEP_DELTA_OCTAVE] = { 3, 0, 4, 1, 5, 2, 6 };
      // TODO: optimize -TCP_MIN
      return steps[(tpc-Tpc::TPC_MIN) % STEP_DELTA_OCTAVE];
// without a table, could also be rendered as:
//      return ((tpc-Tpc::TPC_MIN) * STEP_DELTA_TPC) / STEP_DELTA_OCTAVE + TPC_FIRST_STEP;
      }

//---------------------------------------------------------
//   tpc2stepByKey
//---------------------------------------------------------

int tpc2stepByKey(int tpc, int key, int* pAlter)
      {
      if(pAlter)
            *pAlter = tpc2alterByKey(tpc, key);
      return tpc2step(tpc);
      }

//---------------------------------------------------------
//   step2tpc
//---------------------------------------------------------

int step2tpc(const QString& stepName, AccidentalVal alter)
      {
      if (stepName.isEmpty())
            return Tpc::TPC_INVALID;
      int r;
      switch (stepName[0].toLower().toLatin1()) {
            case 'c': r = 0; break;
            case 'd': r = 1; break;
            case 'e': r = 2; break;
            case 'f': r = 3; break;
            case 'g': r = 4; break;
            case 'a': r = 5; break;
            case 'b': r = 6; break;
            default:
                  return Tpc::TPC_INVALID;
            }
      return step2tpc(r, alter);
      }

//---------------------------------------------------------
//   step2deltaPitchByKey
//
// returns the delta pitch above natural C for the given step in the given key
// step: 0 - 6
// key: -7 - +7
//---------------------------------------------------------

static const int pitchByStepAndKey[int(Key::NUM_OF)][STEP_DELTA_OCTAVE] = {
// step  C   D   E   F   G   A   B           Key
      { -1,  1,  3,  4,  6,  8, 10},      // Cb
      { -1,  1,  3,  5,  6,  8, 10},      // Gb
      {  0,  1,  3,  5,  6,  8, 10},      // Db
      {  0,  1,  3,  5,  7,  8, 10},      // Ab
      {  0,  2,  3,  5,  7,  8, 10},      // Eb
      {  0,  2,  3,  5,  7,  9, 10},      // B
      {  0,  2,  4,  5,  7,  9, 10},      // F
      {  0,  2,  4,  5,  7,  9, 11},      // C
      {  0,  2,  4,  6,  7,  9, 11},      // G
      {  1,  2,  4,  6,  7,  9, 11},      // D
      {  1,  2,  4,  6,  8,  9, 11},      // A
      {  1,  3,  4,  6,  8,  9, 11},      // E
      {  1,  3,  4,  6,  8, 10, 11},      // H
      {  1,  3,  5,  6,  8, 10, 11},      // F#
      {  1,  3,  5,  6,  8, 10, 12},      // C#
};

int step2deltaPitchByKey(int step, int key)
      {
      while (step < 0)
            step+= STEP_DELTA_OCTAVE;
      while (key < int(Key::MIN))
            key += KEY_DELTA_ENHARMONIC;
      while (key > int(Key::MAX))
            key -= KEY_DELTA_ENHARMONIC;
      return pitchByStepAndKey[key-int(Key::MIN)][step % STEP_DELTA_OCTAVE];
      }

//---------------------------------------------------------
//   tpc2pitch
//---------------------------------------------------------

int tpc2pitch(int tpc)
      {
      Q_ASSERT(tpcIsValid(tpc));

      static int pitches[] = {
//step:     F   C   G   D   A   E   B
            3, -2,  5,  0,  7,  2,  9,     // bb
            4, -1,  6,  1,  8,  3, 10,     // b
            5,  0,  7,  2,  9,  4, 11,     // -
            6,  1,  8,  3, 10,  5, 12,     // #
            7,  2,  9,  4, 11,  6, 13      // ##
            };
      return pitches[tpc + 1];
      }

//---------------------------------------------------------
//   tpc2alterByKey
//
// returns the alteration (-3 to 3) of a given tpc in the given key
// to understand the formula:
//    in the highest key (C#Maj), each of the first 7 tpcs (Fbb to Bbb; tpc-Tpc::TPC_MIN: 0 to 7)
//          is 3 semitones below its key degree (alter = -3)
//    the second 7 tpcs (Fb to Bb; tpc-Tpc::TPC_MIN: 8 to 13) are 2 semitones below (alter = -2) and so on up to 1
//    thus, for C#Maj:
// (1)      (tpc-Tpc::TPC_MIN) - 0         =  0 to 34 (for tcp-TCP_MIN from 0 to 34)
// (2)      (tpc-Tpc::TPC_MIN) - 0) / 7    =  0 to 4  (for each settuple of tcp's) and finally
// (3)      (tcp-Tpc::TPC_MIN) - 0) / 7 -3 = -3 to 1  (for each settuple of tcp's)
//          where 0 = Key::C_S - Key::MAX
//    for each previous key, the result of (1) increases by 1 and the classes of alter are shifted 1 TPC 'up':
//          F#Maj: Fbb-Ebb => -3, Bbb to Eb => -2 and so on
//          BMaj:  Fbb-Abb => -3, Ebb to Ab => -2 and so on
//          and so on
//    thus, for any 'key', the formula is:
//          ((tcp-Tpc::TPC_MIN) - (key-Key::MAX)) / TCP_DELTA_SEMITONE - 3
//---------------------------------------------------------

int tpc2alterByKey(int tpc, int key) {
      return (tpc - key - int(Tpc::TPC_MIN) + int(Key::MAX)) / TPC_DELTA_SEMITONE - 3;
      }

//---------------------------------------------------------
//   tpc2name
//    return note name
//---------------------------------------------------------

QString tpc2name(int tpc, NoteSpellingType spelling, bool lowerCase)
      {
      QString s;
      QString acc;
      tpc2name(tpc, spelling, lowerCase, s, acc);
      return s + acc;
      }

//---------------------------------------------------------
//   tpc2name
//---------------------------------------------------------

void tpc2name(int tpc, NoteSpellingType spelling, bool lowerCase, QString& s, QString& acc)
      {
      int n;
      tpc2name(tpc, spelling, lowerCase, s, n);
      switch (n) {
            case -2: acc = "bb" ; break;
            case -1:
                  if (spelling != NoteSpellingType::GERMAN)
                        acc = "b";
                  else
                        // render flats as "es" except for A and E, which get "s"
                        acc = (tpc == 10 || tpc == 11) ? "s" : "es";
                  break;
            case  0: acc = ""; break;
            case  1: acc = (spelling != NoteSpellingType::GERMAN) ? "#" : "is"; break;
            case  2: acc = "##"; break;
            default:
                  qDebug("tpc2name(%d): acc %d", tpc, n);
                  acc = "";
                  break;
            }
      }

//---------------------------------------------------------
//   tpc2name
//---------------------------------------------------------

void tpc2name(int tpc, NoteSpellingType spelling, bool lowerCase, QString& s, int& acc)
      {
      const char names[]  = "FCGDAEB";
      const char gnames[] = "FCGDAEH";
      const QString inames[] = { "Fa", "Do", "Sol", "Re", "La", "Mi", "Si" };

      acc = ((tpc+1) / 7) - 2;
      int idx = (tpc + 1) % 7;
      switch (spelling) {
            case NoteSpellingType::GERMAN:
                  s = gnames[idx];
                  if (s == "H" && acc == -1) {
                        s = "B";
                        acc = 0;
                        }
                  break;
            case NoteSpellingType::SOLFEGGIO:   s = inames[idx]; break;
            default:          s = names[idx]; break;
            }
      if (lowerCase)
            s = s.toLower();
      }

//---------------------------------------------------------
//   tpc2stepName
//---------------------------------------------------------

QString tpc2stepName(int tpc)
      {
      const char names[] = "FCGDAEB";
      return QString(names[(tpc + 1) % 7]);
      }

// table of alternative spellings for one octave
// each entry is the TPC of the note
//    tab1 does not contain double sharps
//    tab2 does not contain double flats

static const int tab1[24] = {
      14,  2,  // 60  C   Dbb
      21,  9,  // 61  C#  Db
      16,  4,  // 62  D   Ebb
      23, 11,  // 63  D#  Eb
      18,  6,  // 64  E   Fb
      13,  1,  // 65  F   Gbb
      20,  8,  // 66  F#  Gb
      15,  3,  // 67  G   Abb
      22, 10,  // 68  G#  Ab
      17,  5,  // 69  A   Bbb
      24, 12,  // 70  A#  Bb
      19,  7,  // 71  B   Cb
      };

static const int tab2[24] = {
      26, 14,  // 60  B#  C
      21,  9,  // 61  C#  Db
      28, 16,  // 62  C## D
      23, 11,  // 63  D#  Eb
      30, 18,  // 64  D## E
      25, 13,  // 65  E#  F
      20,  8,  // 66  F#  Gb
      27, 15,  // 67  F## G
      22, 10,  // 68  G#  Ab
      29, 17,  // 69  G## A
      24, 12,  // 70  A#  Bb
      31, 19,  // 71  A## B
      };

int intervalPenalty[13] = {
      0, 0, 0, 0, 0, 0, 1, 3, 1, 1, 1, 3, 3
      };

//---------------------------------------------------------
//   enharmonicSpelling
//---------------------------------------------------------

static const int enharmonicSpelling[15][34] = {
      {
//Ces f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      0, 0, 0, 0, 0, 0, 0, // b
      1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//Ges f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 0, 0, 0, 0, 0, 0, // b
      0, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//Des f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//As  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 1, 1, 1, 1, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//Es  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 1, 1, 1,
      0, 0, 1, 1, 1, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//Bb  f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 1, 1,
      1, 0, 0, 1, 1, 1, 1, // #     // (ws) penalty for f#
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//F   f  c  g  d  a  e  b           // extra penalty for a# b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 1,
      0, 0, 0, 0, 1, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//C   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b
      0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//G   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 1, 0, 0, 0, 0, // b
      1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//D   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 1, 1, 0, 0, 0, // b
      1, 1, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//A   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 1, 1, 1, 0, 0, // b
      1, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//E   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 1, 1, 1, 1, 0, // b
      1, 1, 1, 1, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      0, 0, 1, 1, 1, 1, 1  // ##
      },
      {
//H   f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 1, 1, 1, 1, 1, // b
      1, 1, 1, 1, 1, 0, 0,
      0, 0, 0, 0, 0, 1, 1, // #
      1, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//Fis f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 1, 1, 1, 1, 1, // b
      100, 1, 1, 1, 1, 1, 0,
      0, 0, 0, 0, 0, 0, 0, // #
      0, 1, 1, 1, 1, 1, 1  // ##
      },
      {
//Cis f  c  g  d  a  e  b
         1, 1, 1, 1, 1, 1, // bb
      1, 1, 0, 0, 0, 0, 0, // b  //Fis
      100, 1, 1, 1, 1, 0, 0,
      0, 0, 0, 0, 0, 0, 0, // #
      0, 0, 1, 1, 1, 1, 1  // ##
      }
      };

//---------------------------------------------------------
//   penalty
//---------------------------------------------------------

static int penalty(int lof1, int lof2, int k)
      {
      if (k < 0 || k >= 15)
            qFatal("Illegal key %d >= 15", k);
      Q_ASSERT(lof1 >= 0 && lof1 < 34);
      Q_ASSERT(lof2 >= 0 && lof2 < 34);
      int penalty  = enharmonicSpelling[k][lof1] * 4 + enharmonicSpelling[k][lof2] * 4;
      int distance = lof2 > lof1 ? lof2 - lof1 : lof1 - lof2;
      if (distance > 12)
            penalty += 3;
      else
            penalty += intervalPenalty[distance];
      return penalty;
      }

static const int WINDOW       = 9;
#if 1 // yet(?) unused
static const int WINDOW_SHIFT = 3;
static const int ASIZE        = 1024;   // 2 ** WINDOW
#endif

//---------------------------------------------------------
//   computeWindow
//---------------------------------------------------------

static int computeWindow(const QList<Event>& notes, int start, int end, int keyIdx)
      {
      int p   = 10000;
      int idx = -1;
      int pitch[10];

      Q_ASSERT((end-start) < 10 && start != end);

      int i = start;
      int k = 0;
      while (i < end)
            pitch[k++] = notes[i++].dataA() % 12;

      for (; k < 10; ++k)
            pitch[k] = pitch[k-1];

      for (int i = 0; i < 512; ++i) {
            int pa    = 0;
            int pb    = 0;
            int l     = pitch[0] * 2 + (i & 1);
            Q_ASSERT((l >= 0) && (l < int(sizeof(tab1)/sizeof(*tab1))));
            int lof1a = tab1[l];
            int lof1b = tab2[l];

            for (int k = 1; k < 10; ++k) {
                  int l = pitch[k] * 2 + ((i & (1 << k)) >> k);
                  Q_ASSERT((l >= 0) && (l < int(sizeof(tab1)/sizeof(*tab1))));
                  int lof2a = tab1[l];
                  int lof2b = tab2[l];
                  pa += penalty(lof1a, lof2a, keyIdx);
                  pb += penalty(lof1b, lof2b, keyIdx);
                  lof1a = lof2a;
                  lof1b = lof2b;
                  }
            if (pa < pb) {
                  if (pa < p) {
                        p   = pa;
                        idx = i;
                        }
                  }
            else {
                  if (pb < p) {
                        p   = pb;
                        idx = i * -1;
                        }
                  }
            }
      return idx;
      }

//---------------------------------------------------------
//   tpc
//---------------------------------------------------------

int tpc(int idx, int pitch, int opt)
      {
      const int* tab;
      if (opt < 0) {
            tab = tab2;
            opt *= -1;
            }
      else
            tab = tab1;
      int i = (pitch % 12) * 2 + ((opt & (1 << idx)) >> idx);
      Q_ASSERT(i >= 0 && i < 24);
      return tab[i];
      }

//---------------------------------------------------------
//   computeWindow
//---------------------------------------------------------

int computeWindow(const QList<Note*>& notes, int start, int end)
      {
      int p   = 10000;
      int idx = -1;
      int pitch[10];
      int key[10];

      int i = start;
      int k = 0;
      while (i < end) {
            pitch[k] = notes[i]->pitch() % 12;
            int tick = notes[i]->chord()->tick();
            key[k]   = notes[i]->staff()->key(tick) + 7;
            if (key[k] < 0 || key[k] > 14) {
                  qDebug("illegal key at tick %d: %d, window %d-%d",
                     tick, key[k] - 7, start, end);
                  return 0;
                  // abort();
                  }
            ++k;
            ++i;
            }

      for (; k < 10; ++k) {
            pitch[k] = pitch[k-1];
            key[k]   = key[k-1];
            }

      for (int i = 0; i < 512; ++i) {
            int pa    = 0;
            int pb    = 0;
            int l     = pitch[0] * 2 + (i & 1);
            Q_ASSERT(l >= 0 && l <= (int)(sizeof(tab1)/sizeof(*tab1)));
            int lof1a = tab1[l];
            int lof1b = tab2[l];

            for (int k = 1; k < 10; ++k) {
                  int l = pitch[k] * 2 + ((i & (1 << k)) >> k);
                  Q_ASSERT(l >= 0 && l <= (int)(sizeof(tab1)/sizeof(*tab1)));
                  int lof2a = tab1[l];
                  int lof2b = tab2[l];
                  pa += penalty(lof1a, lof2a, key[k]);
                  pb += penalty(lof1b, lof2b, key[k]);
                  lof1a = lof2a;
                  lof1b = lof2b;
                  }
            if (pa < pb) {
                  if (pa < p) {
                        p   = pa;
                        idx = i;
                        }
                  }
            else {
                  if (pb < p) {
                        p   = pb;
                        idx = i * -1;
                        }
                  }
            }
/*      qDebug("compute window\n   ");
      for (int i = 0; i < 10; ++i)
            qDebug("%2d ", pitch[i]);
      qDebug("\n   ");
      for (int i = 0; i < 10; ++i)
            qDebug("%2d ", key[i]);
      qDebug("\n   ");
      for (int i = 0; i < 10; ++i)
            qDebug("%2d ", tpc(i, pitch[i], idx));
*/
      return idx;
      }

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void spell(QList<Event>& notes, int key)
      {
      key += 7;

      int n = notes.size();
      if (n == 0)
            return;

      int start = 0;
      while (start < n) {
            int end = start + WINDOW;
            if (end > n)
                  end = n;
            int opt = computeWindow(notes, start, end, key);
            const int* tab;
            if (opt < 0) {
                  tab = tab2;
                  opt *= -1;
                  }
            else
                  tab = tab1;

            if (start == 0) {
                  notes[0].setTpc(tab[(notes[0].dataA() % 12) * 2 + (opt & 1)]);
                  if (n > 1)
                        notes[1].setTpc(tab[(notes[1].dataA() % 12) * 2 + ((opt & 2)>>1)]);
                  if (n > 2)
                        notes[2].setTpc(tab[(notes[2].dataA() % 12) * 2 + ((opt & 4)>>2)]);
                  }
            if ((end - start) >= 6) {
                  notes[start+3].setTpc(tab[(notes[start+3].dataA() % 12) * 2 + ((opt &  8) >> 3)]);
                  notes[start+4].setTpc(tab[(notes[start+4].dataA() % 12) * 2 + ((opt & 16) >> 4)]);
                  notes[start+5].setTpc(tab[(notes[start+5].dataA() % 12) * 2 + ((opt & 32) >> 5)]);
                  }
            if (end == n) {
                  int n = end - start;
                  int k;
                  switch(n - 6) {
                        case 3:
                              k = end - start - 3;
                              notes[end-3].setTpc(tab[(notes[end-3].dataA() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        case 2:
                              k = end - start - 2;
                              notes[end-2].setTpc(tab[(notes[end-2].dataA() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        case 1:
                              k = end - start - 1;
                              notes[end-1].setTpc(tab[(notes[end-1].dataA() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        }
                  break;
                  }
            // advance to next window
            start += 3;
            }
      }

//---------------------------------------------------------
//   spell
//---------------------------------------------------------

void Score::spellNotelist(QList<Note*>& notes)
      {
      int n = notes.size();

      int start = 0;
      while (start < n) {
            int end = start + WINDOW;
            if (end > n)
                  end = n;
            int opt = computeWindow(notes, start, end);
            const int* tab;
            if (opt < 0) {
                  tab = tab2;
                  opt *= -1;
                  }
            else
                  tab = tab1;

            if (start == 0) {
                  undoChangeTpc(notes[0], tab[(notes[0]->pitch() % 12) * 2 + (opt & 1)]);
                  if (n > 1)
                        undoChangeTpc(notes[1], tab[(notes[1]->pitch() % 12) * 2 + ((opt & 2)>>1)]);
                  if (n > 2)
                        undoChangeTpc(notes[2], tab[(notes[2]->pitch() % 12) * 2 + ((opt & 4)>>2)]);
                  }
            if ((end - start) >= 6) {
                  undoChangeTpc(notes[start+3], tab[(notes[start+3]->pitch() % 12) * 2 + ((opt &  8) >> 3)]);
                  undoChangeTpc(notes[start+4], tab[(notes[start+4]->pitch() % 12) * 2 + ((opt & 16) >> 4)]);
                  undoChangeTpc(notes[start+5], tab[(notes[start+5]->pitch() % 12) * 2 + ((opt & 32) >> 5)]);
                  }
            if (end == n) {
                  int n = end - start;
                  int k;
                  switch(n - 6) {
                        case 3:
                              k = end - start - 3;
                              undoChangeTpc(notes[end-3], tab[(notes[end-3]->pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        case 2:
                              k = end - start - 2;
                              undoChangeTpc(notes[end-2], tab[(notes[end-2]->pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        case 1:
                              k = end - start - 1;
                              undoChangeTpc(notes[end-1], tab[(notes[end-1]->pitch() % 12) * 2 + ((opt & (1<<k)) >> k)]);
                        }
                  break;
                  }
            // advance to next window
            start += 3;
            }
      }

//---------------------------------------------------------
//   pitch2tpc2
//---------------------------------------------------------

// pitch2tpc2(pitch, false) replaced by pitch2tpc(pitch, Key::C, Prefer::FLATS)
// pitch2tpc2(pitch, true) replaced by pitch2tpc(pitch, Key::C, Prefer::SHARPS)

//---------------------------------------------------------
//   pitch2tpc
//    preferred pitch spelling depending on key
//    key -7 to +7
//
// The value of prefer sets the preferred mix of flats and sharps
// for pitches that are non-diatonic in the key specified, by
// positioning the window along the tpc sequence.
//
// Scale tones are the range shown in [ ].
// A value of 8 (Prefer::FLATS) specifies 5b 2b 6b 3b 7b [4 1 5 2 6 3 7]
// A value of 11 (Prefer::NEAREST) specifies 3b 7b [4 1 5 2 6 3 7] 4# 1# 5#
// A value of 13 (Prefer::SHARPS) specifies [4 1 5 2 6 3 7] 4# 1# 5# 2# 6#
//
// Examples for Prefer::NEAREST (n indicates explicit natural):
// C major will use Eb Bb [F C G D A E B] F# C# G#.
// E major will use Gn Dn [A E B F# C# G# D#] A# E# B#.
// F# major will use An En [B F# C# G# D# A# E#] B# Fx Cx.
// Eb major will use Gb Db [Ab Eb Bb F C G D] An En Bn.
// Gb major will use Bbb Fb [Cb Gb Db Ab Eb Bb F] Cn Gn Dn.
//---------------------------------------------------------

int pitch2tpc(int pitch, int key, Prefer prefer)
      {
      return (pitch * 7 + 26 - (int(prefer) + key)) % 12 + (int(prefer) + key);
      }

//---------------------------------------------------------
//   pitch2absStepByKey
//    absolute step (C0 = 0, D0 = 1, ... C1 = 7, D2 = 8, ...) for a pitch/tpc according to key
//    if pAlter not null, returns in it the alteration with respect to the corresponding key degree (-3 to 3)
//    (for instance, an F in GMaj yields alteration -1 i.e. 1 semitone below corresp. deg. of GMaj which is F#)
//    key: between Key::MIN and Key::MAX
//---------------------------------------------------------

int pitch2absStepByKey(int pitch, int tpc, int key, int *pAlter)
      {
      // sanitize input data
      if(pitch < 0)           pitch += PITCH_DELTA_OCTAVE;
      if(pitch > 127)         pitch -= PITCH_DELTA_OCTAVE;
      if(tpc < Tpc::TPC_MIN)  tpc   += TPC_DELTA_ENHARMONIC;
      if(tpc > Tpc::TPC_MAX)  tpc   -= TPC_DELTA_ENHARMONIC;
      if(key < int(Key::MIN)) key   += KEY_DELTA_ENHARMONIC;
      if(key > int(Key::MAX)) key   -= KEY_DELTA_ENHARMONIC;

      int octave = pitch / PITCH_DELTA_OCTAVE;
      if (tpc == Tpc::TPC_C_BB || tpc == Tpc::TPC_C_B)
            ++octave;
      else if (tpc == Tpc::TPC_B_S || tpc == Tpc::TPC_B_SS)
            --octave;
      int step = tpc2step(tpc);
      if(pAlter)
            *pAlter = tpc2alterByKey(tpc, key);
      return octave * STEP_DELTA_OCTAVE + step;
      }

//---------------------------------------------------------
//   absStep2pitchByKey
//    the default pitch for the given absolute step in the given key
//---------------------------------------------------------

int absStep2pitchByKey(int step, int key)
      {
      // sanitize input data
      if(step < 0)            step += STEP_DELTA_OCTAVE;
      if(step > 74)           step -= STEP_DELTA_OCTAVE;
      if(key < int(Key::MIN)) key  += KEY_DELTA_ENHARMONIC;
      if(key > int(Key::MAX)) key  -= KEY_DELTA_ENHARMONIC;

      int octave = step / STEP_DELTA_OCTAVE;
      int deltaPitch = step2deltaPitchByKey(step % STEP_DELTA_OCTAVE, key);
      return octave * PITCH_DELTA_OCTAVE + deltaPitch;
      }

}

