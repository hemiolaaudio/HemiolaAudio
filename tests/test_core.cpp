// Hemiola Audio: tests for the logic cores. Plain C++11, no Rack needed.
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include "../core/TalaTable.hpp"
#include "../core/CycleEngine.hpp"
#include "../core/GatiEngine.hpp"
#include "../core/Modulation.hpp"

using namespace hemiola;

static int failures = 0;
static int checks = 0;

#define CHECK(cond) do { checks++; if (!(cond)) { failures++; std::printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)
#define CHECK_EQ(a, b) do { checks++; long _a = (long)(a); long _b = (long)(b); if (_a != _b) { failures++; std::printf("FAIL line %d: %s == %s (%ld vs %ld)\n", __LINE__, #a, #b, _a, _b); } } while (0)

static const CycleDef& def(const std::string& name) {
	return cycleTable()[findCycle(name)];
}

static void testTable() {
	const std::vector<CycleDef>& t = cycleTable();
	CHECK_EQ((long) t.size(), 13 + 7 + 35);
	for (size_t i = 0; i < t.size(); i++) {
		CHECK(t[i].beats > 0);
		CHECK(t[i].beats <= CycleEngine::kMaxBeats);
		CHECK((int) t[i].sections.size() <= CycleEngine::kMaxSections);
		CHECK_EQ(sumOf(t[i].sections), t[i].beats);
		for (size_t s = 0; s < t[i].sections.size(); s++)
			CHECK(t[i].sections[s] > 0);
		CHECK(!t[i].source.empty());
		// names are unique
		for (size_t j = i + 1; j < t.size(); j++)
			CHECK(t[i].name != t[j].name);
	}

	// Hindustani: expected values typed separately from the table (published values).
	CHECK_EQ(def("Tintal").beats, 16);
	CHECK(def("Tintal").sections == std::vector<int>({4, 4, 4, 4}));
	CHECK_EQ(def("Keherwa").beats, 8);
	CHECK(def("Keherwa").sections == std::vector<int>({4, 4}));
	CHECK_EQ(def("Dadra").beats, 6);
	CHECK(def("Dadra").sections == std::vector<int>({3, 3}));
	CHECK_EQ(def("Rupak").beats, 7);
	CHECK(def("Rupak").sections == std::vector<int>({3, 2, 2}));
	CHECK_EQ(def("Jhaptal").beats, 10);
	CHECK(def("Jhaptal").sections == std::vector<int>({2, 3, 2, 3}));
	CHECK_EQ(def("Ektal").beats, 12);
	CHECK(def("Ektal").sections == std::vector<int>({2, 2, 2, 2, 2, 2}));
	CHECK_EQ(def("Dhamar").beats, 14);
	CHECK(def("Dhamar").sections == std::vector<int>({5, 2, 3, 4}));

	// Carnatic: totals per jati (3, 4, 5, 7, 9) from the published table.
	const char* names[7] = {"Eka", "Rupaka", "Triputa", "Matya", "Jhampa", "Ata", "Dhruva"};
	const int totals[7][5] = {
		{3, 4, 5, 7, 9},
		{5, 6, 7, 9, 11},
		{7, 8, 9, 11, 13},
		{8, 10, 12, 16, 20},
		{6, 7, 8, 10, 12},
		{10, 12, 14, 18, 22},
		{11, 14, 17, 23, 29}
	};
	const int jatis[5] = {3, 4, 5, 7, 9};
	for (int r = 0; r < 7; r++) {
		for (int j = 0; j < 5; j++) {
			std::string n = std::string(names[r]) + " " + std::to_string(jatis[j]);
			if (r == 2 && jatis[j] == 4)
				n = "Adi";
			CHECK_EQ(def(n).beats, totals[r][j]);
		}
	}
	// Adi is 8 beats grouped 4+2+2. Rupaka (4) is 2+4.
	CHECK(def("Adi").sections == std::vector<int>({4, 2, 2}));
	CHECK(def("Rupaka 4").sections == std::vector<int>({2, 4}));
	CHECK(def("Jhampa 4").sections == std::vector<int>({4, 1, 2}));
	CHECK(def("Matya 4").sections == std::vector<int>({4, 2, 4}));
	CHECK(def("Dhruva 4").sections == std::vector<int>({4, 2, 4, 4}));
	CHECK(def("Ata 4").sections == std::vector<int>({4, 4, 2, 2}));
}

