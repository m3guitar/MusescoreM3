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

#include "NoteMappings.h"
#include "xml.h"

//cc
namespace Ms {

//---------------------------------------------------------
//   NoteMappings
//          For alternative notations.
//---------------------------------------------------------

NoteMappings::NoteMappings() : _notePositions { 3, 0, 4, 1, 5, 2, 6, 3, 0, 4, 1, 5, 2, 6, 3,
                                                0, 4, 1, 5, 2, 6, 3, 0, 4, 1, 5, 2, 6, 3, 0,
                                                4, 1, 6, 2, 6 }, //traditional positions
                               _noteHeads { defaultHg, defaultHg, defaultHg, defaultHg, defaultHg,
                                            defaultHg, defaultHg, defaultHg, defaultHg, defaultHg,
                                            defaultHg, defaultHg, defaultHg, defaultHg, defaultHg,
                                            defaultHg, defaultHg, defaultHg, defaultHg, defaultHg,
                                            defaultHg, defaultHg, defaultHg, defaultHg, defaultHg,
                                            defaultHg, defaultHg, defaultHg, defaultHg, defaultHg,
                                            defaultHg, defaultHg, defaultHg, defaultHg, defaultHg }
      {
      setTraditionalClefOffsets();
      }
      
bool NoteMappings::operator==(const NoteMappings& n) const
      {
      return n._notePositions == _notePositions
             && n._noteHeads == _noteHeads
             && n._clefOffsets == _clefOffsets
             && n._octaveDistance == _octaveDistance
             && n._showAccidentals == _showAccidentals;
      }


//TODO: method headers
      
void NoteMappings::write(Xml& xml) const
      {
      xml.stag(QString("noteMappings"));
      writeMappings(xml);
      writeClefOffsets(xml);
      xml.tag("octaveDistance", _octaveDistance);
      xml.tag("showAccidentals", (int)_showAccidentals);
      xml.etag();
      }
      
void NoteMappings::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            QString tag = e.name().toString();
            if (tag == "mappings")
                  readMappings(e);
            else if (tag == "clefOffsets")
                  readClefOffsets(e);
            else if (tag == "octaveDistance")
                  _octaveDistance = e.readInt();
            else if (tag == "showAccidentals")
                  _showAccidentals = e.readBool();
            else
                  throw (QObject::tr("improperly formated."));
            }
      }

void NoteMappings::writeMappings(Xml& xml) const
      {
      xml.stag("mappings");
            
      for (int tpc = -1; tpc < 34; tpc++) {
            QString name = tpc2name(tpc, NoteSpellingType::STANDARD, false, false);
            
            xml.stag(QString("note name=\"%1\" tpc=\"%2\"").arg(name).arg(tpc));
            xml.tag("line-offset", _notePositions[tpc + 1]);
            
            QString groupName;
            NoteHead::Group group = _noteHeads[tpc + 1];
            if (group == NoteHead::Group::HEAD_NORMAL)
                  groupName = "normal";
            else if (group == NoteHead::Group::HEAD_SLASH)
                  groupName = "slash";
            else if (group == NoteHead::Group::HEAD_TRIANGLE)
                  groupName = "triangle";
            else if (group == NoteHead::Group::HEAD_DIAMOND)
                  groupName = "diamond";
            else if (group == NoteHead::Group::HEAD_CROSS)
                  groupName = "cross";
            else if (group == NoteHead::Group::HEAD_XCIRCLE)
                  groupName = "xcircle";
            else if (group == NoteHead::Group::HEAD_DO)
                  groupName = "do";
            else if (group == NoteHead::Group::HEAD_RE)
                  groupName = "re";
            else if (group == NoteHead::Group::HEAD_MI)
                  groupName = "mi";
            else if (group == NoteHead::Group::HEAD_FA)
                  groupName = "fa";
            else if (group == NoteHead::Group::HEAD_LA)
                  groupName = "la";
            else if (group == NoteHead::Group::HEAD_TI)
                  groupName = "ti";
            else if (group == NoteHead::Group::HEAD_SOL)
                  groupName = "sol";
            else if (group == NoteHead::Group::HEAD_BREVIS_ALT)
                  groupName = "brevis_alt";
            else if (group == NoteHead::Group::HEAD_GROUPS)
                  groupName = "groups";
            
            xml.tag("notehead-group", groupName);
            xml.etag();
            }
            
      xml.etag();
      }

