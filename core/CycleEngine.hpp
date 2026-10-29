// Hemiola Audio: the cycle counter and the internal beat clock used by Cycle Clock.
// No Rack code here, so the logic can be tested with a plain compiler.
#pragma once
#include <cmath>
#include "TalaTable.hpp"

namespace hemiola {

struct CycleTick {
	int beat = 0;            // index of the beat that just fired, 0 = first beat of the cycle
	int section = 0;         // index of the section that beat belongs to
	bool cycleStart = false; // true on the first beat of the cycle
	bool sectionStart = false; // true on the first beat of every section
	bool cycleEnd = false;   // true on the last beat of the cycle
	long cycleNumber = 0;    // 0 for the first cycle after a reset
};

class CycleEngine {
public:
	static const int kMaxBeats = 64;
	static const int kMaxSections = 16;

	CycleEngine() {
		applyCycle(0);
		pending = 0;
		nextBeat = 0;
		lastBeat = -1;
		cycleNumber = -1;
	}

	// Ask for a different cycle. It takes effect on the first beat of the next cycle,
	// or straight away if the engine is waiting at the start of a cycle.
	void requestCycle(int index) {
		index = clampIndex(index);
		pending = index;
		if (nextBeat == 0)
			applyCycle(pending);
	}

	// After a reset the next tick is the first beat of the cycle.
	void reset() {
		nextBeat = 0;
		lastBeat = -1;
		cycleNumber = -1;
		applyCycle(pending);
	}

	CycleTick tick() {
		if (nextBeat == 0 && active != pending)
			applyCycle(pending);
		CycleTick t;
		t.beat = nextBeat;
		t.section = beatSection[nextBeat];
		t.cycleStart = (nextBeat == 0);
		t.sectionStart = (nextBeat == sectionFirstBeat[t.section]);
		t.cycleEnd = (nextBeat == beats - 1);
		if (t.cycleStart)
			cycleNumber++;
		t.cycleNumber = cycleNumber;
		lastBeat = nextBeat;
		nextBeat = (nextBeat + 1) % beats;
		return t;
	}

	int activeIndex() const { return active; }
	int pendingIndex() const { return pending; }
	int beatCount() const { return beats; }
	int sectionCount() const { return numSections; }
	int lastFiredBeat() const { return lastBeat; }  // -1 before the first tick
	int sectionLength(int s) const { return sectionLen[s]; }

private:
	int active = 0;
	int pending = 0;
	int beats = 1;
	int numSections = 1;
	int nextBeat = 0;
	int lastBeat = -1;
	long cycleNumber = -1;
	int beatSection[kMaxBeats];
	int sectionFirstBeat[kMaxSections];
	int sectionLen[kMaxSections];

	static int clampIndex(int i) {
		int n = cycleCount();
		if (i < 0) return 0;
		if (i >= n) return n - 1;
		return i;
	}

	void applyCycle(int index) {
		index = clampIndex(index);
		const CycleDef& d = cycleTable()[index];
		active = index;
		beats = d.beats;
		if (beats > kMaxBeats) beats = kMaxBeats;
		numSections = (int) d.sections.size();
		if (numSections > kMaxSections) numSections = kMaxSections;
		int b = 0;
		for (int s = 0; s < numSections; s++) {
			sectionFirstBeat[s] = b;
			sectionLen[s] = d.sections[s];
			for (int k = 0; k < d.sections[s] && b < kMaxBeats; k++)
				beatSection[b++] = s;
		}
	}
};

// A simple beat clock. Call process() once per sample. restart() makes the next
// call fire a beat straight away.
class InternalClock {
public:
	// The next call fires a beat.
	void restart() {
		remaining = 0.0;
		evenNext = false;
	}
	// swing is 0 to 1. Every second beat is delayed by swing * half a beat, so 0 is straight time
	// and about 0.67 is triplet swing. The pair of beats always lasts exactly two beats.
	bool process(float bpm, float swing, float sampleTime) {
		if (swing < 0.f) swing = 0.f;
		if (swing > 1.f) swing = 1.f;
		remaining -= (double) bpm / 60.0 * (double) sampleTime;
		if (remaining > 0.0)
			return false;
		double d = 0.5 * (double) swing;
		double gap = evenNext ? 1.0 - d : 1.0 + d;  // gap until the beat after this one
		evenNext = !evenNext;
		remaining += gap;
		if (remaining <= 0.0)
			remaining = gap;  // guard against a huge step
		return true;
	}
	bool process(float bpm, float sampleTime) { return process(bpm, 0.f, sampleTime); }

private:
	double remaining = 0.0;  // beats until the next beat. The first call fires a beat.
	bool evenNext = false;   // true when the next beat to fire is the delayed one
};

// Tempo from the knob (BPM) and a 1 V per octave CV. The result stays between 20 and 400 BPM.
inline float tempoWithCv(float knobBpm, float volts) {
	float bpm = knobBpm * std::exp2(volts);
	return bpm < 20.f ? 20.f : (bpm > 400.f ? 400.f : bpm);
}

}  // namespace hemiola
