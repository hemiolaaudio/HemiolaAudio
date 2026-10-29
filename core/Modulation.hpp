// Hemiola Audio: small helpers shared by the modules. No Rack code in here.
#pragma once
#include <cmath>

namespace hemiola {

inline int roundToInt(float x) {
	return (int) std::floor(x + 0.5f);
}

inline int clampInt(int v, int lo, int hi) {
	return v < lo ? lo : (v > hi ? hi : v);
}

// LEVEL mode: the knob sets a starting value, the CV moves it by (cv * amount) steps.
// The result is rounded to a whole step and kept between lo and hi.
inline int levelChoice(float knob, float cv, float amount, int lo, int hi) {
	return clampInt(roundToInt(knob + cv * amount), lo, hi);
}

// COUNT mode: every pulse moves one step further, and after the last step it goes back to the first.
class StepCounter {
public:
	void reset() { offset = 0; }
	void advance(int span) {
		if (span < 1)
			span = 1;
		offset = (offset % span + 1) % span;
	}
	// knobValue is where the knob points (lo..lo+span-1). Returns the value after counting.
	int choose(int knobValue, int lo, int span) const {
		if (span < 1)
			span = 1;
		int rel = clampInt(knobValue - lo, 0, span - 1);
		return lo + (rel + offset % span) % span;
	}
	int steps() const { return offset; }

private:
	int offset = 0;
};

// Length of a gate in seconds: a share of a period (a beat, or one part of a beat).
// If the period is not known yet, the fallback is used.
inline float gateSeconds(float period, float share, float fallback) {
	if (share < 0.05f)
		share = 0.05f;
	if (share > 0.95f)
		share = 0.95f;
	float s = (period > 0.f ? period : fallback) * share;
	return s < 0.002f ? 0.002f : s;
}

// Probability gate for one part. r is a random number from 0 up to (not including) 1.
// The part on the beat (anchor) is never dropped, so the beat always stays in place.
inline bool partPasses(float probability, float r, bool anchor) {
	return anchor || r < probability;
}

// A short sine "tick" for counting by ear. The accented tick is higher and louder.
class ClickSynth {
public:
	void trigger(bool accent) {
		freq = accent ? 1800.f : 1100.f;
		amp = accent ? 1.f : 0.45f;
		phase = 0.f;
		t = 0.f;
		active = true;
	}
	// Returns audio in volts (peak 5 V).
	float process(float sampleTime) {
		if (!active)
			return 0.f;
		float env = amp * std::exp(-t / 0.012f);
		float out = env * std::sin(6.2831853f * phase);
		phase += freq * sampleTime;
		if (phase >= 1.f)
			phase -= 1.f;
		t += sampleTime;
		if (t > 0.1f)
			active = false;
		return 5.f * out;
	}
	bool isActive() const { return active; }

private:
	float freq = 1100.f, amp = 0.f, phase = 0.f, t = 0.f;
	bool active = false;
};

}  // namespace hemiola
