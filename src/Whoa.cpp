#include "plugin.hpp"
#include "HemiolaUI.hpp"
#include "WhoaLayout.hpp"
#include "WhoaEngine.hpp"
#include "WhoaFx.hpp"
#include <cmath>

// Whoa: a two-operator FM voice. One modulator, one carrier, each with its own gate and ADSR, plus a common gate.
// Polyphonic up to 16 voices, summed to stereo, then a clocked delay, and a four-mode reverb.
// DRAFT: not yet heard in Rack.

namespace {
const float kRatios[] = {0.5f, 1.f, 1.5f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 12.f, 16.f};
const int kNumRatios = 12;

float timeFromKnob(float k) {  // 1 ms to 5 s, exponential
	return 0.001f * std::pow(5000.f, k);
}

// Shows the real time in seconds instead of the knob position.
struct TimeQuantity : ParamQuantity {
	float getDisplayValue() override { return timeFromKnob(getValue()); }
	void setDisplayValue(float v) override { setValue(std::log(std::max(v, 0.001f) / 0.001f) / std::log(5000.f)); }
};
// Reverb pre-delay: 0 to 1000 ms, squared taper so the short end is easy to set.
struct PreQuantity : ParamQuantity {
	float getDisplayValue() override { return getValue() * getValue() * 1000.f; }
	void setDisplayValue(float v) override { setValue(std::sqrt(clamp(v, 0.f, 1000.f) / 1000.f)); }
};
const float kDepth[4] = {0.25f, 0.5f, 0.75f, 1.f};
}  // namespace

struct Whoa : Module {
	enum ParamId {
		RATIO_PARAM, FINE_PARAM, INDEX_PARAM, FEEDBACK_PARAM,
		MOD_A_PARAM, MOD_D_PARAM, MOD_S_PARAM, MOD_R_PARAM,
		CAR_A_PARAM, CAR_D_PARAM, CAR_S_PARAM, CAR_R_PARAM,
		DRIVE_PARAM, SPREAD_PARAM, ENVAMT_PARAM, REV_PRE_PARAM,
		DLY_TIME_PARAM, DLY_FB_PARAM, DLY_MIX_PARAM,
		REV_TYPE_PARAM, REV_SIZE_PARAM, REV_DECAY_PARAM, REV_TONE_PARAM, REV_MIX_PARAM,
		PARAMS_LEN
	};
	enum InputId { VOCT_INPUT, GATE_M_INPUT, GATE_C_INPUT, GATE_INPUT, CV_INDEX_INPUT, CV_FB_INPUT, ACCENT_INPUT,
		CV_MA_INPUT, CV_MD_INPUT, CV_MS_INPUT, CV_MR_INPUT, CV_CA_INPUT, CV_CD_INPUT, CV_CS_INPUT, CV_CR_INPUT,
		CLOCK_INPUT, CV_DFB_INPUT, CV_DMIX_INPUT, INPUTS_LEN };
	enum OutputId { OUT_L_OUTPUT, OUT_R_OUTPUT, GATE_OUT_OUTPUT, OUTPUTS_LEN };
	enum LightId { LIGHTS_LEN };

	whoa::Voice voices[16];
	dsp::SchmittTrigger tM[16], tC[16], tG[16];
	whoa::Fx fx;
	float fxRate = 0.f;
	float cvDepth[3] = {0.5f, 0.5f, 0.5f};   // voice, envelopes, delay: set in the right-click menu

