#include "../core/WhoaFx.hpp"
#include <cstdio>
#include <cstdlib>
#include <vector>
using namespace whoa;
static int fails = 0;
#define CHECK(c, msg) do { if (!(c)) { printf("FAIL: %s\n", msg); fails++; } else printf("ok:   %s\n", msg); } while (0)
static const float SR = 48000.f;

// beat pulse train: 10 V for 5 ms at the given period
static float pulse(long i, float beatSec) {
	long per = (long) (beatSec * SR);
	return ((i % per) < (long) (0.005f * SR)) ? 10.f : 0.f;
}
static float rmsRange(const std::vector<float>& v, size_t a, size_t b) {
	double s = 0; for (size_t i = a; i < b; i++) s += v[i] * v[i];
	return (float) std::sqrt(s / (double) (b - a));
}

int main() {
	{ // clock meter measures 120 BPM and 131 BPM
		ClockMeter c; c.init(SR); float per = 0;
		for (long i = 0; i < (long) (4 * SR); i++) per = c.process(pulse(i, 0.5f));
		CHECK(std::fabs(per - 0.5f) < 0.001f, "clock meter reads 120 BPM");
		c.init(SR);
		float beat = 60.f / 131.f;
		for (long i = 0; i < (long) (4 * SR); i++) per = c.process(pulse(i, beat));
		CHECK(std::fabs(per - beat) < 0.001f, "clock meter reads 131 BPM");
	}
	{ // falls back to 120 BPM with no clock
		ClockMeter c; c.init(SR); float per = 0;
		for (long i = 0; i < (long) (3 * SR); i++) per = c.process(0.f);
		CHECK(std::fabs(per - 0.5f) < 1e-6f, "no clock falls back to 120 BPM");
	}
	{ // delay echo lands on the division: 1/8 at 120 BPM is 250 ms
		Fx fx; fx.init(SR); FxParams p; p.delayDiv = 7; p.delayMix = 1.f; p.delayFb = 0.f; p.revMix = 0.f;
		std::vector<float> o; float l, r;
		for (long i = 0; i < (long) (2 * SR); i++) {
			float x = (i == 48000) ? 1.f : 0.f;
			fx.process(x, x, pulse(i, 0.5f), p, l, r); o.push_back(l + r);
		}
		size_t pk = 0; float mx = 0;
		for (size_t i = 49000; i < o.size(); i++) if (std::fabs(o[i]) > mx) { mx = std::fabs(o[i]); pk = i; }
		float ms = (pk - 48000) / SR * 1000.f;
		CHECK(std::fabs(ms - 250.f) < 2.f, "1/8 echo arrives at 250 ms (120 BPM)");
	}
	{ // dotted 1/4 at 131 BPM
		Fx fx; fx.init(SR); FxParams p; p.delayDiv = 11; p.delayMix = 1.f; p.delayFb = 0.f;
		float beat = 60.f / 131.f; std::vector<float> o; float l, r;
		for (long i = 0; i < (long) (3 * SR); i++) {
			float x = (i == 72000) ? 1.f : 0.f;
			fx.process(x, x, pulse(i, beat), p, l, r); o.push_back(l + r);
		}
		size_t pk = 0; float mx = 0;
		for (size_t i = 73000; i < o.size(); i++) if (std::fabs(o[i]) > mx) { mx = std::fabs(o[i]); pk = i; }
		float ms = (pk - 72000) / SR * 1000.f;
		CHECK(std::fabs(ms - 1.5f * beat * 1000.f) < 3.f, "dotted 1/4 echo follows 131 BPM");
	}
	{ // feedback repeats decay and stay bounded at maximum
		Fx fx; fx.init(SR); FxParams p; p.delayDiv = 4; p.delayMix = 1.f; p.delayFb = 1.f;
		std::vector<float> o; float l, r;
		for (long i = 0; i < (long) (10 * SR); i++) {
			float x = (i == 100) ? 1.f : 0.f; fx.process(x, x, 0.f, p, l, r); o.push_back(l); }
		float mx = 0; for (float x : o) mx = std::max(mx, std::fabs(x));
		CHECK(mx < 1.5f && std::isfinite(mx), "delay at maximum feedback stays bounded");
		CHECK(rmsRange(o, (size_t) (9 * SR), (size_t) (10 * SR)) < rmsRange(o, 0, (size_t) (1 * SR)), "delay feedback decays");
	}
	// reverb helper: burst then tail
	struct Tail { std::vector<float> o; };
	auto run = [&](int mode, float size, float decay, float secs, bool noiseBurst) {
		Fx fx; fx.init(SR); FxParams p; p.revMode = mode; p.revSize = size; p.revDecay = decay; p.revMix = 1.f;
		Tail t; float l, r; srand(7);
		for (long i = 0; i < (long) (secs * SR); i++) {
			float x = 0.f; if (noiseBurst && i < 4800) x = 0.8f * ((rand() / (float) RAND_MAX) * 2.f - 1.f); else if (!noiseBurst && i == 10) x = 1.f;
			fx.process(x, x, 0.f, p, l, r); t.o.push_back(l);
		}
		return t;
	};
	{ // room is short, hall is longer, vast at full decay does not die
		Tail room = run(REV_ROOM, 0.3f, 0.2f, 6.f, true);
		Tail hall = run(REV_HALL, 0.6f, 0.6f, 12.f, true);
		float rr = rmsRange(room.o, (size_t) (3 * SR), (size_t) (4 * SR));
		float pk = rmsRange(room.o, 4800, 14400);
		CHECK(rr < pk * 0.001f, "room tail is gone after 3 s");
		float h1 = rmsRange(hall.o, (size_t) (1.5f * SR), (size_t) (2 * SR));
		CHECK(h1 > rr * 10.f, "hall rings longer than room");
		Tail vast = run(REV_VAST, 1.f, 1.f, 25.f, true);
		float v1 = rmsRange(vast.o, (size_t) (3 * SR), (size_t) (5 * SR));
		float v2 = rmsRange(vast.o, (size_t) (22 * SR), (size_t) (24 * SR));
		CHECK(v2 > v1 * 0.5f, "vast at maximum decay holds for 24 s");
		float mx = 0; for (float x : vast.o) mx = std::max(mx, std::fabs(x));
		CHECK(mx < 2.f && std::isfinite(mx), "vast at maximum stays bounded");
	}
	{ // shimmer: octave up energy and stability
		Tail s = run(REV_SHIMMER, 0.5f, 1.f, 30.f, true);
		float mx = 0; for (float x : s.o) mx = std::max(mx, std::fabs(x));
		CHECK(mx < 2.f && std::isfinite(mx), "shimmer at maximum decay stays bounded over 30 s");
		// sine in, shimmer on: more energy near 2x pitch than dry reverb alone
		auto sine = [&](int mode) {
			Fx fx; fx.init(SR); FxParams p; p.revMode = mode; p.revSize = 0.5f; p.revDecay = 0.8f; p.revMix = 1.f; float l, r;
			std::vector<float> o;
			for (long i = 0; i < (long) (6 * SR); i++) {
				float x = (i < (long) (2 * SR)) ? 0.4f * std::sin(2.f * FX_PI * 440.f * i / SR) : 0.f;
				fx.process(x, x, 0.f, p, l, r); o.push_back(l); }
			double a = 0, b = 0;
			for (size_t i = (size_t) (3 * SR); i < o.size(); i++) {
				a += o[i] * std::sin(2.f * FX_PI * 880.f * i / SR); b += o[i] * std::cos(2.f * FX_PI * 880.f * i / SR); }
			return std::sqrt(a * a + b * b) / (o.size() - 3 * SR);
		};
		double sh = sine(REV_SHIMMER), hl = sine(REV_HALL);
		CHECK(sh > hl * 2.0, "shimmer adds an octave above the played note");
	}
	{ // pre-delay holds the reverb back
		auto onset = [&](float pre) {
			Fx fx; fx.init(SR); FxParams p; p.revMix = 1.f; p.revPre = pre; p.revSize = 0.3f; p.revDecay = 0.3f; float l, r;
			for (long i = 0; i < (long) (1.5 * SR); i++) {
				fx.process(i == 10 ? 1.f : 0.f, i == 10 ? 1.f : 0.f, 0.f, p, l, r);
				if (std::fabs(l) > 1e-3f) return i / SR;
			}
			return 9.f;
		};
		float o0 = onset(0.f), o5 = onset(0.5f);
		CHECK(o0 < 0.1f, "reverb starts at once with no pre-delay");
		CHECK(o5 > 0.24f && o5 < 0.35f, "pre-delay 0.5 on the knob gives about 250 ms");
	}
	{ // fuzz: random settings, loud noise, no NaN, bounded
		Fx fx; fx.init(SR); srand(3); float l, r; float mx = 0; bool ok = true;
		FxParams p;
		for (long i = 0; i < (long) (20 * SR); i++) {
			if (i % 4800 == 0) {
				p.delayDiv = rand() % 16; p.delayFb = rand() / (float) RAND_MAX; p.delayMix = rand() / (float) RAND_MAX;
				p.revMode = rand() % 4; p.revSize = rand() / (float) RAND_MAX; p.revDecay = rand() / (float) RAND_MAX;
				p.revTone = rand() / (float) RAND_MAX; p.revMix = rand() / (float) RAND_MAX;
			}
			float x = 0.9f * ((rand() / (float) RAND_MAX) * 2.f - 1.f);
			fx.process(x, -x, pulse(i, 0.4f), p, l, r);
			if (!std::isfinite(l) || !std::isfinite(r)) ok = false;
			mx = std::max(mx, std::max(std::fabs(l), std::fabs(r)));
		}
		CHECK(ok, "fuzz: no NaN or infinity");
		CHECK(mx < 12.f, "fuzz: output stays bounded");
	}
	printf(fails ? "\n%d FAILED\n" : "\nall FX tests passed\n", fails);
	return fails ? 1 : 0;
}