static void testEngineSequence() {
	CycleEngine e;
	e.requestCycle(findCycle("Tintal"));
	e.reset();
	CHECK_EQ(e.beatCount(), 16);
	CHECK_EQ(e.lastFiredBeat(), -1);
	for (int cycle = 0; cycle < 3; cycle++) {
		for (int b = 0; b < 16; b++) {
			CycleTick t = e.tick();
			CHECK_EQ(t.beat, b);
			CHECK_EQ(t.cycleNumber, cycle);
			CHECK(t.cycleStart == (b == 0));
			CHECK(t.cycleEnd == (b == 15));
			CHECK(t.sectionStart == (b % 4 == 0));
			CHECK_EQ(t.section, b / 4);
		}
	}
	CHECK_EQ(e.lastFiredBeat(), 15);
}

static void testEngineSections() {
	CycleEngine e;
	e.requestCycle(findCycle("Jhaptal"));
	e.reset();
	const int starts[10] = {1, 0, 1, 0, 0, 1, 0, 1, 0, 0};
	for (int b = 0; b < 10; b++) {
		CycleTick t = e.tick();
		CHECK_EQ(t.sectionStart ? 1 : 0, starts[b]);
	}
	// Rupak 3+2+2
	e.requestCycle(findCycle("Rupak"));
	e.reset();
	const int rs[7] = {1, 0, 0, 1, 0, 1, 0};
	for (int b = 0; b < 7; b++) {
		CycleTick t = e.tick();
		CHECK_EQ(t.sectionStart ? 1 : 0, rs[b]);
	}
}

static void testEngineQueue() {
	CycleEngine e;
	e.requestCycle(findCycle("Tintal"));
	e.reset();
	for (int b = 0; b < 5; b++)
		e.tick();  // mid-cycle, beat index 4 fired
	e.requestCycle(findCycle("Keherwa"));
	CHECK_EQ(e.activeIndex(), findCycle("Tintal"));  // not applied yet
	CHECK_EQ(e.pendingIndex(), findCycle("Keherwa"));
	CHECK_EQ(e.beatCount(), 16);
	// finish the Tintal cycle: beats 5..15
	for (int b = 5; b < 16; b++) {
		CycleTick t = e.tick();
		CHECK_EQ(t.beat, b);
		CHECK_EQ(e.beatCount(), 16);
	}
	// next tick starts Keherwa
	CycleTick t = e.tick();
	CHECK_EQ(t.beat, 0);
	CHECK(t.cycleStart);
	CHECK_EQ(e.beatCount(), 8);
	CHECK_EQ(e.activeIndex(), findCycle("Keherwa"));
	// Keherwa now loops over 8 beats
	for (int b = 1; b < 8; b++)
		CHECK_EQ(e.tick().beat, b);
	CHECK_EQ(e.tick().beat, 0);

	// Immediate change when waiting at the start of a cycle.
	CycleEngine f;
	f.requestCycle(findCycle("Dadra"));
	CHECK_EQ(f.activeIndex(), findCycle("Dadra"));
	CHECK_EQ(f.beatCount(), 6);
	// Reset applies a pending change straight away, even mid-cycle.
	f.tick();
	f.tick();
	f.requestCycle(findCycle("Ektal"));
	CHECK_EQ(f.beatCount(), 6);
	f.reset();
	CHECK_EQ(f.beatCount(), 12);
	CHECK_EQ(f.tick().beat, 0);

	// Out-of-range requests are clamped, never crash.
	CycleEngine g;
	g.requestCycle(-5);
	CHECK_EQ(g.pendingIndex(), 0);
	g.requestCycle(100000);
	CHECK_EQ(g.pendingIndex(), cycleCount() - 1);
	g.reset();
	for (int i = 0; i < 100; i++)
		g.tick();
}

static void testEveryCycleRuns() {
	for (int i = 0; i < cycleCount(); i++) {
		CycleEngine e;
		e.requestCycle(i);
		e.reset();
		int n = e.beatCount();
		const CycleDef& d = cycleTable()[i];
		CHECK_EQ(n, d.beats);
		int sectionStarts = 0;
		for (int cycle = 0; cycle < 2; cycle++) {
			for (int b = 0; b < n; b++) {
				CycleTick t = e.tick();
				CHECK_EQ(t.beat, b);
				if (t.sectionStart) sectionStarts++;
			}
		}
		CHECK_EQ(sectionStarts, 2 * (int) d.sections.size());
	}
}

