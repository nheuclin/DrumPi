//application.cpp

#include "application.hpp"

using namespace drumpi;


//States

PerformanceMode::PerformanceMode() {
	label = PERFORMANCE_MODE;
}

void PerformanceMode::interpretKeyPress(ApplicationCallback* appc, int key) {
	Application* app = static_cast<Application*>(appc);
	std::vector<drumID_t> drumsActive;
	std::vector<bool> activeDrums(NUM_DRUMS);
	switch (key) {
		case KEY_A:
		case KEY_S:
		case KEY_D:
		case KEY_F:
		case KEY_J:
		case KEY_K:
		case KEY_L:
		case KEY_SEMICOLON:
			// Trigger the drum sound
			app->playbackEngine.trigger(interpretDrumKey(key));
			drumsActive = app->playbackEngine.getActive();
			for(int i = 0; i < drumsActive.size(); i++) {
				activeDrums[drumsActive[i]] = true;
			}
			app->display.showPerformance(activeDrums, 0.5f);
			//Display: Toggle respective drum square and level meter
			break;
		
		case KEY_M:
			//Display: Show S to indicate mode change
			app->setState(SEQUENCER_MODE);	//change state to SequencerMode
			break;
		
		case KEY_COMMA:
			// Decrease master volume
			app->playbackEngine.volumeDown();
			//Display: Show new master volume
			break;
		case KEY_DOT:
			// Increase master volume
			app->playbackEngine.volumeUp();
			//Display: Show new master volume
			break;
		
		case KEY_V:
			app->setState(SET_DRUM_VOLUME_MODE);	//change state to SetDrumVolumeMode
			break;
	}
}


SequencerMode::SequencerMode() {
	label = SEQUENCER_MODE;
	currentdrum = interpretDrumKey(KEY_A);	//default drum A
	currentpage = 0;	//default page 1 (beats 1-8)
	//Display: Show tom 1 drum page 1 sequence
}

void SequencerMode::interpretKeyPress(ApplicationCallback* appc, int key) {
	Application* app = static_cast<Application*>(appc);
	switch (key) {
		case KEY_A:
		case KEY_S:
		case KEY_D:
		case KEY_F:
		case KEY_J:
		case KEY_K:
		case KEY_L:
		case KEY_SEMICOLON:
			// Interpret the drum key
			currentdrum = interpretDrumKey(key);
			// Trigger the drum sound
			app->playbackEngine.trigger(currentdrum);
			//Display: Toggle corresponding DP
			//display currentpage for respective drum
			break;
		
		case KEY_1:
		case KEY_2:
		case KEY_3:
		case KEY_4:
		case KEY_5:
		case KEY_6:
		case KEY_7:
		case KEY_8:
			// Toggle currentdrum in relevant beat
			app->seq->toggle(
				currentdrum,
				(key - KEY_1) + (currentpage * 8)
			);
			break;
		
		case KEY_TAB:
			currentpage++;
			if (currentpage > 1) currentpage = 0;
			// Display relevant beats for currentdrum
			break;
			
		case KEY_SPACE:
			// Toggle Sequencer's activity state
			if(app->seqClocker->isActive()) {
				// Stop the Sequencer's clock
				app->seqClocker->stop();
				// Reset the Sequencer to first step, not clearing the sequence
				app->seq->reset(false);
			} else {
				app->seqClocker->start();
			}
			// Toggle play/pause sequence display
			break;

		case KEY_M:
			//Display: switch to performance display mode
			//stop sequencer before switching to PerformanceMode
			app->seqClocker->stop();
			app->setState(PERFORMANCE_MODE);	//change state to PerformanceMode
			break;

		case KEY_COMMA:
			// Decrease master volume
			app->playbackEngine.volumeDown();
			//Display: updated master volume
			break;
		case KEY_DOT:
			// Increase master volume
			app->playbackEngine.volumeUp();
			//Display: updated master volume
			break;

		case KEY_T:
			app->setState(SET_TEMPO_MODE);	//change state to SetTempoMode
			break;
		case KEY_V:
			app->setState(SET_DRUM_VOLUME_MODE);	//change state to SetDrumVolumeMode
			break;
	}
}


SetTempoMode::SetTempoMode() {
	label = SET_TEMPO_MODE;
}

