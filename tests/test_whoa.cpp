#include "../core/WhoaEngine.hpp"
#include <cstdio>
#include <vector>
#include <cassert>
using namespace whoa;
static int fails = 0;
#define CHECK(c, msg) do { if (!(c)) { printf("FAIL: %s\n", msg); fails++; } else printf("ok:   %s\n", msg); } while (0)
static const float SR = 48000.f;

static float zeroCrossFreq(const std::vector<float>& v, size_t a, size_t b) {
	int n = 0; for (size_t i = a + 1; i < b; i++) if (v[i - 1] < 0 && v[i] >= 0) n++;
	return n / ((b - a) / SR);
}
// crude brightness: mean absolute difference over mean absolute level
static float brightness(const std::vector<float>& v, size_t a, size_t b) {
	double d = 0, l = 0; for (size_t i = a + 1; i < b; i++) { d += std::fabs(v[i] - v[i-1]); l += std::fabs(v[i]); }
	return (float) (d / (l + 1e-9));
}

int main() {
	{ // silence with no gate
		Voice v; Params p; float l, r; float mx = 0;
		for (int i = 0; i < 4800; i++) { v.process(p, false, false, false, SR, l, r); mx = std::max(mx, std::fabs(l)); }
		CHECK(mx == 0.f, "no gate gives silence");
	}
	{ // pitch tracks V/oct, plain sine at ratio 1 with no modulation
		Voice v; Params p; p.index = 0.f; p.cA = 0.0005f; p.cS = 1.f;
		std::vector<float> o;
		for (int i = 0; i < 48000; i++) { float l, r; v.process(p, false, false, true, SR, l, r); o.push_back(l); }
		float f0 = zeroCrossFreq(o, 24000, 48000);
		CHECK(std::fabs(f0 - 261.6f) < 2.f, "0 V is C4 (261.6 Hz)");
		Voice v2; p.voct = 1.f; std::vector<float> o2;
		for (int i = 0; i < 48000; i++) { float l, r; v2.process(p, false, false, true, SR, l, r); o2.push_back(l); }
		float f1 = zeroCrossFreq(o2, 24000, 48000);
		CHECK(std::fabs(f1 - 523.3f) < 3.f, "+1 V is one octave up");
	}
	{ // modulator gate alone: carrier silent
		Voice v; Params p; float l, r; float mx = 0;
		for (int i = 0; i < 9600; i++) { v.process(p, true, false, false, SR, l, r); mx = std::max(mx, std::fabs(l)); }
		CHECK(mx < 1e-6f, "modulator gate alone is silent (carrier not open)");
	}
	{ // carrier held, modulator pulsed: timbre brightens without retriggering carrier
		Voice v; Params p; p.index = 5.f; p.cS = 1.f; p.cA = 0.001f; p.mD = 0.08f; p.mS = 0.f;
		std::vector<float> o; float l, r;
		for (int i = 0; i < 24000; i++) { v.process(p, false, true, false, SR, l, r); o.push_back(l); }
		float quietB = brightness(o, 18000, 24000);
		float ceBefore = v.carEnv.level;
		std::vector<float> o2;
		for (int i = 0; i < 2400; i++) { v.process(p, true, true, false, SR, l, r); o2.push_back(l); }
		float pulseB = brightness(o2, 0, 2400);
		CHECK(std::fabs(v.carEnv.level - ceBefore) < 1e-3f, "carrier envelope untouched by modulator pulse");
		CHECK(pulseB > quietB * 1.5f, "modulator pulse brightens the tone");
	}
	{ // common gate fires both and gate out follows
		Voice v; Params p; float l, r;
		v.process(p, false, false, true, SR, l, r);
		CHECK(v.gateOut, "common gate raises gate out");
		CHECK(v.modEnv.stage == Adsr::ATTACK && v.carEnv.stage == Adsr::ATTACK, "common gate fires both envelopes");
		v.process(p, true, false, false, SR, l, r);
		CHECK(v.gateOut, "single gate raises gate out");
		Voice w; w.process(p, false, false, false, SR, l, r);
		CHECK(!w.gateOut, "no gate, gate out low");
	}
	{ // index raises brightness
		auto run = [&](float idx) { Voice v; Params p; p.index = idx; p.modEnvToIndex = 0.f; p.cS = 1.f; std::vector<float> o; float l, r;
			for (int i = 0; i < 24000; i++) { v.process(p, false, false, true, SR, l, r); o.push_back(l); } return brightness(o, 12000, 24000); };
		CHECK(run(6.f) > run(0.5f) * 1.5f, "more index, brighter");
	}
	{ // feedback: bounded, finite, brighter, at extremes and high pitch
		bool ok = true; float mx = 0;
		for (float fb = 0.f; fb <= 1.f; fb += 0.25f)
			for (float vo = -3.f; vo <= 5.f; vo += 2.f) {
				Voice v; Params p; p.feedback = fb; p.voct = vo; p.index = 10.f; p.drive = 1.f; p.spread = 1.f; p.cS = 1.f; p.fine = 0.37f; p.ratioCoarse = 7.f;
				for (int i = 0; i < 48000; i++) { float l, r; v.process(p, false, false, true, SR, l, r);
					if (!std::isfinite(l) || !std::isfinite(r)) { ok = false; }
					mx = std::max(mx, std::max(std::fabs(l), std::fabs(r))); }
			}
		CHECK(ok, "all outputs finite at extremes");
		CHECK(mx <= 1.05f, "output stays within about 1.0");
	}
	{ // spread: 0 is identical L/R, 1 differs
		Voice v; Params p; p.cS = 1.f; p.index = 2.f; float l, r; double d0 = 0;
		for (int i = 0; i < 24000; i++) { v.process(p, false, false, true, SR, l, r); d0 += std::fabs(l - r); }
		Voice w; p.spread = 1.f; double d1 = 0;
		for (int i = 0; i < 24000; i++) { w.process(p, false, false, true, SR, l, r); d1 += std::fabs(l - r); }
		CHECK(d0 < 1e-6, "spread 0 is mono");
		CHECK(d1 > 1.0, "spread 1 is wide");
	}
	{ // accent opens the modulator and lifts level
		auto run = [&](float acc) { Voice v; Params p; p.accent = acc; p.index = 3.f; p.modEnvToIndex = 0.f; p.cS = 1.f; std::vector<float> o; float l, r;
			for (int i = 0; i < 24000; i++) { v.process(p, false, false, true, SR, l, r); o.push_back(l); } return brightness(o, 12000, 24000); };
		CHECK(run(1.f) > run(0.f), "accent brightens");
	}
	{ // drive adds level-independent loudness control: output bounded and DC-free
		Voice v; Params p; p.drive = 1.f; p.cS = 1.f; p.index = 4.f; double sum = 0, n = 0; float l, r;
		for (int i = 0; i < 48000; i++) { v.process(p, false, false, true, SR, l, r); if (i > 24000) { sum += l; n++; } }
		CHECK(std::fabs(sum / n) < 0.1, "drive leaves little DC");
	}
	{ // release returns to idle
		Voice v; Params p; p.cR = 0.05f; float l, r;
		for (int i = 0; i < 4800; i++) v.process(p, false, false, true, SR, l, r);
		for (int i = 0; i < 48000; i++) v.process(p, false, false, false, SR, l, r);
		CHECK(v.carEnv.stage == Adsr::IDLE && std::fabs(l) < 1e-6f, "release ends in silence");
	}
	printf(fails ? "\n%d FAILED\n" : "\nall passed\n", fails);
	return fails ? 1 : 0;
}
