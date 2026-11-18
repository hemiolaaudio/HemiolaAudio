// Whoa effects: clocked stereo delay, four-mode reverb (Room, Hall, Vast, Shimmer).
// Rack-free. All memory is allocated in init(), never in process().
#pragma once
#include <cmath>
#include <vector>
#include <algorithm>

namespace whoa {

static const float FX_PI = 3.14159265358979f;

inline float fxFlush(float x) { return (std::fabs(x) < 1e-20f) ? 0.f : x; }
// Leaves small signals alone, folds large ones in smoothly. Keeps a lossless loop from running away.
inline float fxSat(float x) {
	float a = std::fabs(x);
	if (a < 0.6f) return x;
	float s = 0.6f + 0.4f * std::tanh((a - 0.6f) / 0.4f);
	return x < 0.f ? -s : s;
}
inline float fxOnePole(float cutoffHz, float sr) {
	return 1.f - std::exp(-2.f * FX_PI * std::min(cutoffHz, 0.45f * sr) / sr);
}

// ---------------------------------------------------------------- delay divisions
// Beats are quarter notes. The clock input carries one pulse per beat.
inline int divisionCount() { return 16; }
inline float divisionBeats(int i) {
	static const float b[16] = {
		0.125f, 1.f / 7.f, 1.f / 6.f, 0.2f, 0.25f, 0.375f, 1.f / 3.f, 0.5f,
		0.75f, 2.f / 3.f, 1.f, 1.5f, 4.f / 3.f, 2.f, 3.f, 4.f };
	return b[std::max(0, std::min(15, i))];
}
inline const char* divisionName(int i) {
	static const char* const n[16] = {
		"1/32", "1/7 beat", "1/16T", "1/5 beat", "1/16", "1/16.", "1/8T", "1/8",
		"1/8.", "1/4T", "1/4", "1/4.", "1/2T", "1/2", "1/2.", "1 bar" };
	return n[std::max(0, std::min(15, i))];
}

// Measures the beat period from clock pulses. Falls back to 120 BPM when nothing is patched.
struct ClockMeter {
	float sr = 48000.f;
	bool high = false;
	long since = 0;
	float period = 0.5f;   // seconds per beat
	bool locked = false;
	void init(float s) { sr = s; high = false; since = 0; period = 0.5f; locked = false; }
	float process(float v) {
		since++;
		if (!high && v >= 1.f) {
			high = true;
			float t = since / sr;
			if (t > 0.04f && t < 6.f) {
				if (locked && std::fabs(t - period) < 0.12f * period) period += 0.5f * (t - period);
				else period = t;
				locked = true;
			}
			since = 0;
		}
		else if (high && v <= 0.1f) high = false;
		if (since > (long) (6.f * sr)) { locked = false; period = 0.5f; }
		return period;
	}
};

// ---------------------------------------------------------------- delay
struct Delay {
	float sr = 48000.f;
	std::vector<float> bl, br;
	size_t n = 0, w = 0;
	float time = 0.25f;     // smoothed delay time, seconds
	float tk = 0.001f;
	float lpL = 0.f, lpR = 0.f, lpk = 0.1f;
	void init(float s) {
		sr = s; n = (size_t) (8.5f * s) + 8; bl.assign(n, 0.f); br.assign(n, 0.f);
		w = 0; time = 0.25f; lpL = lpR = 0.f; tk = 1.f - std::exp(-1.f / (0.06f * s)); lpk = fxOnePole(5500.f, s);
	}
	float tap(const std::vector<float>& b, float d) const {
		float rp = (float) w - d; while (rp < 0.f) rp += (float) n;
		if (rp >= (float) n) rp -= (float) n;
		size_t i0 = (size_t) rp; if (i0 >= n) i0 = n - 1;
		float f = rp - (float) i0; size_t i1 = (i0 + 1) % n;
		return b[i0] * (1.f - f) + b[i1] * f;
	}
	// Stereo in, stereo wet out. Feedback crosses channels (ping-pong).
	void process(float inL, float inR, float target, float fb, float& wetL, float& wetR) {
		time += (target - time) * tk;
		float d = std::max(2.f, std::min(time * sr, (float) n - 4.f));
		float a = tap(bl, d), b = tap(br, d);
		wetL = a; wetR = b;
		lpL += lpk * (a - lpL); lpR += lpk * (b - lpR);
		float g = std::min(fb, 0.97f);
		bl[w] = fxFlush(fxSat(inL + g * lpR));
		br[w] = fxFlush(fxSat(inR + g * lpL));
		w = (w + 1) % n;
	}
};

// ---------------------------------------------------------------- reverb
enum { REV_ROOM = 0, REV_HALL = 1, REV_VAST = 2, REV_SHIMMER = 3 };

struct Reverb {
	enum { N = 8 };
	float sr = 48000.f;
	struct Line {
		std::vector<float> b; size_t n = 0, w = 0;
		float lp = 0.f, phase = 0.f, rate = 0.f;
	} L[N];
	struct AP { std::vector<float> b; size_t n = 0, w = 0; } ap[4];
	// shimmer pitch shifter (octave up), two crossfaded taps
	std::vector<float> sb; size_t sn = 0, sw = 0; float sp = 0.f; float shLp = 0.f;
	float scale = 1.f;      // smoothed size scale
	float shOut = 0.f;
	float dcX = 0.f, dcY = 0.f;
	float gain[8] = {0,0,0,0,0,0,0,0}, lpk = 0.1f, sk = 0.001f, shk = 0.1f;
	int gcount = 0;
	std::vector<float> pb[2]; size_t pn = 0, pw = 0; float pre = 0.f, pk = 0.001f;   // pre-delay

