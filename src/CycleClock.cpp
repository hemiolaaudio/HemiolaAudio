#include "plugin.hpp"
#include "HemiolaUI.hpp"
#include "PanelLayout.hpp"
#include "TalaTable.hpp"
#include "CycleEngine.hpp"
#include "Modulation.hpp"

struct CycleClock : Module {
	enum ParamId { TEMPO_PARAM, CYCLE_PARAM, CV_AMOUNT_PARAM, GATE_LEN_PARAM, RUN_PARAM, RESET_PARAM, ACCENT_PARAM, CV_MODE_PARAM, SWING_PARAM, PARAMS_LEN };
	enum InputId { CLOCK_INPUT, RESET_INPUT, CYCLE_CV_INPUT, TEMPO_CV_INPUT, RUN_INPUT, INPUTS_LEN };
	enum OutputId {
		BEAT_TRIG_OUTPUT, SECTION_TRIG_OUTPUT, CYCLE_TRIG_OUTPUT, END_TRIG_OUTPUT,
		BEAT_GATE_OUTPUT, SECTION_GATE_OUTPUT, CYCLE_GATE_OUTPUT, END_GATE_OUTPUT,
		POSITION_OUTPUT, CLICK_OUTPUT, OUTPUTS_LEN
	};
	enum LightId { LIGHTS_LEN };

	hemiola::CycleEngine engine;
	hemiola::InternalClock clock;
	hemiola::StepCounter counter;
	hemiola::ClickSynth click;
	dsp::SchmittTrigger clockTrigger, resetTrigger, resetButtonTrigger, cvPulseTrigger, runTrigger;
	dsp::PulseGenerator trig[4];
	dsp::PulseGenerator gate[4];

	int lastRequested = -1;
	float sinceBeat = 0.f;
	float beatPeriod = 0.f;

	// Read by the display on the interface thread.
	int shownBeat = -1;