void NoteMappings::readMappings(XmlReader& e)
      {
      while (e.readNextStartElement() && (e.name().toString() == "note")) {
            int tpc = e.attribute("tpc", "-2").toInt();
            
            while (e.readNextStartElement()) {
                  QString tag = e.name().toString();
                  if (tag == "line-offset") {
                        int offset = e.readInt();
                        _notePositions[tpc + 1] = offset;
                        }
                  else if (tag == "notehead-group") {
                        QString text = e.readElementText();
                        NoteHead::Group group;
                        if (text == "normal")
                              group = NoteHead::Group::HEAD_NORMAL;
                        else if (text == "cross")
                              group = NoteHead::Group::HEAD_CROSS;
                        else if (text == "diamond")
                              group = NoteHead::Group::HEAD_DIAMOND;
                        else if (text == "triangle")
                              group = NoteHead::Group::HEAD_TRIANGLE;
                        else if (text == "mi")
                              group = NoteHead::Group::HEAD_MI;
                        else if (text == "slash")
                              group = NoteHead::Group::HEAD_SLASH;
                        else if (text == "xcircle")
                              group = NoteHead::Group::HEAD_XCIRCLE;
                        else if (text == "do")
                              group = NoteHead::Group::HEAD_DO;
                        else if (text == "re")
                              group = NoteHead::Group::HEAD_RE;
                        else if (text == "fa")
                              group = NoteHead::Group::HEAD_FA;
                        else if (text == "la")
                              group = NoteHead::Group::HEAD_LA;
                        else if (text == "ti")
                              group = NoteHead::Group::HEAD_TI;
                        else if (text == "sol")
                              group = NoteHead::Group::HEAD_SOL;
                        else if (text == "brevis_alt")
                              group = NoteHead::Group::HEAD_BREVIS_ALT;
                        else if (text == "groups")
                              group =  NoteHead::Group::HEAD_GROUPS;
                        else
                              throw (QObject::tr("file includes an unrecognized notehead group: %1").arg(text));

                        _noteHeads[tpc + 1] = group;
                        }
                  }
            }
      }
      
void NoteMappings::writeClefOffsets(Xml& xml) const
      {
      xml.stag("clefOffsets");
      
      std::map<ClefType, int>::const_iterator itr = _clefOffsets.begin();
      while (itr != _clefOffsets.end()) {
            
            QString name;
            ClefType ct = itr->first;
            if (ct == ClefType::G)
                  name = "G";
            else if (ct == ClefType::G1)
                  name = "G8va";
            else if (ct == ClefType::G2)
                  name = "G15ma";
            else if (ct == ClefType::G3)
                  name = "G8vb";
            else if (ct == ClefType::F)
                  name = "F";
            else if (ct == ClefType::F8)
                  name = "F8vb";
            else if (ct == ClefType::F15)
                  name = "F15mb";
            else if (ct == ClefType::F_B)
                  name = "F3";
            else if (ct == ClefType::F_C)
                  name = "F5";
            else if (ct == ClefType::C1)
                  name = "C1";
            else if (ct == ClefType::C2)
                  name = "C2";
            else if (ct == ClefType::C3)
                  name = "C3";
            else if (ct == ClefType::C4)
                  name = "C4";
            else if (ct == ClefType::C5)
                  name = "C5";
            else if (ct == ClefType::G4)
                  name = "G1";
            else if (ct == ClefType::F_8VA)
                  name = "F8va";
            else if (ct == ClefType::F_15MA)
                  name = "F15ma";
            else if (ct == ClefType::INVALID)
                  name = "INVALID";
            
            xml.tag(QString("clef name=\"%1\"").arg(name), itr->second);
            itr++;
            }
      
      xml.etag();
      }

