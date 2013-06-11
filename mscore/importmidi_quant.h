#ifndef IMPORTMIDI_QUANT_H
#define IMPORTMIDI_QUANT_H


namespace Ms {

class MidiChord;
class TimeSigMap;
struct TupletData;

namespace Quantize {

void applyAdaptiveQuant(std::multimap<int, MidiChord> &, const TimeSigMap *, int);

void applyGridQuant(std::multimap<int, MidiChord> &chords,
                    const TimeSigMap* sigmap,
                    int lastTick);

void quantizeChordsAndFindTuplets(std::multimap<int, TupletData> &tupletEvents,
                                  std::multimap<int, MidiChord> &chords,
                                  const TimeSigMap* sigmap,
                                  int lastTick);

} // namespace Quantize
} // namespace Ms


#endif // IMPORTMIDI_QUANT_H