	CycleClock() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(TEMPO_PARAM, 30.f, 300.f, 120.f, "Tempo", " BPM");
		paramQuantities[TEMPO_PARAM]->snapEnabled = true;
		struct CycleQuantity : ParamQuantity {
			std::string getDisplayValueString() override {
				int i = (int) std::round(getValue());
				const std::vector<hemiola::CycleDef>& t = hemiola::cycleTable();
				if (i < 0 || i >= (int) t.size())
					return "";
				return t[i].name + " (" + std::to_string(t[i].beats) + " beats)";
			}
		};
		int def = hemiola::findCycle("Tintal");
		if (def < 0)
			def = 0;
		configParam<CycleQuantity>(CYCLE_PARAM, 0.f, (float) (hemiola::cycleCount() - 1), (float) def, "Cycle");
		paramQuantities[CYCLE_PARAM]->snapEnabled = true;
		configParam(CV_AMOUNT_PARAM, -6.f, 6.f, 1.f, "CV amount (Level mode)", " steps per volt");
		configParam(GATE_LEN_PARAM, 0.05f, 0.95f, 0.5f, "Gate length", " % of a beat", 0.f, 100.f);
		configSwitch(RUN_PARAM, 0.f, 1.f, 1.f, "Run", {"Stopped", "Running"});
		configButton(RESET_PARAM, "Reset");
		configSwitch(ACCENT_PARAM, 0.f, 1.f, 1.f, "Accent on beat 1", {"Off", "On"});
		configSwitch(CV_MODE_PARAM, 0.f, 1.f, 0.f, "Cycle CV mode", {"Level", "Count"});
		configParam(SWING_PARAM, 0.f, 1.f, 0.f, "Swing (internal clock)", " %", 0.f, 100.f);
		configInput(TEMPO_CV_INPUT, "Tempo CV (1 V per octave, internal clock)");
		configInput(RUN_INPUT, "Run (each pulse switches between running and stopped)");
		configInput(CLOCK_INPUT, "Beat clock (leave empty to use the internal clock)");
		configInput(RESET_INPUT, "Reset (next beat becomes the first beat)");
		configInput(CYCLE_CV_INPUT, "Cycle choice CV");
		configOutput(BEAT_TRIG_OUTPUT, "Beat trigger");
		configOutput(SECTION_TRIG_OUTPUT, "Section start trigger");
		configOutput(CYCLE_TRIG_OUTPUT, "Cycle start trigger");
		configOutput(END_TRIG_OUTPUT, "Cycle end trigger (last beat)");
		configOutput(BEAT_GATE_OUTPUT, "Beat gate");
		configOutput(SECTION_GATE_OUTPUT, "Section start gate");
		configOutput(CYCLE_GATE_OUTPUT, "Cycle start gate");
		configOutput(END_GATE_OUTPUT, "Cycle end gate (last beat)");
		configOutput(POSITION_OUTPUT, "Position in cycle (0 to 10 V)");
		configOutput(CLICK_OUTPUT, "Click (audio)");
	}

	void doReset() {
		engine.reset();
		clock.restart();
		sinceBeat = 0.f;
		beatPeriod = 0.f;
		shownBeat = -1;
	}

	void onReset() override {
		lastRequested = -1;
		counter.reset();
		doReset();
	}

	int chosenCycle() {
		int n = hemiola::cycleCount();
		int knob = hemiola::clampInt((int) std::round(params[CYCLE_PARAM].getValue()), 0, n - 1);
		if (!inputs[CYCLE_CV_INPUT].isConnected()) {
			counter.reset();
			return knob;
		}
		if (params[CV_MODE_PARAM].getValue() > 0.5f) {
			if (cvPulseTrigger.process(inputs[CYCLE_CV_INPUT].getVoltage(), 0.1f, 2.f))
				counter.advance(n);
			return counter.choose(knob, 0, n);
		}
		counter.reset();
		return hemiola::levelChoice((float) knob, inputs[CYCLE_CV_INPUT].getVoltage(), params[CV_AMOUNT_PARAM].getValue(), 0, n - 1);
	}

	void process(const ProcessArgs& args) override {
		int want = chosenCycle();
		if (want != lastRequested) {
			engine.requestCycle(want);
			lastRequested = want;
		}

		bool rst = resetTrigger.process(inputs[RESET_INPUT].getVoltage(), 0.1f, 2.f);
		rst = resetButtonTrigger.process(params[RESET_PARAM].getValue()) || rst;
		if (rst)
			doReset();

		if (runTrigger.process(inputs[RUN_INPUT].getVoltage(), 0.1f, 2.f))
			params[RUN_PARAM].setValue(params[RUN_PARAM].getValue() > 0.5f ? 0.f : 1.f);
		bool running = params[RUN_PARAM].getValue() > 0.5f;
		bool external = inputs[CLOCK_INPUT].isConnected();
		float bpm = params[TEMPO_PARAM].getValue();
		if (inputs[TEMPO_CV_INPUT].isConnected())
			bpm = hemiola::tempoWithCv(bpm, inputs[TEMPO_CV_INPUT].getVoltage());
		bool beatEdge = false;
		if (external)
			beatEdge = clockTrigger.process(inputs[CLOCK_INPUT].getVoltage(), 0.1f, 2.f) && running;
		else if (running)
			beatEdge = clock.process(bpm, params[SWING_PARAM].getValue(), args.sampleTime);

		if (beatEdge) {
			hemiola::CycleTick t = engine.tick();
			if (sinceBeat > 0.f && sinceBeat < 4.f)
				beatPeriod = sinceBeat;
			sinceBeat = 0.f;
			shownBeat = t.beat;

			float period = external ? beatPeriod : 60.f / bpm;
			float g = hemiola::gateSeconds(period, params[GATE_LEN_PARAM].getValue(), 0.25f);
			trig[0].trigger(1e-3f);
			gate[0].trigger(g);
			if (t.sectionStart) {
				trig[1].trigger(1e-3f);
				gate[1].trigger(g);
			}
			if (t.cycleStart) {
				trig[2].trigger(1e-3f);
				gate[2].trigger(g);
			}
			if (t.cycleEnd) {
				trig[3].trigger(1e-3f);
				gate[3].trigger(g);
			}
			click.trigger(params[ACCENT_PARAM].getValue() > 0.5f && t.cycleStart);
		}
		else {
			sinceBeat += args.sampleTime;
		}

		float position = 0.f;
		int fired = engine.lastFiredBeat();
		if (fired >= 0 && engine.beatCount() > 0) {
			float period = external ? beatPeriod : 60.f / bpm;
			float frac = period > 0.f ? clamp(sinceBeat / period, 0.f, 1.f) : 0.f;
			position = clamp(((float) fired + frac) / (float) engine.beatCount(), 0.f, 1.f) * 10.f;
		}

		for (int i = 0; i < 4; i++) {
			outputs[BEAT_TRIG_OUTPUT + i].setVoltage(trig[i].process(args.sampleTime) ? 10.f : 0.f);
			outputs[BEAT_GATE_OUTPUT + i].setVoltage(gate[i].process(args.sampleTime) ? 10.f : 0.f);
		}
		outputs[POSITION_OUTPUT].setVoltage(position);
		outputs[CLICK_OUTPUT].setVoltage(click.process(args.sampleTime));
	}
};

