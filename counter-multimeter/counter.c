#include "counter.h"

uint32_t cnt_MeasureFrequency(uint8_t precision, uint32_t estimate) {
	uint64_t resolution = 1;
	uint8_t p = precision;
	for (; p > 0; p--) {
		resolution *= 10;
	}
	resolution *= 1000;
	if (estimate >= CNT_REF_TIMER_FREQ || estimate == 0) {
		// use normal measurement mode:
		// calculate necessary time
		uint32_t ms;
		if (estimate > 0) {
			ms = resolution / estimate;
		} else {
			ms = COUNTER_DEF_SAMPLE_TIME;
		}
		// take measurement
		uint32_t pulses = counter_MeasureRefGate(CNT_GATE_MS(ms));
		// convert to frequency
		return (((uint64_t) pulses) * 1000) / ms;
	} else {
		// use indirect measurement as this will be faster:
		// calculate necessary time to achieve desired precision
		uint32_t ms = resolution / CNT_REF_TIMER_FREQ;
		// estimate edges within that time
		uint32_t edges = estimate / 500 * ms;
		// sample frequency using double the amount of time as precaution
		uint32_t timediff = counter_SignalPulsesTime(edges, ms * 2);
		if (timediff == 0) {
			// timeout occured, estimate was too high
			// decrease estimate
			estimate >>= 1;
			// sample again
			return cnt_MeasureFrequency(precision, estimate);
		} else {
			// got a result
			// convert to frequency
			return (((uint64_t) edges) * CNT_REF_TIMER_FREQ) / (timediff * 2);
		}
	}
}

uint32_t cnt_GetEstimate(void) {
	uint32_t estimate = 0;
	uint8_t prescaler = 0;
	switch (counter.input) {
	case CNT_IN_TTL:
		// no prescaler available
		// -> take measurement
		estimate = counter_MeasureRefGate(CNT_GATE_MS(COUNTER_DEF_SAMPLE_TIME));
		// convert to frequency
		estimate *= 1000;
		estimate /= COUNTER_DEF_SAMPLE_TIME;
		break;
	case CNT_IN_LF:
		// multiple prescalers available
		// -> start with highest prescaler to avoid aliasing
		do {
			if (!prescaler) {
				// set prescaler in first loop iteration
				prescaler = CNT_HIGHEST_PRESCALER_LF;
			} else {
				// decrease prescaler in further loop iterations
				prescaler = CNT_LOWER_PRESCALER_LF(prescaler);
			}
			// set new prescaler
			counter_SelectInput(CNT_IN_LF, prescaler);
			// take measurement
			estimate = counter_MeasureRefGate(
					CNT_GATE_MS(COUNTER_DEF_SAMPLE_TIME));
			// keep lowering the prescaler until the frequency at
			// the input pin (e.i. after the prescaler) rises above
			// 500kHz or the lowest prescaler has been reached
		} while ((prescaler != CNT_LOWEST_PRESCALER_LF)
				&& (estimate < COUNTER_DEF_SAMPLE_TIME * 500));
		// convert to frequency
		estimate *= 1000;
		estimate /= COUNTER_DEF_SAMPLE_TIME;
		estimate *= prescaler;
		break;
	}
	return estimate;
}

uint8_t cnt_GetOptimalPrescaler(uint32_t estimate) {
	uint8_t pre = 0;
	switch (counter.input) {
	case CNT_IN_TTL:
		// no prescaler available for TTL input
		pre = 1;
		break;
	case CNT_IN_LF:
		// start with lowest prescaler and increase as much as necessary
		pre = CNT_LOWEST_PRESCALER_LF;
		while (estimate / pre > COUNTER_MAX_INPUTPIN_FREQ
				&& pre != CNT_HIGHEST_PRESCALER_LF) {
			pre = CNT_HIGHER_PRESCALER_LF(pre);
		}
		break;
	}
	return pre;
}