static void testSwingAndTempo() {
	const float sr = 48000.f;
	// 120 BPM, swing 0.5: a beat lasts 24000 samples. Delay = 0.5 * 0.5 beat = 6000 samples.
	{
		InternalClock c;
		std::vector<long> at;
		for (long i = 0; i < 24000L * 8; i++)
			if (c.process(120.f, 0.5f, 1.f / sr)) at.push_back(i);
		CHECK_EQ((long) at.size(), 8);
		CHECK_EQ(at[0], 0);
		CHECK(std::labs(at[1] - 30000) <= 1);   // delayed beat
		CHECK(std::labs(at[2] - 48000) <= 1);   // the pair still lasts exactly two beats
		CHECK(std::labs(at[3] - 78000) <= 1);
		CHECK(std::labs(at[4] - 96000) <= 1);
	}
	// Swing 0 behaves exactly like the old straight clock.
	{
		InternalClock a, b;
		for (long i = 0; i < 100000; i++)
			CHECK(a.process(133.f, 0.f, 1.f / sr) == b.process(133.f, 1.f / sr));
	}
	// Swing out of range is clamped. Full swing delays by half a beat and never fires twice at once.
	{
		InternalClock c;
		long n = 0;
		for (long i = 0; i < 48000L * 4; i++)
			if (c.process(120.f, 9.f, 1.f / sr)) n++;
		CHECK_EQ(n, 8);
	}
	// A restart begins again with the straight (first) beat.
	{
		InternalClock c;
		for (long i = 0; i < 30000; i++) c.process(120.f, 0.5f, 1.f / sr);
		c.restart();
		CHECK(c.process(120.f, 0.5f, 1.f / sr));
	}
	// Tempo CV is 1 V per octave and stays between 20 and 400 BPM.
	CHECK(std::fabs(tempoWithCv(120.f, 0.f) - 120.f) < 1e-3f);
	CHECK(std::fabs(tempoWithCv(120.f, 1.f) - 240.f) < 1e-2f);
	CHECK(std::fabs(tempoWithCv(120.f, -1.f) - 60.f) < 1e-2f);
	CHECK(std::fabs(tempoWithCv(120.f, 5.f) - 400.f) < 1e-3f);
	CHECK(std::fabs(tempoWithCv(30.f, -5.f) - 20.f) < 1e-3f);
	// Part probability: the beat part is never dropped.
	CHECK(partPasses(0.f, 0.99f, true));
	CHECK(!partPasses(0.f, 0.0f, false));
	CHECK(partPasses(1.f, 0.9999f, false));
	CHECK(partPasses(0.5f, 0.49f, false));
	CHECK(!partPasses(0.5f, 0.5f, false));
	{
		unsigned x = 12345u;
		int hits = 0;
		for (int i = 0; i < 20000; i++) {
			x = x * 1664525u + 1013904223u;
			float r = (float) (x >> 8) / 16777216.f;
			if (partPasses(0.3f, r, false)) hits++;
		}
		CHECK(hits > 5600 && hits < 6400);
	}
	// Level CV on the divider: 0 to 10 V sweeps about the whole 1 to 64 range.
	CHECK_EQ(levelChoice(1.f, 0.f, 6.3f, 1, 64), 1);
	CHECK_EQ(levelChoice(1.f, 10.f, 6.3f, 1, 64), 64);
	CHECK_EQ(levelChoice(32.f, -10.f, 6.3f, 1, 64), 1);
}

