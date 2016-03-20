#ifndef __PancakeHouse__Oscillator__
#define __PancakeHouse__Oscillator__

#include <math.h>



class Oscillator {
public:
	enum OscillatorMode {
		OSCILLATOR_MODE_SINE = 0,
		OSCILLATOR_MODE_SAW,
		OSCILLATOR_MODE_SQUARE,
		OSCILLATOR_MODE_TRIANGLE,
		kNumOscillatorModes
	};
	void setMode(OscillatorMode mode);
	void setFrequency(double frequency);
	void setSampleRate(double sampleRate);
	void generate(double* buffer, int nFrames);
	inline void setMuted(bool muted) { isMuted = muted; }
	double nextSample();
	Oscillator() :
		mOscillatorMode(OSCILLATOR_MODE_SINE),
		mPI(2 * acos(0.0)),
		twoPI(2 * mPI),
		isMuted(true),
		mFrequency(440.0),
		mPhase(0.0),
		mSampleRate(44100.0) {
		updateIncrement();
	};

private:
	OscillatorMode mOscillatorMode;
	const double mPI;
	double mFrequency;
	double mPhase;
	double mSampleRate;
	double mPhaseIncrement;
	void updateIncrement();
	const double twoPI;
	bool isMuted;
};

#endif /* defined(__PancakeHouse__Oscillator__) */
/* defined(__PancakeHouse__Oscillator__) */