	static float baseMs(int i) {
		static const float m[N] = { 29.7f, 37.1f, 41.1f, 43.7f, 53.3f, 59.9f, 67.7f, 79.3f };
		return m[i];
	}
	void init(float s) {
		sr = s;
		for (int i = 0; i < N; i++) {
			L[i].n = (size_t) (0.95f * s) + 8; L[i].b.assign(L[i].n, 0.f);
			L[i].w = 0; L[i].lp = 0.f; L[i].phase = 0.13f * i; L[i].rate = 0.11f + 0.043f * i;
		}
		static const float apMs[4] = { 4.7f, 6.9f, 5.3f, 8.1f };
		for (int i = 0; i < 4; i++) { ap[i].n = (size_t) (apMs[i] * 0.001f * s) + 2; ap[i].b.assign(ap[i].n, 0.f); ap[i].w = 0; }
		sn = (size_t) (0.1f * s) + 8; sb.assign(sn, 0.f); sw = 0; sp = 0.f; shLp = 0.f;
		scale = 1.f; shOut = 0.f; dcX = dcY = 0.f; gcount = 0;
		sk = 1.f - std::exp(-1.f / (0.25f * s)); shk = fxOnePole(6000.f, s);
		pn = (size_t) (1.05f * s) + 8; pb[0].assign(pn, 0.f); pb[1].assign(pn, 0.f); pw = 0; pre = 0.f;
		pk = 1.f - std::exp(-1.f / (0.05f * s));
	}
	static float allpass(AP& a, float x) {
		float y = a.b[a.w]; float v = x + 0.6f * y; a.b[a.w] = v; a.w = (a.w + 1) % a.n; return y - 0.6f * v;
	}
	float readLine(const Line& l, float d) const {
		float rp = (float) l.w - d; while (rp < 0.f) rp += (float) l.n;
		if (rp >= (float) l.n) rp -= (float) l.n;   // a tiny negative value can round up to exactly n
		size_t i0 = (size_t) rp; if (i0 >= l.n) i0 = l.n - 1;
		float f = rp - (float) i0; size_t i1 = (i0 + 1) % l.n;
		return l.b[i0] * (1.f - f) + l.b[i1] * f;
	}
	static void modeShape(int mode, float size, float decay, float& sc, float& rt, float& modMs, float& rateMul, bool& hold) {
		hold = false;
		float e = std::max(0.f, std::min(1.f, decay));
		switch (mode) {
			case REV_HALL:    sc = 0.8f + 1.4f * size; rt = 0.8f * std::pow(11.f, e); modMs = 0.6f; rateMul = 1.f; break;
			case REV_VAST:    sc = 2.f + 7.f * size;   rt = 4.f * std::pow(15.f, e);  modMs = 2.5f; rateMul = 0.5f;
				hold = (e >= 0.98f); break;
			case REV_SHIMMER: sc = 1.f + 2.f * size;   rt = 2.f * std::pow(10.f, e);  modMs = 0.8f; rateMul = 0.8f; break;
			default:          sc = 0.25f + 0.75f * size; rt = 0.2f * std::pow(12.5f, e); modMs = 0.2f; rateMul = 1.f; break;
		}
	}
	// Stereo in, stereo wet out.
	// preSec: pre-delay in seconds (0 to 1). Stereo in, stereo wet out.
	void process(float inL, float inR, int mode, float size, float decay, float tone, float preSec, float& outL, float& outR) {
		pre += (std::max(0.f, std::min(1.f, preSec)) - pre) * pk;
		pb[0][pw] = inL; pb[1][pw] = inR;
		{
			float rp = (float) pw - std::max(1.f, pre * sr); while (rp < 0.f) rp += (float) pn;
			if (rp >= (float) pn) rp -= (float) pn;
			size_t i0 = (size_t) rp; if (i0 >= pn) i0 = pn - 1;
			float fr = rp - (float) i0; size_t i1 = (i0 + 1) % pn;
			inL = pb[0][i0] * (1.f - fr) + pb[0][i1] * fr; inR = pb[1][i0] * (1.f - fr) + pb[1][i1] * fr;
		}
		pw = (pw + 1) % pn;
		float sc, rt, modMs, rateMul; bool hold;
		modeShape(mode, size, decay, sc, rt, modMs, rateMul, hold);
		scale += (sc - scale) * sk;
		if (gcount == 0) {   // block-rate: gains and tone filter
			lpk = fxOnePole(1500.f * std::pow(11.f, tone), sr);
			for (int i = 0; i < N; i++) {
				float len = baseMs(i) * 0.001f * sr * scale;
				gain[i] = hold ? 1.f : std::pow(10.f, -3.f * len / (rt * sr));
			}
		}
		gcount = (gcount + 1) & 31;

		// input diffusion
		float xl = allpass(ap[1], allpass(ap[0], inL));
		float xr = allpass(ap[3], allpass(ap[2], inR));

		// read lines
		float v[N];
		for (int i = 0; i < N; i++) {
			Line& l = L[i];
			float len = baseMs(i) * 0.001f * sr * scale;
			l.phase += l.rate * rateMul / sr; if (l.phase >= 1.f) l.phase -= 1.f;
			float m = std::sin(2.f * FX_PI * l.phase) * modMs * 0.001f * sr;
			float d = std::max(2.f, std::min(len + m, (float) l.n - 4.f));
			float y = readLine(l, d);
			// per-line gain for the requested RT60
			float g = gain[i];
			l.lp += lpk * (y - l.lp);
			v[i] = g * l.lp;
		}
		// shimmer: octave-up shifter on the mono sum of the lines, fed back into the loop
		float sum = 0.f; for (int i = 0; i < N; i++) sum += v[i];
		float shin = (mode == REV_SHIMMER) ? sum * 0.35f : 0.f;
		sb[sw] = fxFlush(shin); 
		float W = 0.08f * sr;
		float d1 = (1.f - sp) * W + 2.f, p2 = sp + 0.5f; if (p2 >= 1.f) p2 -= 1.f;
		float d2 = (1.f - p2) * W + 2.f;
		auto stap = [&](float d) {
			float rp = (float) sw - d; while (rp < 0.f) rp += (float) sn;
			if (rp >= (float) sn) rp -= (float) sn;
			size_t i0 = (size_t) rp; if (i0 >= sn) i0 = sn - 1;
			float f = rp - (float) i0; size_t i1 = (i0 + 1) % sn;
			return sb[i0] * (1.f - f) + sb[i1] * f; };
		float s1 = std::sin(FX_PI * sp), s2 = std::sin(FX_PI * p2);
		float sh = stap(d1) * s1 * s1 + stap(d2) * s2 * s2;
		sw = (sw + 1) % sn; sp += 1.f / W; if (sp >= 1.f) sp -= 1.f;
		shLp += shk * (sh - shLp);
		shOut = (mode == REV_SHIMMER) ? shLp : 0.f;

		// Hadamard mix (fast Walsh-Hadamard, scaled to be lossless)
		float h[N]; for (int i = 0; i < N; i++) h[i] = v[i];
		for (int s = 1; s < N; s <<= 1)
			for (int i = 0; i < N; i += 2 * s)
				for (int j = i; j < i + s; j++) { float a = h[j], b = h[j + s]; h[j] = a + b; h[j + s] = a - b; }
		const float norm = 0.35355339f;
		for (int i = 0; i < N; i++) {
			float in = ((i & 1) ? xr : xl) * 0.25f + shOut * 0.35f * ((i & 2) ? -1.f : 1.f);
			Line& l = L[i];
			l.b[l.w] = fxFlush(fxSat(h[i] * norm + in));
			l.w = (l.w + 1) % l.n;
		}
		// stereo taps: two orthogonal sign patterns
		float oL = 0.f, oR = 0.f;
		static const float sl[N] = { 1, -1, 1, -1, 1, -1, 1, -1 };
		static const float sr2[N] = { 1, 1, -1, -1, 1, 1, -1, -1 };
		for (int i = 0; i < N; i++) { oL += sl[i] * v[i]; oR += sr2[i] * v[i]; }
		float lvl = (mode == REV_ROOM) ? 1.6f : 1.4f;
		outL = oL * 0.35f * lvl; outR = oR * 0.35f * lvl;
	}
};

struct FxParams {
	int delayDiv = 10;      // index into the division table
	float delayFb = 0.35f, delayMix = 0.f;
	int revMode = REV_HALL;
	float revSize = 0.5f, revDecay = 0.5f, revTone = 0.6f, revMix = 0.f;
	float revPre = 0.f;     // pre-delay knob 0..1, taper to 0..1000 ms
};

struct Fx {
	ClockMeter clock; Delay delay; Reverb reverb;
	void init(float sr) { clock.init(sr); delay.init(sr); reverb.init(sr); }
	void process(float inL, float inR, float clockV, const FxParams& p, float& outL, float& outR) {
		float beat = clock.process(clockV);
		float target = divisionBeats(p.delayDiv) * beat;
		float dl, dr; delay.process(inL, inR, target, p.delayFb, dl, dr);
		float dm = std::max(0.f, std::min(1.f, p.delayMix));
		float dg = std::sin(0.5f * FX_PI * dm), dd = std::cos(0.5f * FX_PI * dm);
		float xl = inL * dd + dl * dg, xr = inR * dd + dr * dg;
		float rl, rr; reverb.process(xl, xr, p.revMode, p.revSize, p.revDecay, p.revTone, p.revPre * p.revPre, rl, rr);
		float rm = std::max(0.f, std::min(1.f, p.revMix));
		float rg = std::sin(0.5f * FX_PI * rm), rd = std::cos(0.5f * FX_PI * rm);
		float yl = xl * rd + rl * rg, yr = xr * rd + rr * rg;
		outL = yl; outR = yr;
	}
};

} // namespace whoa
