#pragma once
#include "plugin.hpp"
#include <cctype>

namespace hemiola_ui {

inline std::shared_ptr<Svg> art(const char* name) {
	return Svg::load(asset::plugin(pluginInstance, std::string("res/components/") + name));
}

struct HKnob : app::SvgKnob {
	HKnob() {
		minAngle = -0.75f * M_PI;
		maxAngle = 0.75f * M_PI;
		setSvg(art("Knob.svg"));
		if (shadow)
			shadow->opacity = 0.f;
	}
};

struct HPortIn : app::SvgPort {
	HPortIn() { setSvg(art("PortIn.svg")); }
};

struct HPortOut : app::SvgPort {
	HPortOut() { setSvg(art("PortOut.svg")); }
};

// Latching button, shows the accent colour while on.
struct HButtonLatch : app::SvgSwitch {
	HButtonLatch() {
		addFrame(art("ButtonOff.svg"));
		addFrame(art("ButtonAccent.svg"));
		if (shadow)
			shadow->opacity = 0.f;
	}
};

// Momentary button, fills while pressed.
struct HButtonPress : app::SvgSwitch {
	HButtonPress() {
		momentary = true;
		addFrame(art("ButtonOff.svg"));
		addFrame(art("ButtonInk.svg"));
		if (shadow)
			shadow->opacity = 0.f;
	}
};

// Two-position switch: value 0 = thumb down, value 1 = thumb up.
struct HToggle : app::SvgSwitch {
	HToggle() {
		addFrame(art("ToggleDown.svg"));
		addFrame(art("ToggleUp.svg"));
		if (shadow)
			shadow->opacity = 0.f;
	}
};

inline std::string upper(std::string s) {
	for (size_t i = 0; i < s.size(); i++)
		s[i] = (char) std::toupper((unsigned char) s[i]);
	return s;
}

// Draws one centred line of text. emMm is the font size in millimetres, yMm the middle of the capital letters.
inline void drawLine(const Widget::DrawArgs& args, std::shared_ptr<window::Font> font, float cx, float yMm, float emMm,
                     float r, float g, float b, const char* text) {
	float px = emMm * 2.952756f;
	nvgFontFaceId(args.vg, font->handle);
	nvgFontSize(args.vg, px);
	nvgTextLetterSpacing(args.vg, px * 0.06f);
	nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_BASELINE);
	nvgFillColor(args.vg, nvgRGBf(r, g, b));
	nvgText(args.vg, cx, mm2px(yMm + 0.35f * emMm), text, NULL);
}

// Display text inside the recess: a name, a large figure, and a small note.
inline void drawDisplay(const Widget::DrawArgs& args, Rect box, const std::string& name, const std::string& figure, const std::string& note) {
	std::shared_ptr<window::Font> medium = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Montserrat-Medium.ttf"));
	std::shared_ptr<window::Font> light = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Montserrat-Light.ttf"));
	std::shared_ptr<window::Font> regular = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Montserrat-Regular.ttf"));
	if (!medium || !light || !regular)
		return;
	float cx = box.size.x / 2.f;
	drawLine(args, medium, cx, 3.8f, 2.5f, 0.176f, 0.172f, 0.161f, upper(name).c_str());
	drawLine(args, light, cx, 8.6f, 5.0f, 0.247f, 0.353f, 0.451f, figure.c_str());
	drawLine(args, regular, cx, 13.2f, 2.1f, 0.541f, 0.522f, 0.478f, note.c_str());
}

}  // namespace hemiola_ui