static void testInternalClock() {
	InternalClock c;
	const float sr = 48000.f;
	int ticks = 0;
	std::vector<long> at;
	for (long i = 0; i < (long) (sr * 10); i++) {
		if (c.process(120.f, 1.f / sr)) {
			ticks++;
			at.push_back(i);
		}
	}
	// 120 BPM for 10 seconds (480000 samples): beats at 0, 24000, ... 456000 = 20 beats.
	CHECK_EQ(ticks, 20);
	CHECK_EQ(at[0], 0);
	for (size_t i = 1; i < at.size(); i++) {
		long gap = at[i] - at[i - 1];
		CHECK(gap >= 23999 && gap <= 24001);
	}
	// Restart fires on the very next sample.
	InternalClock d;
	d.process(60.f, 1.f / sr);
	for (int i = 0; i < 100; i++) d.process(60.f, 1.f / sr);
	d.restart();
	CHECK(d.process(60.f, 1.f / sr));
	// Extreme tempo never fires more than once per call.
	InternalClock e;
	for (int i = 0; i < 1000; i++) e.process(1000000.f, 1.f);
}

// Run the Gati engine over a train of beat edges and return the sample positions of the parts.
static std::vector<long> runGati(GatiEngine& g, const std::vector<long>& edges, long total, std::vector<int>* firsts = nullptr) {
	std::vector<long> out;
	size_t ei = 0;
	for (long s = 0; s < total; s++) {
		bool edge = (ei < edges.size() && edges[ei] == s);
		if (edge) ei++;
		bool first = false;
		if (g.process(edge, first)) {
			out.push_back(s);
			if (firsts) firsts->push_back(first ? 1 : 0);
		}
	}
	return out;
}

static void testDivider() {
	// Every 3rd part, starting with the first.
	{
		PartDivider d;
		d.setDivide(3);
		std::vector<int> got;
		for (int i = 0; i < 9; i++) got.push_back(d.process(true) ? 1 : 0);
		CHECK(got == std::vector<int>({1, 0, 0, 1, 0, 0, 1, 0, 0}));
	}
	// Samples without a part never count.
	{
		PartDivider d;
		d.setDivide(2);
		CHECK(!d.process(false));
		CHECK(d.process(true));
		CHECK(!d.process(false));
		CHECK(!d.process(true));
		CHECK(d.process(true));
	}
	// Divide by 1 passes everything.
	{
		PartDivider d;
		d.setDivide(1);
		for (int i = 0; i < 5; i++) CHECK(d.process(true));
	}
	// Range is clamped, and /64 fires once in 64 parts.
	{
		PartDivider d;
		d.setDivide(1000);
		CHECK_EQ(d.divideBy(), 64);
		d.setDivide(-5);
		CHECK_EQ(d.divideBy(), 1);
		d.setDivide(64);
		int fired = 0;
		for (int i = 0; i < 640; i++) fired += d.process(true) ? 1 : 0;
		CHECK_EQ(fired, 10);
	}
	// A new N restarts on the next part; the same N does not disturb the count.
	{
		PartDivider d;
		d.setDivide(4);
		CHECK(d.process(true));
		CHECK(!d.process(true));
		d.setDivide(4);
		CHECK(!d.process(true));
		d.setDivide(2);
		CHECK(d.process(true));
		CHECK(!d.process(true));
		CHECK(d.process(true));
	}
	// Reset restarts the count.
	{
		PartDivider d;
		d.setDivide(3);
		d.process(true);
		d.process(true);
		d.reset();
		CHECK(d.process(true));
	}
	// Chained with the nadai engine: Sankirna (9) divided by 9 gives exactly one pulse per beat, on the beat.
	{
		GatiEngine g;
		g.setDivision(9);
		g.reset();
		PartDivider d;
		d.setDivide(9);
		std::vector<long> edges;
		for (int i = 0; i < 4; i++) edges.push_back(i * 900);
		size_t ei = 0;
		std::vector<long> passed;
		for (long s = 0; s < 3600; s++) {
			bool edge = (ei < edges.size() && edges[ei] == s);
			if (edge) ei++;
			bool first = false;
			bool part = g.process(edge, first);
			if (d.process(part)) passed.push_back(s);
			if (part && !g.hasTempo()) d.reset();  // the learning beat does not count
		}
		CHECK_EQ((long) passed.size(), 4);
		CHECK_EQ(passed[0], 0);
		CHECK_EQ(passed[1], 900);
		CHECK_EQ(passed[3], 2700);
	}
}

