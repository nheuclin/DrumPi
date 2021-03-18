// File: defs.hpp
#ifndef DRUMPI_DEFS_H
#define DRUMPI_DEFS_H

/*! Use this file to define types, constants etc that are needed in the program.
DO NOT instantiate any objects, variables etc here. */

namespace drumpi {

typedef enum _DrumIDs {
    KICK_DRUM = 0,
    SNARE_DRUM,
    _NUM_DRUMS
} drumID_t;

#define NUM_DRUMS (int)_DrumIDs::_NUM_DRUMS

typedef enum _StateLabels {
    PERFORMANCE_MODE,
    SEQUENCER_MODE,
    SET_TEMPO_MODE,
    SET_DRUM_VOLUME_MODE
} stateLabel_t;

} // namespace drumpi

#endif
