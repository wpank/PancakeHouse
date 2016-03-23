#pragma once
#include <cmath>
class Filter
{
public:
	enum FilterMode {
		FILTER_MODE_LOWPASS = 0,
		FILER_MODE_HIGHPASS,
		FILTER_MODE_BANDPASS,
		knumFilterModes
	};
	Filter() :
		cutoff(0.99),
		resonance(0.01),
		cutoffMod(0.0),
		mode(FILTER_MODE_LOWPASS),
		buf0(0.0),
		buf1(0.0),
		buf2(0.0),
		buf3(0.0){}

	double process(double inputValue);
	inline void setCutoff(double newCutoff) { cutoff = newCutoff; calculateFeedbackAmount(); }
	inline void setResonance(double newResonance) { resonance = newResonance; calculateFeedbackAmount(); }
	inline void setFilterMode(FilterMode newMode) { mode = newMode; }

	inline void setCutoffMod(double newCutoffMod) {
		cutoffMod = newCutoffMod;
		calculateFeedbackAmount();
	}

	void reset() {
		buf0 = buf1 = buf2 = buf3 = 0.0;
	}

private:
	double cutoff;
	double resonance;
	FilterMode mode;
	double feedbackAmount;
	inline void calculateFeedbackAmount() { feedbackAmount = resonance + resonance / (1.0 - getCalculatedCutoff()); }
	double buf0;
	double buf1;
	double buf2;
	double buf3;

	double cutoffMod;
	inline double getCalculatedCutoff() const {
		return fmax(fmin(cutoff + cutoffMod, 0.99), .01);
	};


};