struct CycleDisplay : TransparentWidget {
	CycleClock* module = nullptr;

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer == 1) {
			std::string name, note, figure;
			const std::vector<hemiola::CycleDef>& t = hemiola::cycleTable();
			if (module) {
				int a = module->engine.activeIndex();
				int p = module->engine.pendingIndex();
				if (a < 0 || a >= (int) t.size())
					a = 0;
				name = t[a].name;
				int b = module->shownBeat;
				figure = (b < 0 ? std::string("-") : std::to_string(b + 1)) + " / " + std::to_string(t[a].beats);
				if (p >= 0 && p != a && p < (int) t.size())
					note = "next: " + t[p].name;
				else
					note = hemiola::sectionsText(t[a]);
			}
			else {
				name = "Tintal";
				figure = "5 / 16";
				note = "4+4+4+4";
			}
			hemiola_ui::drawDisplay(args, box, name, figure, note);
		}
		TransparentWidget::drawLayer(args, layer);
	}
};

struct CycleClockWidget : ModuleWidget {
	CycleClockWidget(CycleClock* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/CycleClock.svg")));
		namespace L = CycleClockLayout;
		using namespace hemiola_ui;

		CycleDisplay* d = new CycleDisplay;
		d->module = module;
		d->box.pos = mm2px(Vec(L::displayX, L::displayY));
		d->box.size = mm2px(Vec(L::displayW, L::displayH));
		addChild(d);

		addParam(createParamCentered<HKnob>(mm2px(Vec(L::tempoX, L::tempoY)), module, CycleClock::TEMPO_PARAM));
		addParam(createParamCentered<HKnob>(mm2px(Vec(L::cycleX, L::cycleY)), module, CycleClock::CYCLE_PARAM));
		addParam(createParamCentered<HKnob>(mm2px(Vec(L::cvAmountX, L::cvAmountY)), module, CycleClock::CV_AMOUNT_PARAM));
		addParam(createParamCentered<HKnob>(mm2px(Vec(L::gateLenX, L::gateLenY)), module, CycleClock::GATE_LEN_PARAM));
		addParam(createParamCentered<HButtonLatch>(mm2px(Vec(L::runX, L::runY)), module, CycleClock::RUN_PARAM));
		addParam(createParamCentered<HButtonPress>(mm2px(Vec(L::resetX, L::resetY)), module, CycleClock::RESET_PARAM));
		addParam(createParamCentered<HKnob>(mm2px(Vec(L::swingX, L::swingY)), module, CycleClock::SWING_PARAM));
		addParam(createParamCentered<HToggle>(mm2px(Vec(L::accentX, L::accentY)), module, CycleClock::ACCENT_PARAM));
		addParam(createParamCentered<HToggle>(mm2px(Vec(L::cvModeX, L::cvModeY)), module, CycleClock::CV_MODE_PARAM));

		addInput(createInputCentered<HPortIn>(mm2px(Vec(L::clkInX, L::clkInY)), module, CycleClock::CLOCK_INPUT));
		addInput(createInputCentered<HPortIn>(mm2px(Vec(L::rstInX, L::rstInY)), module, CycleClock::RESET_INPUT));
		addInput(createInputCentered<HPortIn>(mm2px(Vec(L::cvInX, L::cvInY)), module, CycleClock::CYCLE_CV_INPUT));
		addInput(createInputCentered<HPortIn>(mm2px(Vec(L::tempoCvX, L::tempoCvY)), module, CycleClock::TEMPO_CV_INPUT));
		addInput(createInputCentered<HPortIn>(mm2px(Vec(L::runInX, L::runInY)), module, CycleClock::RUN_INPUT));

		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::clickX, L::clickY)), module, CycleClock::CLICK_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::posOutX, L::posOutY)), module, CycleClock::POSITION_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::beatTrigX, L::beatTrigY)), module, CycleClock::BEAT_TRIG_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::sectionTrigX, L::sectionTrigY)), module, CycleClock::SECTION_TRIG_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::cycleTrigX, L::cycleTrigY)), module, CycleClock::CYCLE_TRIG_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::endTrigX, L::endTrigY)), module, CycleClock::END_TRIG_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::beatGateX, L::beatGateY)), module, CycleClock::BEAT_GATE_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::sectionGateX, L::sectionGateY)), module, CycleClock::SECTION_GATE_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::cycleGateX, L::cycleGateY)), module, CycleClock::CYCLE_GATE_OUTPUT));
		addOutput(createOutputCentered<HPortOut>(mm2px(Vec(L::endGateX, L::endGateY)), module, CycleClock::END_GATE_OUTPUT));
	}
};

Model* modelCycleClock = createModel<CycleClock, CycleClockWidget>("CycleClock");
