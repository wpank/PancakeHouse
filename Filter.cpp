#include "Filter.h"



double Filter::process(double inputValue) {
	if (inputValue == 0.0) return inputValue;
	double calculatedCutoff = getCalculatedCutoff();
	buf0 += calculatedCutoff * (inputValue - buf0 + feedbackAmount * (buf0 - buf1));
	buf1 += calculatedCutoff * (buf0 - buf1);
	buf2 += calculatedCutoff * (buf1 - buf2);
	buf3 += calculatedCutoff * (buf2 - buf3);
	switch (mode) {
		case FILTER_MODE_LOWPASS:
			return buf1;
		case FILER_MODE_HIGHPASS:
			return inputValue - buf0;
		case FILTER_MODE_BANDPASS:
			return buf0 - buf3;
		default:
			return 0.0;
	}
}