	Whoa() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(RATIO_PARAM, 0.f, kNumRatios - 1.f, 1.f, "Ratio (modulator to carrier)",
			{"0.5", "1", "1.5", "2", "3", "4", "5", "6", "7", "8", "12", "16"});
		paramQuantities[RATIO_PARAM]->snapEnabled = true;
		configParam(FINE_PARAM, -0.5f, 0.5f, 0.f, "Fine ratio", "");
		configParam(INDEX_PARAM, 0.f, 10.f, 2.f, "Index (how hard the modulator pushes the carrier)");
		configParam(FEEDBACK_PARAM, 0.f, 1.f, 0.f, "Modulator feedback", " %", 0.f, 100.f);
		const char* names[8] = {"Modulator attack", "Modulator decay", "Modulator sustain", "Modulator release",
			"Carrier attack", "Carrier decay", "Carrier sustain", "Carrier release"};
		const float defs[8] = {0.f, 0.45f, 0.f, 0.4f, 0.f, 0.6f, 0.6f, 0.5f};
		for (int i = 0; i < 8; i++) {
			bool sus = (i % 4) == 2;
			if (sus)
				configParam(MOD_A_PARAM + i, 0.f, 1.f, defs[i], names[i], " %", 0.f, 100.f);
			else {
				configParam<TimeQuantity>(MOD_A_PARAM + i, 0.f, 1.f, defs[i], names[i], " s");
			}
		}
		configParam(DRIVE_PARAM, 0.f, 1.f, 0.f, "Drive", " %", 0.f, 100.f);
		configParam(SPREAD_PARAM, 0.f, 1.f, 0.f, "Stereo spread", " %", 0.f, 100.f);
		configParam(ENVAMT_PARAM, 0.f, 1.f, 1.f, "Modulator envelope on index", " %", 0.f, 100.f);
		configParam<PreQuantity>(REV_PRE_PARAM, 0.f, 1.f, 0.f, "Reverb pre-delay", " ms");
		{
			std::vector<std::string> dn;
			for (int i = 0; i < whoa::divisionCount(); i++) dn.push_back(whoa::divisionName(i));
			configSwitch(DLY_TIME_PARAM, 0.f, 15.f, 10.f, "Delay time (note value of the clock beat)", dn);
			paramQuantities[DLY_TIME_PARAM]->snapEnabled = true;
		}
		configParam(DLY_FB_PARAM, 0.f, 1.f, 0.35f, "Delay feedback", " %", 0.f, 100.f);
		configParam(DLY_MIX_PARAM, 0.f, 1.f, 0.f, "Delay mix (dry to wet)", " %", 0.f, 100.f);
		configSwitch(REV_TYPE_PARAM, 0.f, 3.f, 1.f, "Reverb type", {"Room", "Hall", "Vast", "Shimmer"});
		paramQuantities[REV_TYPE_PARAM]->snapEnabled = true;
		configParam(REV_SIZE_PARAM, 0.f, 1.f, 0.5f, "Reverb size", " %", 0.f, 100.f);
		configParam(REV_DECAY_PARAM, 0.f, 1.f, 0.5f, "Reverb decay (Vast: top of the knob holds the tail)", " %", 0.f, 100.f);
		configParam(REV_TONE_PARAM, 0.f, 1.f, 0.6f, "Reverb tone (damping)", " %", 0.f, 100.f);
		configParam(REV_MIX_PARAM, 0.f, 1.f, 0.f, "Reverb mix (dry to wet)", " %", 0.f, 100.f);
		configInput(VOCT_INPUT, "V/oct");
		configInput(GATE_M_INPUT, "Modulator gate");
		configInput(GATE_C_INPUT, "Carrier gate");
		configInput(GATE_INPUT, "Common gate (fires both)");
		configInput(CV_INDEX_INPUT, "Index CV");
		configInput(CV_FB_INPUT, "Feedback CV");
		configInput(ACCENT_INPUT, "Accent (0 to 10 V)");
		const char* cvn[8] = {"Modulator attack CV", "Modulator decay CV", "Modulator sustain CV", "Modulator release CV",
			"Carrier attack CV", "Carrier decay CV", "Carrier sustain CV", "Carrier release CV"};
		for (int i = 0; i < 8; i++) configInput(CV_MA_INPUT + i, cvn[i]);
		configInput(CLOCK_INPUT, "Clock in (one pulse per beat; 120 BPM if unpatched)");
		configInput(CV_DFB_INPUT, "Delay feedback CV");
		configInput(CV_DMIX_INPUT, "Delay mix CV");
		configOutput(OUT_L_OUTPUT, "Left");
		configOutput(OUT_R_OUTPUT, "Right");
		configOutput(GATE_OUT_OUTPUT, "Gate out (high while any gate is high)");
	}

	void process(const ProcessArgs& args) override {
		int n = std::max(1, std::max(inputs[VOCT_INPUT].getChannels(),
			std::max(inputs[GATE_M_INPUT].getChannels(), std::max(inputs[GATE_C_INPUT].getChannels(), inputs[GATE_INPUT].getChannels()))));
		int ri = (int) std::round(params[RATIO_PARAM].getValue());
		ri = clamp(ri, 0, kNumRatios - 1);
		const float cvV = cvDepth[0], cvE = cvDepth[1], cvD = cvDepth[2];
		if (fxRate != args.sampleRate) { fxRate = args.sampleRate; fx.init(fxRate); }
		auto env = [&](int param, int input, int c) {
			float k = params[param].getValue() + inputs[input].getPolyVoltage(c) * 0.1f * cvE;
			return clamp(k, 0.f, 1.f);
		};
		float sumL = 0.f, sumR = 0.f;
		for (int c = 0; c < n; c++) {
			whoa::Params p;
			p.voct = inputs[VOCT_INPUT].getPolyVoltage(c);
			p.ratioCoarse = kRatios[ri];
			p.fine = params[FINE_PARAM].getValue();
			p.index = clamp(params[INDEX_PARAM].getValue() + inputs[CV_INDEX_INPUT].getPolyVoltage(c) * cvV, 0.f, 10.f);
			p.feedback = clamp(params[FEEDBACK_PARAM].getValue() + inputs[CV_FB_INPUT].getPolyVoltage(c) * 0.1f * cvV, 0.f, 1.f);
			p.modEnvToIndex = params[ENVAMT_PARAM].getValue();
			p.drive = params[DRIVE_PARAM].getValue();
			p.spread = params[SPREAD_PARAM].getValue();
			p.accent = clamp(inputs[ACCENT_INPUT].getPolyVoltage(c) / 10.f, 0.f, 1.f);
			p.mA = timeFromKnob(env(MOD_A_PARAM, CV_MA_INPUT, c));
			p.mD = timeFromKnob(env(MOD_D_PARAM, CV_MD_INPUT, c));
			p.mS = env(MOD_S_PARAM, CV_MS_INPUT, c);
			p.mR = timeFromKnob(env(MOD_R_PARAM, CV_MR_INPUT, c));
			p.cA = timeFromKnob(env(CAR_A_PARAM, CV_CA_INPUT, c));
			p.cD = timeFromKnob(env(CAR_D_PARAM, CV_CD_INPUT, c));
			p.cS = env(CAR_S_PARAM, CV_CS_INPUT, c);
			p.cR = timeFromKnob(env(CAR_R_PARAM, CV_CR_INPUT, c));
			// gates with hysteresis, held as states
			tM[c].process(inputs[GATE_M_INPUT].getPolyVoltage(c), 0.1f, 1.f);
			tC[c].process(inputs[GATE_C_INPUT].getPolyVoltage(c), 0.1f, 1.f);
			tG[c].process(inputs[GATE_INPUT].getPolyVoltage(c), 0.1f, 1.f);
			float l, r;
			voices[c].process(p, tM[c].isHigh(), tC[c].isHigh(), tG[c].isHigh(), args.sampleRate, l, r);
			sumL += l; sumR += r;
			outputs[GATE_OUT_OUTPUT].setVoltage(voices[c].gateOut ? 10.f : 0.f, c);
		}
		// voices summed to stereo (scaled by the voice count so chords do not clip), then the effects
		float g = 1.f / std::sqrt((float) n);
		whoa::FxParams fp;
		fp.delayDiv = clamp((int) std::round(params[DLY_TIME_PARAM].getValue()), 0, 15);
		fp.delayFb = clamp(params[DLY_FB_PARAM].getValue() + inputs[CV_DFB_INPUT].getVoltage() * 0.1f * cvD, 0.f, 1.f);
		fp.delayMix = clamp(params[DLY_MIX_PARAM].getValue() + inputs[CV_DMIX_INPUT].getVoltage() * 0.1f * cvD, 0.f, 1.f);
		fp.revMode = clamp((int) std::round(params[REV_TYPE_PARAM].getValue()), 0, 3);
		fp.revSize = params[REV_SIZE_PARAM].getValue();
		fp.revDecay = params[REV_DECAY_PARAM].getValue();
		fp.revTone = params[REV_TONE_PARAM].getValue();
		fp.revMix = params[REV_MIX_PARAM].getValue();
		fp.revPre = params[REV_PRE_PARAM].getValue();
		float oL, oR;
		fx.process(sumL * g, sumR * g, inputs[CLOCK_INPUT].getVoltage(), fp, oL, oR);
		outputs[OUT_L_OUTPUT].setVoltage(oL * 5.f);
		outputs[OUT_R_OUTPUT].setVoltage(oR * 5.f);
		outputs[OUT_L_OUTPUT].setChannels(1);
		outputs[OUT_R_OUTPUT].setChannels(1);
		outputs[GATE_OUT_OUTPUT].setChannels(n);
	}

	json_t* dataToJson() override {
		json_t* r = json_object();
		json_object_set_new(r, "cvVoice", json_real(cvDepth[0]));
		json_object_set_new(r, "cvEnv", json_real(cvDepth[1]));
		json_object_set_new(r, "cvDelay", json_real(cvDepth[2]));
		return r;
	}
	void dataFromJson(json_t* r) override {
		const char* k[3] = {"cvVoice", "cvEnv", "cvDelay"};
		for (int i = 0; i < 3; i++) {
			json_t* v = json_object_get(r, k[i]);
			if (v) cvDepth[i] = clamp((float) json_number_value(v), 0.f, 1.f);
		}
	}
	int depthIndex(int g) const {
		int best = 0;
		for (int i = 1; i < 4; i++) if (std::fabs(kDepth[i] - cvDepth[g]) < std::fabs(kDepth[best] - cvDepth[g])) best = i;
		return best;
	}

	void onReset() override {
		for (int i = 0; i < 3; i++) cvDepth[i] = 0.5f;
		for (int i = 0; i < 16; i++) voices[i] = whoa::Voice();
		if (fxRate > 0.f) fx.init(fxRate);
	}
};

