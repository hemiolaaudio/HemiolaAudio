// Whoa: two-operator FM voice. Rack-free engine.
#pragma once
#include <cmath>
#include <algorithm>

namespace whoa {

constexpr float PI = 3.14159265358979f;
constexpr float TWO_PI = 6.28318530717959f;

// ADSR with exponential-style segments. Times in seconds.
struct Adsr {
	enum Stage { IDLE, ATTACK, DECAY, SUSTAIN, RELEASE };
	Stage stage = IDLE;
	float level = 0.f;
	bool gate = false;

	float step(bool g, float a, float d, float s, float r, float sr) {
		if (g && !gate) stage = ATTACK;
		if (!g && gate) stage = RELEASE;
		gate = g;
		auto coef = [&](float t) { return 1.f - std::exp(-4.6f / (std::max(t, 0.0005f) * sr)); };
		switch (stage) {
			case ATTACK:
				level += (1.2f - level) * coef(a);
				if (level >= 1.f) { level = 1.f; stage = DECAY; }
				break;
			case DECAY:
				level += (s - level) * coef(d);
				if (std::fabs(level - s) < 1e-4f) { level = s; stage = SUSTAIN; }
				break;
			case SUSTAIN:
				level = s;
				break;
			case RELEASE:
				level += (0.f - level) * coef(r);
				if (level < 1e-5f) { level = 0.f; stage = IDLE; }
				break;
			default:
				break;
		}
		return level;
	}
};

struct Params {
	float voct = 0.f;          // V/oct, 0 V = C4
	float ratioCoarse = 1.f;   // carrier : modulator, from a stepped list
	float fine = 0.f;          // -0.5 .. +0.5, added to ratio
	float index = 2.f;         // modulation depth, 0 .. 10
	float feedback = 0.f;      // 0 .. 1
	float modEnvToIndex = 1.f; // 0 .. 1, how much the modulator envelope moves the index
	float drive = 0.f;         // 0 .. 1
	float spread = 0.f;        // 0 .. 1
	float accent = 0.f;        // 0 .. 1, opens the modulator wider and lifts the level
	float mA = 0.001f, mD = 0.15f, mS = 0.f, mR = 0.1f;
	float cA = 0.001f, cD = 0.3f, cS = 0.6f, cR = 0.2f;
};

struct Voice {
	Adsr modEnv, carEnv;
	float phM = 0.f, phCL = 0.f, phCR = 0.f;
	float fb1 = 0.f, fb2 = 0.f;
	bool gateOut = false;
	float xl = 0.f, xr = 0.f, ylp = 0.f, yrp = 0.f; // DC blocker state

	static float freqFromVoct(float v) { return 261.6256f * std::exp2(v); }

	// gM, gC are the individual gates, gCommon fires both.
	void process(const Params& p, bool gM, bool gC, bool gCommon, float sr, float& outL, float& outR) {
		bool mg = gM || gCommon;
		bool cg = gC || gCommon;
		gateOut = mg || cg;
		float me = modEnv.step(mg, p.mA, p.mD, p.mS, p.mR, sr);
		float ce = carEnv.step(cg, p.cA, p.cD, p.cS, p.cR, sr);

		float f = std::min(freqFromVoct(p.voct), sr * 0.25f);
		float ratio = std::max(0.01f, p.ratioCoarse + p.fine);
		float fc = f;
		float fm = std::min(f * ratio, sr * 0.45f);
		float det = 1.f + p.spread * 0.006f; // about 10 cents at most

		// 4x oversampling of the operator chain
		float accL = 0.f, accR = 0.f;
		for (int k = 0; k < 4; k++) {
			float srs = sr * 4.f;
			float fbAmt = p.feedback * p.feedback * 2.2f;
			float m = std::sin(phM * TWO_PI + fbAmt * 0.5f * (fb1 + fb2));
			fb2 = fb1;
			fb1 = m;
			phM += fm / srs;
			phM -= std::floor(phM);
			float depth = p.index * (1.f - p.modEnvToIndex + p.modEnvToIndex * me) * (1.f + 0.6f * p.accent);
			float mod = m * depth;
			float cl = std::sin(phCL * TWO_PI + mod);
			float cr = std::sin(phCR * TWO_PI + mod);
			phCL += fc * det / srs;
			phCR += fc / det / srs;
			phCL -= std::floor(phCL);
			phCR -= std::floor(phCR);
			accL += cl;
			accR += cr;
		}
		float lvl = ce * (0.8f + 0.2f * p.accent);
		float l = accL * 0.25f * lvl;
		float r = accR * 0.25f * lvl;
		if (p.drive > 0.f) {
			// asymmetric tanh: the bias adds even harmonics, the subtraction removes the DC it creates
			float g = 1.f + p.drive * 9.f;
			float bias = 0.25f * p.drive;
			float dc = std::tanh(g * bias);
			float hi = std::tanh(g * (1.f + bias)) - dc;
			float lo = std::tanh(g * (-1.f + bias)) - dc;
			float norm = 0.95f / std::max(std::fabs(hi), std::fabs(lo));
			l = (std::tanh(g * (l + bias)) - dc) * norm;
			r = (std::tanh(g * (r + bias)) - dc) * norm;
			// the asymmetry shifts the average, so block DC
			float yl = l - xl + 0.995f * ylp; xl = l; ylp = yl; l = yl;
			float yr = r - xr + 0.995f * yrp; xr = r; yrp = yr; r = yr;
		}
		outL = l;
		outR = r;
		if (!std::isfinite(outL)) outL = 0.f;
		if (!std::isfinite(outR)) outR = 0.f;
	}
};

} // namespace whoa
