#include "plugin.hpp"
#include "HemiolaUI.hpp"
#include "PanelLayout.hpp"
#include "GatiEngine.hpp"
#include "Modulation.hpp"

// R&T (Rhythm & Thalam): splits every beat into 1 to 9 equal parts (what Carnatic music calls nadai, also gati).
struct RhythmTalam : Module {
	enum ParamId { DIVISION_PARAM, CV_AMOUNT_PARAM, GATE_LEN_PARAM, CV_MODE_PARAM, ACCENT_PARAM, DIVIDE_PARAM, PROB_PARAM, PARAMS_LEN };
	enum InputId { CLOCK_INPUT, CV_INPUT, RESET_INPUT, DIVIDE_CV_INPUT, INPUTS_LEN };
	enum OutputId { PARTS_TRIG_OUTPUT, FIRST_TRIG_OUTPUT, PARTS_GATE_OUTPUT, FIRST_GATE_OUTPUT, CLICK_OUTPUT, DIV_TRIG_OUTPUT, DIV_GATE_OUTPUT, OUTPUTS_LEN };
	enum LightId { LIGHTS_LEN };

	hemiola::GatiEngine engine;
	hemiola::PartDivider divider;
	hemiola::StepCounter counter;
	hemiola::ClickSynth click;
	dsp::SchmittTrigger clockTrigger, cvPulseTrigger, resetTrigger;
	dsp::PulseGenerator partsTrig, firstTrig, partsGate, firstGate, divTrig, divGate;
	int lastDivision = -1;
	float sinceBeat = 0.f;
	float beatPeriod = 0.f;

