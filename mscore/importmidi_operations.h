#ifndef IMPORTMIDI_OPERATIONS_H
#define IMPORTMIDI_OPERATIONS_H

#include <QList>


struct MidiTrackOperations
      {
      bool doImport;
      bool doLHRHSeparation;
      };

typedef QList<MidiTrackOperations> tMidiImportOperations;

class MidiImportOperations
      {
   public:
      const tMidiImportOperations& allOperations() const { return operations_; }
      void addTrackOperations(const MidiTrackOperations& operations);
      void clear();

   private:
      tMidiImportOperations operations_;
      };


#endif // IMPORTMIDI_OPERATIONS_H