void SetTempoMode::interpretKeyPress(ApplicationCallback* appc, int key) {
	Application* app = static_cast<Application*>(appc);
	switch (key) {
		case KEY_DOT:
			//increase tempo
			app->seqClocker->setRateBPM(app->seqClocker->getRateBPM() + 20);
			break;
		case KEY_COMMA:
			//decrease tempo
			app->seqClocker->setRateBPM(app->seqClocker->getRateBPM() - 20);
			break;
			
		case KEY_T:
		case KEY_ESC:
			// Exit SetTempoMode
			app->setState(SEQUENCER_MODE);	//change state to SequencerMode
			break;
	}
}


SetDrumVolumeMode::SetDrumVolumeMode() {
	label = SET_DRUM_VOLUME_MODE;
	drumselected = interpretDrumKey(KEY_A);	//default drum A
	previousstate = PERFORMANCE_MODE;	//default previous state
}

void SetDrumVolumeMode::interpretKeyPress(ApplicationCallback *appc, int key) {
	Application* app = static_cast<Application*>(appc);
	switch (key) {
		case KEY_DOT:
			// Increase selected drum's volume
			app->playbackEngine.volumeUp(drumselected);
			// Trigger selected drum for user reference
			app->playbackEngine.trigger(drumselected);
			break;
		case KEY_COMMA:
			// Decrease selected drum's volume
			app->playbackEngine.volumeDown(drumselected);
			// Trigger selected drum for user reference
			app->playbackEngine.trigger(drumselected);
			break;
		
		case KEY_A:
		case KEY_S:
		case KEY_D:
		case KEY_F:
		case KEY_J:
		case KEY_K:
		case KEY_L:
		case KEY_SEMICOLON:
			// Interpret the drum key
			drumselected = interpretDrumKey(key);
			// Trigger the drum sound
			app->playbackEngine.trigger(drumselected);
			break;

		case KEY_V:
		case KEY_ESC:
			// Exit SetDrumVolumeMode
			app->setState(previousstate);
			break;
	}
}


//Application

Application::Application() {
	currentstate = &performancemode;
	kbdThread.kbdIn.connectCallback(this);

	// Jack client
	audioEngine.reset(new audio::JackClient("DrumPi"));

	// Get the PlaybackEngine to load all of the audio samples for bank 0
	for (int i = 0; i < NUM_DRUMS; i++) {
		playbackEngine.setSource((drumID_t)i, 0, audio::SOURCE_PREGENERATED);
	}

	// Sequencer
	seq.reset(new Sequencer(16));
	// SequencerClock
	seqClocker.reset(new SequencerClock(seq, playbackEngine));
}

void Application::run() {
	// Start the audio stream
	audioEngine->start(playbackEngine);

	kbdThread.start();

	while(running) {}

	kbdThread.stop();

	audioEngine->stop();
}

void Application::interpretKeyPress(int key) {
	currentstate->interpretKeyPress(this, key);
}

void Application::setState(stateLabel_t newstate) {
	switch (newstate) {
		case PERFORMANCE_MODE:
			currentstate = &performancemode;
			//switch display to performance mode
			break;
		case SEQUENCER_MODE:
			currentstate = &sequencermode;
			sequencermode.currentpage = 0;	//switch to default page
			sequencermode.currentdrum = currentstate->interpretDrumKey(KEY_A);	//switch to default drum A
			//switch display to sequencer mode
			break;
		case SET_TEMPO_MODE:
			currentstate = &settempomode;
			break;
		case SET_DRUM_VOLUME_MODE:
			if (currentstate == &sequencermode) {
				setdrumvolumemode.previousstate = SEQUENCER_MODE;
				currentstate = &setdrumvolumemode;
			} else if (currentstate == &performancemode) {
				setdrumvolumemode.previousstate = PERFORMANCE_MODE;
				currentstate = &setdrumvolumemode;
			}
			break;
	}
}

drumID_t State::interpretDrumKey(int key) {
	switch (key) {
		case KEY_A : default :
			return DRUM_1;
			break;
		case KEY_S:
			return DRUM_2;
			break;
		case KEY_D:
			return DRUM_3;
			break;
		case KEY_F:
			return DRUM_4;
			break;
		case KEY_J:
			return DRUM_5;
			break;
		case KEY_K:
			return DRUM_6;
			break;
		case KEY_L:
			return DRUM_7;
			break;
		case KEY_SEMICOLON:
			return DRUM_8;
			break;
	}
}