void WhoaWidget_appendMenu(Whoa* m, Menu* menu);

struct WhoaWidget : ModuleWidget {
	WhoaWidget(Whoa* module) {
		using namespace hemiola_ui;
		namespace L = whoa_layout;
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Whoa.svg")));
#define KNOB(N, P) addParam(createParamCentered<HKnob>(mm2px(Vec(L::N##X, L::N##Y)), module, Whoa::P))
		KNOB(RATIO, RATIO_PARAM); KNOB(FINE, FINE_PARAM); KNOB(INDEX, INDEX_PARAM); KNOB(FEEDBACK, FEEDBACK_PARAM);
		KNOB(MOD_A, MOD_A_PARAM); KNOB(MOD_D, MOD_D_PARAM); KNOB(MOD_S, MOD_S_PARAM); KNOB(MOD_R, MOD_R_PARAM);
		KNOB(CAR_A, CAR_A_PARAM); KNOB(CAR_D, CAR_D_PARAM); KNOB(CAR_S, CAR_S_PARAM); KNOB(CAR_R, CAR_R_PARAM);
		KNOB(DRIVE, DRIVE_PARAM); KNOB(SPREAD, SPREAD_PARAM); KNOB(ENVAMT, ENVAMT_PARAM);
		KNOB(DLY_TIME, DLY_TIME_PARAM); KNOB(DLY_FB, DLY_FB_PARAM); KNOB(DLY_MIX, DLY_MIX_PARAM);
		KNOB(REV_TYPE, REV_TYPE_PARAM); KNOB(REV_SIZE, REV_SIZE_PARAM); KNOB(REV_DECAY, REV_DECAY_PARAM);
		KNOB(REV_TONE, REV_TONE_PARAM); KNOB(REV_MIX, REV_MIX_PARAM); KNOB(REV_PRE, REV_PRE_PARAM);
#undef KNOB
#define IN(N, P) addInput(createInputCentered<HPortIn>(mm2px(Vec(L::N##X, L::N##Y)), module, Whoa::P))
		IN(VOCT, VOCT_INPUT); IN(GATE_M, GATE_M_INPUT); IN(GATE_C, GATE_C_INPUT); IN(GATE, GATE_INPUT);
		IN(CV_INDEX, CV_INDEX_INPUT); IN(CV_FB, CV_FB_INPUT); IN(ACCENT, ACCENT_INPUT);
		IN(CV_MA, CV_MA_INPUT); IN(CV_MD, CV_MD_INPUT); IN(CV_MS, CV_MS_INPUT); IN(CV_MR, CV_MR_INPUT);
		IN(CV_CA, CV_CA_INPUT); IN(CV_CD, CV_CD_INPUT); IN(CV_CS, CV_CS_INPUT); IN(CV_CR, CV_CR_INPUT);
		IN(CLOCK, CLOCK_INPUT); IN(CV_DFB, CV_DFB_INPUT); IN(CV_DMIX, CV_DMIX_INPUT);
#undef IN
#define OUT(N, P) addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::N##X, L::N##Y)), module, Whoa::P))
		OUT(OUT_L, OUT_L_OUTPUT); OUT(OUT_R, OUT_R_OUTPUT); OUT(GATE_OUT, GATE_OUT_OUTPUT);
#undef OUT
	}
	void appendContextMenu(Menu* menu) override {
		Whoa* m = dynamic_cast<Whoa*>(module);
		if (m) WhoaWidget_appendMenu(m, menu);
	}
};

// CV depth lives in the right-click menu (three groups), saved with the patch.
void WhoaWidget_appendMenu(Whoa* m, Menu* menu) {
	menu->addChild(new MenuSeparator);
	menu->addChild(createMenuLabel("CV depth (10 V sweeps this share of the knob)"));
	static const char* names[3] = {"Voice CV: index, feedback", "Envelope CVs: all eight", "Delay CVs: feedback, mix"};
	for (int g = 0; g < 3; g++)
		menu->addChild(createIndexSubmenuItem(names[g], {"25%", "50%", "75%", "100%"},
			[=]() { return (size_t) m->depthIndex(g); },
			[=](size_t i) { m->cvDepth[g] = kDepth[i]; }));
}

Model* modelWhoa = createModel<Whoa, WhoaWidget>("Whoa");