static void testGati() {
	// Division 4, beats every 100 samples.
	{
		GatiEngine g;
		g.reset();
		g.setDivision(4);
		g.reset();
		std::vector<long> edges;
		for (int i = 0; i < 6; i++) edges.push_back(i * 100);
		std::vector<int> firsts;
		std::vector<long> out = runGati(g, edges, 600, &firsts);
		// Beat 0 has no tempo yet: only the on-beat part. Then 4 parts per beat.
		CHECK_EQ((long) out.size(), 1 + 5 * 4 + 0);
		CHECK_EQ(out[0], 0);
		// beat 1 (edge at 100): parts at 100, 125, 150, 175
		CHECK_EQ(out[1], 100);
		CHECK_EQ(out[2], 125);
		CHECK_EQ(out[3], 150);
		CHECK_EQ(out[4], 175);
		CHECK_EQ(out[5], 200);
		CHECK_EQ(firsts[1], 1);
		CHECK_EQ(firsts[2], 0);
		CHECK_EQ(firsts[5], 1);
	}
	// Division 7: 7 parts per beat, evenly spaced to within one sample.
	{
		GatiEngine g;
		g.setDivision(7);
		g.reset();
		std::vector<long> edges;
		for (int i = 0; i < 4; i++) edges.push_back(i * 700);
		std::vector<long> out = runGati(g, edges, 2800);
		// edge 0: 1 part. edges 1..3: 7 parts each, except the last beat is cut by the end of the run
		CHECK_EQ((long) out.size(), 1 + 7 * 3);
		for (int k = 0; k < 7; k++)
			CHECK_EQ(out[1 + k], 700 + k * 100);
	}
	// Division 1: only the on-beat part.
	{
		GatiEngine g;
		g.setDivision(1);
		g.reset();
		std::vector<long> edges;
		for (int i = 0; i < 5; i++) edges.push_back(i * 100);
		std::vector<long> out = runGati(g, edges, 500);
		CHECK_EQ((long) out.size(), 5);
	}
	// A new division starts only on the next beat.
	{
		GatiEngine g;
		g.setDivision(2);
		g.reset();
		std::vector<long> edges;
		for (int i = 0; i < 5; i++) edges.push_back(i * 100);
		size_t ei = 0;
		std::vector<long> out;
		for (long s = 0; s < 500; s++) {
			if (s == 160) g.setDivision(4);  // mid-beat change
			bool edge = (ei < edges.size() && edges[ei] == s);
			if (edge) ei++;
			bool first = false;
			if (g.process(edge, first)) out.push_back(s);
		}
		// beats: 0 -> 1 part, 100 -> 2 (100, 150), 200 -> 2? change applied at beat 200 edge
		// so the beat from 100 stays at 2 parts, beats from 200 use 4 parts.
		CHECK_EQ(out[0], 0);
		CHECK_EQ(out[1], 100);
		CHECK_EQ(out[2], 150);
		CHECK_EQ(out[3], 200);
		CHECK_EQ(out[4], 225);
		CHECK_EQ(out[5], 250);
		CHECK_EQ(out[6], 275);
		CHECK_EQ(out[7], 300);
	}
	// If the next beat arrives early, the remaining parts are dropped, none are late or doubled.
	{
		GatiEngine g;
		g.setDivision(4);
		g.reset();
		std::vector<long> edges;
		edges.push_back(0);
		edges.push_back(100);  // period 100
		edges.push_back(160);  // early beat: only parts at 100, 125, 150 fit, next beat at 160
		std::vector<long> out = runGati(g, edges, 220);
		// 0 | 100,125,150 | 160, then period 60 -> next would be due at 160+15... but only after period known (60)
		CHECK_EQ(out[0], 0);
		CHECK_EQ(out[1], 100);
		CHECK_EQ(out[2], 125);
		CHECK_EQ(out[3], 150);
		CHECK_EQ(out[4], 160);
		// last beat had period 60 known from the early edge, parts at 175, 190, 205
		CHECK_EQ(out[5], 175);
		CHECK_EQ(out[6], 190);
		CHECK_EQ(out[7], 205);
		CHECK_EQ((long) out.size(), 8);
	}
	// If the clock stops, the parts stop after one beat (never runaway).
	{
		GatiEngine g;
		g.setDivision(9);
		g.reset();
		std::vector<long> edges;
		edges.push_back(0);
		edges.push_back(90);
		std::vector<long> out = runGati(g, edges, 100000);
		CHECK_EQ((long) out.size(), 1 + 9);
	}
}