	RhythmTalam() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(DIVISION_PARAM, 1.f, 9.f, 4.f, "Parts per beat");
		paramQuantities[DIVISION_PARAM]->snapEnabled = true;
		configParam(CV_AMOUNT_PARAM, -1.f, 1.f, 1.f, "CV amount (Level mode)", " parts per volt", 0.f, 1.f);
		configParam(GATE_LEN_PARAM, 0.05f, 0.95f, 0.5f, "Gate length", " % of a part", 0.f, 100.f);
		configSwitch(CV_MODE_PARAM, 0.f, 1.f, 0.f, "CV mode", {"Level", "Count"});
		configSwitch(ACCENT_PARAM, 0.f, 1.f, 1.f, "Accent on the beat", {"Off", "On"});
		configParam(DIVIDE_PARAM, 1.f, 64.f, 1.f, "Divide the parts by");
		paramQuantities[DIVIDE_PARAM]->snapEnabled = true;
		configParam(PROB_PARAM, 0.f, 1.f, 1.f, "Chance that each part sounds (the part on the beat always does)", " %", 0.f, 100.f);
		configInput(RESET_INPUT, "Reset (the next part starts a new DIVIDE count)");
		configInput(DIVIDE_CV_INPUT, "Divide CV (0 to 10 V sweeps 1 to 64)");
		configInput(CLOCK_INPUT, "Beat clock");
		configInput(CV_INPUT, "Division CV");
		configOutput(PARTS_TRIG_OUTPUT, "Parts trigger (every part, including the one on the beat)");
		configOutput(FIRST_TRIG_OUTPUT, "First-part trigger (only the part on the beat)");
		configOutput(PARTS_GATE_OUTPUT, "Parts gate");
		configOutput(FIRST_GATE_OUTPUT, "First-part gate (length is a share of the whole beat)");
		configOutput(CLICK_OUTPUT, "Click (audio)");
		configOutput(DIV_TRIG_OUTPUT, "Divided trigger (every Nth part)");
		configOutput(DIV_GATE_OUTPUT, "Divided gate (length is a share of the divided step)");
	}

	void onReset() override {
		engine.reset();
		divider.reset();
		counter.reset();
		lastDivision = -1;
		sinceBeat = 0.f;
		beatPeriod = 0.f;
	}

	int division() {
		int knob = hemiola::clampInt((int) std::round(params[DIVISION_PARAM].getValue()), 1, 9);
		if (!inputs[CV_INPUT].isConnected()) {
			counter.reset();
			return knob;
		}
		if (params[CV_MODE_PARAM].getValue() > 0.5f) {
			if (cvPulseTrigger.process(inputs[CV_INPUT].getVoltage(), 0.1f, 2.f))
				counter.advance(9);
			return counter.choose(knob, 1, 9);
		}
		counter.reset();
		return hemiola::levelChoice((float) knob, inputs[CV_INPUT].getVoltage(), params[CV_AMOUNT_PARAM].getValue(), 1, 9);
	}

	void process(const ProcessArgs& args) override {
		int d = division();
		if (d != lastDivision) {
			engine.setDivision(d);
			lastDivision = d;
		}
		bool edge = clockTrigger.process(inputs[CLOCK_INPUT].getVoltage(), 0.1f, 2.f);
		if (edge) {
			if (sinceBeat > 0.f && sinceBeat < 4.f)
				beatPeriod = sinceBeat;
			sinceBeat = 0.f;
		}
		else {
			sinceBeat += args.sampleTime;
		}

		bool first = false;
		if (resetTrigger.process(inputs[RESET_INPUT].getVoltage(), 0.1f, 2.f))
			divider.reset();  // before the part is counted, so a reset on the beat starts the count on that part
		bool part = engine.process(edge, first);
		int every = hemiola::clampInt((int) std::round(params[DIVIDE_PARAM].getValue()), 1, 64);
		if (inputs[DIVIDE_CV_INPUT].isConnected())
			every = hemiola::levelChoice((float) every, inputs[DIVIDE_CV_INPUT].getVoltage(), 6.3f, 1, 64);
		divider.setDivide(every);
		bool counted = divider.process(part);
		float chance = params[PROB_PARAM].getValue();
		bool pass = part && hemiola::partPasses(chance, random::uniform(), first);
		bool divPart = counted && pass;
		if (part && !engine.hasTempo())
			divider.reset();  // the first beat only learns the tempo, so it is not counted
		if (part) {
			float share = params[GATE_LEN_PARAM].getValue();
			int n = engine.activeDivision();
			if (pass) {
				partsTrig.trigger(1e-3f);
				partsGate.trigger(hemiola::gateSeconds(beatPeriod / (float) n, share, 0.05f));
			}
			if (first) {
				firstTrig.trigger(1e-3f);
				firstGate.trigger(hemiola::gateSeconds(beatPeriod, share, 0.2f));
			}
			click.trigger(params[ACCENT_PARAM].getValue() > 0.5f && first);
		}
		if (divPart) {
			float share = params[GATE_LEN_PARAM].getValue();
			divTrig.trigger(1e-3f);
			divGate.trigger(hemiola::gateSeconds(beatPeriod / (float) engine.activeDivision() * (float) every, share, 0.05f));
		}
		outputs[DIV_TRIG_OUTPUT].setVoltage(divTrig.process(args.sampleTime) ? 10.f : 0.f);
		outputs[DIV_GATE_OUTPUT].setVoltage(divGate.process(args.sampleTime) ? 10.f : 0.f);
		outputs[PARTS_TRIG_OUTPUT].setVoltage(partsTrig.process(args.sampleTime) ? 10.f : 0.f);
		outputs[FIRST_TRIG_OUTPUT].setVoltage(firstTrig.process(args.sampleTime) ? 10.f : 0.f);
		outputs[PARTS_GATE_OUTPUT].setVoltage(partsGate.process(args.sampleTime) ? 10.f : 0.f);
		outputs[FIRST_GATE_OUTPUT].setVoltage(firstGate.process(args.sampleTime) ? 10.f : 0.f);
		outputs[CLICK_OUTPUT].setVoltage(click.process(args.sampleTime));
	}
};

