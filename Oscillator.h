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
	double nextSample();
	Oscillator() :
		mOscillatorMode(OSCILLATOR_MODE_SINE),
		mPI(2 * acos(0.0)),
		twoPI(2 * mPI),
		mFrequency(440.0),
		mPhase(0.0),
		mPitchMod(0.0){
		updateIncrement();
	};

	void reset() { mPhase = 0.0;  }
	void setPitchMod(double amount);
private:
	OscillatorMode mOscillatorMode;
	const double mPI;
	double mFrequency;
	double mPhase;
	static double mSampleRate;
	double mPhaseIncrement;
	void updateIncrement();
	const double twoPI;
	double mPitchMod;
};

#endif /* defined(__PancakeHouse__Oscillator__) */
/* defined(__PancakeHouse__Oscillator__) */