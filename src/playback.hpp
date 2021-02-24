// File: playback.hpp
#ifndef DRUMPI_PLAYBACK_H
#define DRUMPI_PLAYBACK_H

#include <vector>

#include "defs.hpp"
#include "sampleSource.hpp"

namespace drumpi {
namespace audio {

/*! Sample handling class.
Manages audio clips for sending to output. */
template<typename T = SAMPLE_FORMAT>
class PlaybackEngine {
    public:
        /*! Constructor. */
        PlaybackEngine();

        /*! Retrieves samples.
        \param nSamples number of samples to return.
        \return a buffer of samples. */
        std::vector<T> getSamples(int nSamples);

        /*! Adds the specified drum to the output engine.
        \param id ID of the drum to add. */
        void addToOutput(int id);

        /*! Removes the specified drum sample from the output.
        \param id ID of the drum to remove. */
        void removeFromOutput(int id);
    
    private:
        /*! SampleSource object pointers. */
        std::vector<SampleSource<T>> sources;
        /*! Switches to store whether each source is being played. */
        std::vector<bool> switches;
        /*! Types of sample retrievers. */
        std::vector<sampleSourceType_t> types;

        /*! Counter storing the number of active clips. */
        int numActive;
};

} // namespace audio
} // namespace drumpi

#endif // define DRUMPI_PLAYBACK_H