uint32_t cnt_TakeMeasurement(uint8_t range) {
	if (range == COUNTER_RANGE_NONE)
		// don't take measurement
		return 0;
	// check TTL input first
	counter_SelectInput(CNT_IN_TTL, CNT_TTL_PRE_1);
	uint32_t estimateTTL = cnt_GetEstimate();
	counter_SelectInput(CNT_IN_LF, CNT_HIGHEST_PRESCALER_LF);
	uint32_t estimateLF = cnt_GetEstimate();
	uint32_t estimate = 0;
	uint32_t measurement = 0;
	uint8_t Prescaler = 0;
	uint8_t AliasingWarning = 0;
	// calculate prescaler
	switch (range) {
	case COUNTER_RANGE_AUTO:
		if (estimateTTL == 0) {
			// no input signal at TTL input
			// -> switch to LF input
			// calculate optimal prescaler (auto-ranging)
			Prescaler = cnt_GetOptimalPrescaler(estimateLF);
			// select prescaler
			counter_SelectInput(CNT_IN_LF, Prescaler);
			estimate = estimateLF;
		} else {
			// using TTL with prescaler = 1
			counter_SelectInput(CNT_IN_TTL, CNT_TTL_PRE_1);
			Prescaler = CNT_TTL_PRE_1;
			estimate = estimateTTL;
		}
		break;
	case COUNTER_RANGE_4MHz_TTL:
		counter_SelectInput(CNT_IN_TTL, CNT_TTL_PRE_1);
		Prescaler = CNT_TTL_PRE_1;
		break;
	case COUNTER_RANGE_4MHz:
		counter_SelectInput(CNT_IN_LF, CNT_LF_PRE_1);
		Prescaler = CNT_LF_PRE_1;
		if (estimateLF > 4000000UL)
			AliasingWarning = 1;
		break;
	case COUNTER_RANGE_16MHz:
		counter_SelectInput(CNT_IN_LF, CNT_LF_PRE_4);
		Prescaler = CNT_LF_PRE_4;
		if (estimateLF > 16000000UL)
			AliasingWarning = 1;
		break;
	case COUNTER_RANGE_32MHz:
		counter_SelectInput(CNT_IN_LF, CNT_LF_PRE_8);
		Prescaler = CNT_LF_PRE_8;
		if (estimateLF > 32000000UL)
			AliasingWarning = 1;
		break;
	case COUNTER_RANGE_64MHz:
		counter_SelectInput(CNT_IN_LF, CNT_LF_PRE_16);
		Prescaler = CNT_LF_PRE_16;
		if (estimateLF > 64000000UL)
			AliasingWarning = 1;
		break;
	case COUNTER_RANGE_128MHz:
		counter_SelectInput(CNT_IN_LF, CNT_LF_PRE_32);
		Prescaler = CNT_LF_PRE_32;
		if (estimateLF > 128000000UL)
			AliasingWarning = 1;
		break;
//	case COUNTER_RANGE_256MHz:
//		counter_SelectInput(CNT_IN_LF, CNT_LF_PRE_64);
//		Prescaler = CNT_TTL_PRE_1;
//		if (estimateLF > 256000000UL)
//			AliasingWarning = 1;
//		break;
	}
	if (Prescaler == 0) {
		// shouldn't happen -> abort
		return 0;
	}
	// we have a range -> get measurement
	if (range == COUNTER_RANGE_AUTO) {
		measurement = cnt_MeasureFrequency(COUNTER_DEF_PRECISION,
				estimate / Prescaler) * Prescaler;
	} else {
		// manual range
		if (range == COUNTER_RANGE_4MHz_TTL) {
			// no prescaler -> just measure frequency
			measurement = cnt_MeasureFrequency(COUNTER_DEF_PRECISION,
					estimateTTL);
		} else {
			// LF input selected
			measurement = cnt_MeasureFrequency(COUNTER_DEF_PRECISION,
					estimateLF / Prescaler) * Prescaler;
		}
	}
	return measurement;
}