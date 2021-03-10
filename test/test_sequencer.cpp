#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SequencerTest
#include <boost/test/unit_test.hpp>
#include <sequencer.hpp>

#include "defs.hpp"

using namespace drumpi;
using namespace sequencer;

int numSteps = 8;

BOOST_AUTO_TEST_CASE(constructors) {
	// Test for NULL object pointers
	Sequencer seq1(numSteps);
	Sequencer seq2(numSteps);

	BOOST_CHECK(&seq1);
	BOOST_CHECK(&seq2);
}

BOOST_AUTO_TEST_CASE(stepping) {
	// Test the stepping functionality
	Sequencer seq(numSteps);

	// Should be on step 0
	BOOST_CHECK(seq.getStepNum() == 0);

	// Advance by one, should be on step 1
	seq.step();
	BOOST_CHECK(seq.getStepNum() == 1);

	// Advance by 2, should be on step 3
	seq.step(2);
	BOOST_CHECK(seq.getStepNum() == 3);

	// Check wrapping around
	// Wrap around and then step again
	for (int i = 0; i < 6; i++) seq.step();
	BOOST_CHECK(seq.getStepNum() == 1);
}

BOOST_AUTO_TEST_CASE(addRemoveDrums) {
	// Test adding, removing and retrieving drums to/from various Steps
	Sequencer seq(numSteps);
	drumID_t d1 = KICK_DRUM;
	drumID_t d2 = SNARE_DRUM;

	seq.step();
	// Add d1 to current step (1)
	seq.addToStep(d1);
	// Add d1 and d2 to step 2
	seq.addToStep(d1, 2);
	seq.addToStep(d2, 2);

	BOOST_CHECK(seq.isActive(d1));
	BOOST_CHECK(seq.isActive(d1, 2));
	BOOST_CHECK(seq.isActive(d2, 2));
	BOOST_CHECK(!seq.isActive(d2)); // d2 not active in current step

	std::vector<drumID_t> active = seq.getActive();
	BOOST_CHECK(active.size() == 1);
	BOOST_CHECK(active[0] == d1);

	seq.removeFromStep(d1);
	active = seq.getActive();
	BOOST_CHECK(active.empty());

	// Check step 2
	seq.step();
	active = seq.getActive();
	BOOST_CHECK(active.size() == 2);
	BOOST_CHECK(active[0] == d1);
	BOOST_CHECK(active[1] == d2);
}

BOOST_AUTO_TEST_CASE(sequence) {
	// Test retrieving the step & sequence pattern
	Sequencer seq(numSteps);

	std::vector<std::vector<bool>> s;

	s.resize(numSteps);
	for (int i = 0; i < s.size(); i++) { // For each step...
		s[i].resize(NUM_DRUMS);
		for (int j = 0; j < s[i].size(); j++) { // For each drum...
			s[i][j] = static_cast<bool>(rand() % 2);
			if (s[i][j]) seq.addToStep((drumID_t)j, i);
		}
	}

	drumID_t td = KICK_DRUM;
	std::vector<bool> steps = seq.getSteps(td);

	bool error = false;
	for (int i = 0; i < NUM_DRUMS; i++) {
		error = error || (steps[i] != s[i][td]);
	}

	BOOST_CHECK(!error);


	std::vector<std::vector<bool>> active = seq.getSequence();

	error = false;
	for (int i = 0; i < s.size(); i++) {
		for (int j = 0; j < s[i].size(); j++) {
			error = error || (active[i][j] != s[i][j]);
		}
	}

	BOOST_CHECK(!error);
}

BOOST_AUTO_TEST_CASE(resetting) {
	// Test resetting the Sequencer
	Sequencer seq(numSteps);

	std::vector<std::vector<bool>> s;

	s.resize(numSteps);
	for (int i = 0; i < s.size(); i++) { // For each step...
		s[i].resize(NUM_DRUMS);
		for (int j = 0; j < s[i].size(); j++) { // For each drum...
			s[i][j] = static_cast<bool>(rand() % 2);
			if (s[i][j]) seq.addToStep((drumID_t)j, i);
		}
	}

	seq.step();
	seq.reset();

	std::vector<std::vector<bool>> active = seq.getSequence();

	bool error = false;
	for (int i = 0; i < s.size(); i++) {
		for (int j = 0; j < s[i].size(); j++) {
			error = error or active[i][j];
		}
	}

	BOOST_CHECK(!error);
	BOOST_CHECK(seq.getStepNum() == 0);
}