struct RhythmTalamDisplay : TransparentWidget {
	RhythmTalam* module = nullptr;

	static const char* nadaiName(int n) {
		switch (n) {
			case 1: return "Beat";
			case 3: return "Tisra";
			case 4: return "Chatusra";
			case 5: return "Khanda";
			case 7: return "Misra";
			case 9: return "Sankirna";
			default: return "Even";
		}
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer == 1) {
			int n = module ? module->engine.activeDivision() : 4;
			int every = module ? module->divider.divideBy() : 1;
			std::string note = every > 1 ? "per beat, divided by " + std::to_string(every) : "per beat";
			hemiola_ui::drawDisplay(args, box, nadaiName(n), std::to_string(n), note);
		}
		TransparentWidget::drawLayer(args, layer);
	}
};

struct RhythmTalamWidget : ModuleWidget {
	RhythmTalamWidget(RhythmTalam* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/RhythmTalam.svg")));
		namespace L = RhythmTalamLayout;
		using namespace hemiola_ui;

		RhythmTalamDisplay* d = new RhythmTalamDisplay;
		d->module = module;
		d->box.pos = mm2px(Vec(L::displayX, L::displayY));
		d->box.size = mm2px(Vec(L::displayW, L::displayH));
		addChild(d);

		addParam(createParamCentered<HKnob>(mm2px(Vec(L::divisionX, L::divisionY)), module, RhythmTalam::DIVISION_PARAM));
		addParam(createParamCentered<HKnob>(mm2px(Vec(L::cvAmountX, L::cvAmountY)), module, RhythmTalam::CV_AMOUNT_PARAM));
		addParam(createParamCentered<HKnob>(mm2px(Vec(L::gateLenX, L::gateLenY)), module, RhythmTalam::GATE_LEN_PARAM));
		addParam(createParamCentered<HKnob>(mm2px(Vec(L::divideX, L::divideY)), module, RhythmTalam::DIVIDE_PARAM));
		addParam(createParamCentered<HKnob>(mm2px(Vec(L::probX, L::probY)), module, RhythmTalam::PROB_PARAM));
		addParam(createParamCentered<HToggle>(mm2px(Vec(L::cvModeX, L::cvModeY)), module, RhythmTalam::CV_MODE_PARAM));
		addParam(createParamCentered<HToggle>(mm2px(Vec(L::accentX, L::accentY)), module, RhythmTalam::ACCENT_PARAM));

		addInput(createInputCentered<HPortIn>(mm2px(Vec(L::clkInX, L::clkInY)), module, RhythmTalam::CLOCK_INPUT));
		addInput(createInputCentered<HPortIn>(mm2px(Vec(L::cvInX, L::cvInY)), module, RhythmTalam::CV_INPUT));
		addInput(createInputCentered<HPortIn>(mm2px(Vec(L::divCvX, L::divCvY)), module, RhythmTalam::DIVIDE_CV_INPUT));
		addInput(createInputCentered<HPortIn>(mm2px(Vec(L::rstInX, L::rstInY)), module, RhythmTalam::RESET_INPUT));

		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::clickX, L::clickY)), module, RhythmTalam::CLICK_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::partsTrigX, L::partsTrigY)), module, RhythmTalam::PARTS_TRIG_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::firstTrigX, L::firstTrigY)), module, RhythmTalam::FIRST_TRIG_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::divTrigX, L::divTrigY)), module, RhythmTalam::DIV_TRIG_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::divGateX, L::divGateY)), module, RhythmTalam::DIV_GATE_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::partsGateX, L::partsGateY)), module, RhythmTalam::PARTS_GATE_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::firstGateX, L::firstGateY)), module, RhythmTalam::FIRST_GATE_OUTPUT));
	}
};

Model* modelRhythmTalam = createModel<RhythmTalam, RhythmTalamWidget>("RhythmTalam");
