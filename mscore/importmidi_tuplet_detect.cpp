#include "importmidi_tuplet_detect.h"
#include "importmidi_tuplet.h"
#include "importmidi_meter.h"
#include "importmidi_chord.h"
#include "importmidi_quant.h"
#include "importmidi_inner.h"
#include "preferences.h"

#include <set>


namespace Ms {
namespace MidiTuplet {

bool isTupletAllowed(const TupletInfo &tupletInfo)
      {
      {
                  // special check for duplets and triplets
      const std::vector<int> nums = {2, 3};
                  // for duplet: if note first and single - only 1/2*tupletLen duration is allowed
                  // for triplet: if note first and single - only 1/3*tupletLen duration is allowed
      for (int num: nums) {
            if (tupletInfo.tupletNumber == num
                        && tupletInfo.chords.size() == 1
                        && tupletInfo.firstChordIndex == 0) {
                  const auto &chordEventIt = tupletInfo.chords.begin()->second;
                  const auto tupletNoteLen = tupletInfo.len / num;
                  for (const auto &note: chordEventIt->second.notes) {
                        if ((note.offTime - chordEventIt->first - tupletNoteLen).absValue()
                                    > tupletNoteLen / 2)
                              return false;
                        }
                  }
            }
      }
                  // for all tuplets
      const bool isHumanPerformance = preferences.midiImportOperations
                              .currentTrackOperations().quantize.humanPerformance;
      const int minAllowedNoteCount = (isHumanPerformance)
                  ? tupletLimits(tupletInfo.tupletNumber).minNoteCountHuman
                  : tupletLimits(tupletInfo.tupletNumber).minNoteCount;
      if ((int)tupletInfo.chords.size() < minAllowedNoteCount)
            return false;
                  // allow duplets and quadruplets with error == regular error
                  // because tuplet notation is simpler in that case
      if (tupletInfo.tupletNumber == 2 || tupletInfo.tupletNumber == 4) {
            if (tupletInfo.tupletSumError > tupletInfo.regularSumError)
                  return false;
            }
      else {
            if (tupletInfo.tupletSumError >= tupletInfo.regularSumError)
                  return false;
            }
                  // at least one note has to have len >= (half tuplet note len)
      const auto tupletNoteLen = tupletInfo.len / tupletInfo.tupletNumber;
      for (const auto &tupletChord: tupletInfo.chords) {
            for (const auto &note: tupletChord.second->second.notes) {
                  if (note.offTime - tupletChord.first >= tupletNoteLen / 2)
                        return true;
                  }
            }
      return false;
      }

std::vector<int> findTupletNumbers(const ReducedFraction &divLen,
                                   const ReducedFraction &barFraction)
      {
      const auto operations = preferences.midiImportOperations.currentTrackOperations();
      std::vector<int> tupletNumbers;

      if (Meter::isCompound(barFraction) && divLen == Meter::beatLength(barFraction)) {
            if (operations.tuplets.duplets)
                  tupletNumbers.push_back(2);
            if (operations.tuplets.quadruplets)
                  tupletNumbers.push_back(4);
            }
      else {
            if (operations.tuplets.triplets)
                  tupletNumbers.push_back(3);
            if (operations.tuplets.quintuplets)
                  tupletNumbers.push_back(5);
            if (operations.tuplets.septuplets)
                  tupletNumbers.push_back(7);
            if (operations.tuplets.nonuplets)
                  tupletNumbers.push_back(9);
            }

      return tupletNumbers;
      }

// find sum length of gaps between successive chords
// less is better

ReducedFraction findSumLengthOfRests(
            const TupletInfo &tupletInfo,
            const ReducedFraction &startBarTick)
      {
      auto beg = tupletInfo.onTime;
      const auto tupletEndTime = tupletInfo.onTime + tupletInfo.len;
      const auto tupletNoteLen = tupletInfo.len / tupletInfo.tupletNumber;
      ReducedFraction sumLen = {0, 1};

      for (const auto &chord: tupletInfo.chords) {
            const auto staccatoIt = tupletInfo.staccatoChords.find(chord.first);
            const MidiChord &midiChord = chord.second->second;
            const auto &chordOnTime = (chord.second->first < startBarTick)
                        ? startBarTick
                        : Quantize::findQuantizedTupletChordOnTime(*chord.second, tupletInfo.len,
                                  tupletLimits(tupletInfo.tupletNumber).ratio, startBarTick);
            if (beg < chordOnTime)
                  sumLen += (chordOnTime - beg);
            ReducedFraction maxOffTime(0, 1);
            for (int i = 0; i != midiChord.notes.size(); ++i) {
                  auto noteOffTime = midiChord.notes[i].offTime;
                  if (staccatoIt != tupletInfo.staccatoChords.end() && i == staccatoIt->second)
                        noteOffTime = chordOnTime + tupletNoteLen;
                  if (noteOffTime > maxOffTime)
                        maxOffTime = noteOffTime;
                  }
            beg = Quantize::findQuantizedTupletNoteOffTime(maxOffTime, tupletInfo.len,
                                    tupletLimits(tupletInfo.tupletNumber).ratio, startBarTick);
            if (beg >= tupletEndTime)
                  break;
            }
      if (beg < tupletEndTime)
            sumLen += (tupletEndTime - beg);
      return sumLen;
      }

TupletInfo findTupletApproximation(
            const ReducedFraction &tupletLen,
            int tupletNumber,
            const ReducedFraction &basicQuant,
            const ReducedFraction &startTupletTime,
            const std::multimap<ReducedFraction, MidiChord>::iterator &startChordIt,
            const std::multimap<ReducedFraction, MidiChord>::iterator &endChordIt)
      {
      TupletInfo tupletInfo;
      tupletInfo.tupletNumber = tupletNumber;
      tupletInfo.onTime = startTupletTime;
      tupletInfo.len = tupletLen;
      const auto tupletNoteLen = tupletLen / tupletNumber;

      struct Error
            {
            bool operator<(const Error &e) const
                  {
                  if (tupletError < e.tupletError)
                        return true;
                  if (tupletError > e.tupletError)
                        return false;
                  return (tupletRegularDiff < e.tupletRegularDiff);
                  }

            ReducedFraction tupletError;
            ReducedFraction tupletRegularDiff;
            };

      struct Candidate
            {
            int posIndex;
            std::multimap<ReducedFraction, MidiChord>::iterator chord;
            ReducedFraction regularError;
            };

      std::multimap<Error, Candidate> chordCandidates;

      for (int posIndex = 0; posIndex != tupletNumber; ++posIndex) {
            const auto tupletNotePos = startTupletTime + tupletNoteLen * posIndex;
            for (auto it = startChordIt; it != endChordIt; ++it) {
                  if (it->first < tupletNotePos - tupletNoteLen / 2)
                        continue;
                  if (it->first > tupletNotePos + tupletNoteLen / 2)
                        break;

                  const auto tupletError = (it->first - tupletNotePos).absValue();
                  const auto regularError = Quantize::findOnTimeQuantError(*it, basicQuant);
                  const auto diff = tupletError - regularError;
                  chordCandidates.insert({{tupletError, diff}, {posIndex, it, regularError}});
                  }
            }

      std::set<std::pair<const ReducedFraction, MidiChord> *> usedChords;
      std::set<int> usedPosIndexes;
      ReducedFraction diffSum;
      int firstChordIndex = 0;

      for (const auto &candidate: chordCandidates) {
            if (diffSum + candidate.first.tupletRegularDiff > ReducedFraction(0, 1))
                  break;
            const Candidate &c = candidate.second;
            if (usedPosIndexes.find(c.posIndex) != usedPosIndexes.end())
                  continue;
            if (usedChords.find(&*c.chord) != usedChords.end())
                  continue;

            usedChords.insert(&*c.chord);
            usedPosIndexes.insert(c.posIndex);
            diffSum += candidate.first.tupletRegularDiff;
            tupletInfo.chords.insert({c.chord->first, c.chord});
            tupletInfo.tupletSumError += candidate.first.tupletError;
            tupletInfo.regularSumError += c.regularError;
                        // if chord was inserted to the beginning - remember its pos index
            if (c.chord->first == tupletInfo.chords.begin()->first)
                  firstChordIndex = c.posIndex;
            }

      tupletInfo.firstChordIndex = firstChordIndex;

      return tupletInfo;
      }

// detect staccato notes; later sum length of rests of this notes
// will be reduced by enlarging the length of notes to the tuplet note length

void detectStaccato(TupletInfo &tuplet)
      {
      if ((int)tuplet.chords.size() >= tupletLimits(tuplet.tupletNumber).minNoteCountStaccato) {
            const auto tupletNoteLen = tuplet.len / tuplet.tupletNumber;
            for (auto &chord: tuplet.chords) {
                  MidiChord &midiChord = chord.second->second;
                  for (int i = 0; i != midiChord.notes.size(); ++i) {
                        if (midiChord.notes[i].offTime - chord.first < tupletNoteLen / 2) {
                                    // later if chord have one or more notes
                                    // with staccato -> entire chord is staccato

                                    // don't mark note as staccato here, only remember it
                                    // because different tuplets may contain this note,
                                    // it will be resolved after tuplet filtering
                              tuplet.staccatoChords.insert({chord.first, i});
                              }
                        }
                  }
            }
      }

// this function is needed because if there are additional chords
// that can be in the middle between tuplet chords,
// and tuplet chords are staccato, i.e. have short length,
// then tied staccato chords would be not pretty-looked converted to notation

bool haveChordsInTheMiddleBetweenTupletChords(
            const std::multimap<ReducedFraction, MidiChord>::iterator startDivChordIt,
            const std::multimap<ReducedFraction, MidiChord>::iterator endDivChordIt,
            const TupletInfo &tuplet)
      {
      std::set<std::pair<const ReducedFraction, MidiChord> *> tupletChords;
      for (const auto &chord: tuplet.chords)
            tupletChords.insert(&*chord.second);

      const auto tupletNoteLen = tuplet.len / tuplet.tupletNumber;
      for (auto it = startDivChordIt; it != endDivChordIt; ++it) {
            if (tupletChords.find(&*it) != tupletChords.end())
                  continue;
            for (int i = 0; i != tuplet.tupletNumber; ++i) {
                  const auto pos = tuplet.onTime + tupletNoteLen * i + tupletNoteLen / 2;
                  if ((pos - it->first).absValue() < tupletNoteLen / 2)
                        return true;
                  }
            }
      return false;
      }

bool isTupletLenAllowed(
            const ReducedFraction &tupletLen,
            int tupletNumber,
            const std::multimap<ReducedFraction, MidiChord>::const_iterator beg,
            const std::multimap<ReducedFraction, MidiChord>::const_iterator end,
            const ReducedFraction &basicQuant)
      {
      const auto tupletNoteLen = tupletLen / tupletNumber;
      const auto regularQuant = Quantize::findQuantForRange(beg, end, basicQuant);
      return tupletNoteLen >= regularQuant;
      }

std::vector<TupletInfo> detectTuplets(
            std::multimap<ReducedFraction, MidiChord> &chords,
            const ReducedFraction &barFraction,
            const ReducedFraction &startBarTick,
            const ReducedFraction &tol,
            const std::multimap<ReducedFraction, MidiChord>::iterator &endBarChordIt,
            const std::multimap<ReducedFraction, MidiChord>::iterator &startBarChordIt,
            const ReducedFraction &basicQuant)
      {
      const auto divLengths = Meter::divisionsOfBarForTuplets(barFraction);

      std::vector<TupletInfo> tuplets;
      int id = 0;
      for (const auto &divLen: divLengths) {
            const auto tupletNumbers = findTupletNumbers(divLen, barFraction);
            const auto div = barFraction / divLen;
            const int divCount = div.numerator() / div.denominator();

            for (int i = 0; i != divCount; ++i) {
                  const auto startDivTime = startBarTick + divLen * i;
                  const auto endDivTime = startBarTick + divLen * (i + 1);
                              // check which chords can be inside tuplet period
                              // [startDivTime - tol, endDivTime]
                  auto startDivChordIt = MChord::findFirstChordInRange(startDivTime - tol, endDivTime,
                                                                       startBarChordIt, endBarChordIt);
                  startDivChordIt = findTupletFreeChord(startDivChordIt, endBarChordIt, startDivTime);
                  if (startDivChordIt == endBarChordIt)
                        continue;

                  Q_ASSERT_X(startDivChordIt->second.isInTuplet == false,
                             "MIDI tuplets: findTuplets", "Voice of the chord has been already set");

                              // end iterator, as usual, point to the next - invalid chord
                  const auto endDivChordIt = chords.lower_bound(endDivTime);
                              // try different tuplets, nested tuplets are not allowed
                  for (const auto &tupletNumber: tupletNumbers) {
                        if (!isTupletLenAllowed(divLen, tupletNumber, startDivChordIt, endDivChordIt,
                                                basicQuant)) {
                              continue;
                              }
                        auto tupletInfo = findTupletApproximation(divLen, tupletNumber,
                                             basicQuant, startDivTime, startDivChordIt, endDivChordIt);

                        if (!haveChordsInTheMiddleBetweenTupletChords(
                                          startDivChordIt, endDivChordIt, tupletInfo)) {
                              detectStaccato(tupletInfo);
                              }
                        tupletInfo.sumLengthOfRests = findSumLengthOfRests(tupletInfo, startBarTick);

                        if (!isTupletAllowed(tupletInfo))
                              continue;
                        tupletInfo.id = id++;
                        tuplets.push_back(tupletInfo);   // tuplet found
                        }      // next tuplet type
                  }
            }

      return tuplets;
      }

} // namespace MidiTuplet
} // namespace Ms