static void testModulation() {
	// LEVEL mode
	CHECK_EQ(levelChoice(4.f, 0.f, 1.f, 1, 9), 4);
	CHECK_EQ(levelChoice(4.f, 2.f, 1.f, 1, 9), 6);
	CHECK_EQ(levelChoice(4.f, 10.f, 1.f, 1, 9), 9);   // a 10 V gate is limited to 9
	CHECK_EQ(levelChoice(4.f, -10.f, 1.f, 1, 9), 1);
	CHECK_EQ(levelChoice(4.f, 10.f, 0.2f, 1, 9), 6);  // a gate with CV amount 0.2 gives knob + 2
	CHECK_EQ(levelChoice(4.f, 10.f, -0.2f, 1, 9), 2); // flipped upside down
	CHECK_EQ(levelChoice(4.f, 0.49f, 1.f, 1, 9), 4);  // rounds to the nearest whole step
	CHECK_EQ(levelChoice(4.f, 0.51f, 1.f, 1, 9), 5);
	// An LFO of +/-5 V around knob 5 sweeps the whole range 1 to 9 at amount 1 only at the extremes
	CHECK_EQ(levelChoice(5.f, -5.f, 1.f, 1, 9), 1);
	CHECK_EQ(levelChoice(5.f, 5.f, 1.f, 1, 9), 9);
	CHECK_EQ(levelChoice(5.f, 5.f, 0.4f, 1, 9), 7);
	// COUNT mode
	{
		StepCounter c;
		CHECK_EQ(c.choose(4, 1, 9), 4);
		c.advance(9);
		CHECK_EQ(c.choose(4, 1, 9), 5);
		for (int i = 0; i < 4; i++)
			c.advance(9);
		CHECK_EQ(c.choose(4, 1, 9), 9);
		c.advance(9);
		CHECK_EQ(c.choose(4, 1, 9), 1);   // after 9 it goes back to 1
		for (int i = 0; i < 9; i++)
			c.advance(9);
		CHECK_EQ(c.choose(4, 1, 9), 1);   // nine pulses make a full lap, back to the same value
		c.reset();
		CHECK_EQ(c.choose(7, 1, 9), 7);
		// cycle list of 55 starting at index 0
		StepCounter d;
		for (int i = 0; i < 54; i++)
			d.advance(55);
		CHECK_EQ(d.choose(0, 0, 55), 54);
		d.advance(55);
		CHECK_EQ(d.choose(0, 0, 55), 0);
	}
	// Gate length
	CHECK(std::fabs(gateSeconds(0.5f, 0.5f, 0.1f) - 0.25f) < 1e-6f);
	CHECK(std::fabs(gateSeconds(0.f, 0.5f, 0.1f) - 0.05f) < 1e-6f);
	CHECK(std::fabs(gateSeconds(0.5f, 5.f, 0.1f) - 0.475f) < 1e-6f);   // share limited to 95 %
	CHECK(gateSeconds(0.5f, 0.f, 0.1f) >= 0.002f);
	// Click: silent before the first trigger, peaks near 5 V (accent) and about 2.25 V (normal), ends within 0.1 s
	{
		ClickSynth k;
		float st = 1.f / 48000.f;
		CHECK(k.process(st) == 0.f);
		float peakA = 0.f, peakN = 0.f;
		k.trigger(true);
		for (int i = 0; i < 4800; i++)
			peakA = std::fmax(peakA, std::fabs(k.process(st)));
		k.trigger(false);
		for (int i = 0; i < 4800; i++)
			peakN = std::fmax(peakN, std::fabs(k.process(st)));
		CHECK(peakA > 4.0f && peakA <= 5.0f);
		CHECK(peakN > 1.5f && peakN < 2.5f);
		CHECK(peakA > 1.8f * peakN);
		for (int i = 0; i < 480; i++)
			k.process(st);
		CHECK(!k.isActive());
		CHECK(k.process(st) == 0.f);
	}
}

int main() {
	testTable();
	testEngineSequence();
	testEngineSections();
	testEngineQueue();
	testEveryCycleRuns();
	testInternalClock();
	testSwingAndTempo();
	testGati();
	testDivider();
	testModulation();
	std::printf("%d checks, %d failures\n", checks, failures);
	return failures ? 1 : 0;
}