void NoteMappings::readClefOffsets(XmlReader& e)
      {
      while (e.readNextStartElement() && (e.name().toString() == "clef")) {
            QString name = e.attribute("name", "");
            
            ClefType ct;
            if (name == "G")
                  ct = ClefType::G;
            else if (name == "G8va")
                  ct = ClefType::G1;
            else if (name == "G15ma")
                  ct = ClefType::G2;
            else if (name == "G8vb")
                  ct = ClefType::G3;
            else if (name == "F")
                  ct = ClefType::F;
            else if (name == "F8vb")
                  ct = ClefType::F8;
            else if (name == "F15mb")
                  ct = ClefType::F15;
            else if (name == "F3")
                  ct = ClefType::F_B;
            else if (name == "F5")
                  ct = ClefType::F_C;
            else if (name == "C1")
                  ct = ClefType::C1;
            else if (name == "C2")
                  ct = ClefType::C2;
            else if (name == "C3")
                  ct = ClefType::C3;
            else if (name == "C4")
                  ct = ClefType::C4;
            else if (name == "C5")
                  ct = ClefType::C5;
            else if (name == "G1")
                  ct = ClefType::G4;
            else if (name == "F8va")
                  ct = ClefType::F_8VA;
            else if (name == "F15ma")
                  ct = ClefType::F_15MA;
            else if (name == "INVALID")
                  ct = ClefType::INVALID;
            else
                  throw (QObject::tr("file includes an unrecognized clef type."));;

            _clefOffsets[ct] = e.readInt();
            }
      }

inline int positive_modulo(int i, int n) {
    return (i % n + n) % n;
}

//TODO: implement key-aware decisions
int NoteMappings::getTpc(int position, int accidental) {
      int tpc = getTpc(position);
      int adjustedTpc = (tpc + (accidental * 7)) % 35;     //check if an accidental-adjusted value gives same tpc
      if (_notePositions[adjustedTpc + 1] == position)     //    if so, choose it instead
            return adjustedTpc;
      else
            return tpc;
      }

//TODO: implement key-aware decisions
//      find simpler way to write this function
int NoteMappings::getTpc(int position) {
      position = positive_modulo(position, 12);

      //check for tpc's without sharps or flats
      for (int i = 14; i < 21; i++) {
            if (positive_modulo(_notePositions[i], _octaveDistance) == position)
                  return i - 1;
            }
      
      //check sharps and flats
      for (int i = 7; i < 14; i++) {
            if (positive_modulo(_notePositions[i], _octaveDistance) == position)
                  return i - 1;
            }
      for (int i = 21; i < 28; i++) {
            if (positive_modulo(_notePositions[i], _octaveDistance) == position)
                  return i - 1;
            }
      
      //save double flats/sharps as last resorts
      for (int i = 0; i < 14; i++) {
            if (positive_modulo(_notePositions[i], _octaveDistance) == position)
                  return i - 1;
            }
      for (int i = 21; i < 35; i++) {
            if (positive_modulo(_notePositions[i], _octaveDistance) == position)
                  return i - 1;
            }
      
      qFatal("position is not mapped by any tpc");
      }

void NoteMappings::setTraditionalClefOffsets() {
      _clefOffsets[ClefType::G] = 45;
      _clefOffsets[ClefType::G1] = 52;
      _clefOffsets[ClefType::G2] = 59;
      _clefOffsets[ClefType::G3] = 38;
      _clefOffsets[ClefType::F] = 33;
      _clefOffsets[ClefType::F8] = 26;
      _clefOffsets[ClefType::F15] = 19;
      _clefOffsets[ClefType::F_B] = 35;
      _clefOffsets[ClefType::F_C] = 31;
      _clefOffsets[ClefType::C1] = 43;
      _clefOffsets[ClefType::C2] = 41;
      _clefOffsets[ClefType::C3] = 39;
      _clefOffsets[ClefType::C4] = 37;
      _clefOffsets[ClefType::C5] = 35;
      _clefOffsets[ClefType::G4] = 47;
      _clefOffsets[ClefType::F_8VA] = 40;
      _clefOffsets[ClefType::F_15MA] = 47;
      }
      
int NoteMappings::getPitch(int tpc, int step) {
      static const int pitches[12] = { 10, 5, 0, 7, 2, 9, 4, 11, 6, 1, 8, 3 };
      
      int octave = (step - _notePositions[tpc + 1]) / _octaveDistance;
      return pitches[(tpc + 12) % 12] + (12 * octave);
      }

}