#pragma once
#include "plugin.hpp"
#include <algorithm>
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

// Width in pixels of a line of text at the given size, with the same letter spacing drawLine uses.
inline float textWidth(const Widget::DrawArgs& args, std::shared_ptr<window::Font> font, float px, const char* text) {
	nvgFontFaceId(args.vg, font->handle);
	nvgFontSize(args.vg, px);
	nvgTextLetterSpacing(args.vg, px * 0.06f);
	float bounds[4];
	nvgTextBounds(args.vg, 0.f, 0.f, text, NULL, bounds);
	return bounds[2] - bounds[0];
}

// Draws one line of text aligned to x (pixels), shrinking it if it would be wider than maxWidth (pixels).
// emMm is the preferred size in millimetres and minEmMm the smallest size allowed. yMm is the middle of the capitals.
inline void drawFitted(const Widget::DrawArgs& args, std::shared_ptr<window::Font> font, float x, int align, float yMm,
                       float emMm, float minEmMm, float maxWidth, float r, float g, float b, const std::string& text) {
	float em = emMm;
	float w = textWidth(args, font, em * 2.952756f, text.c_str());
	if (w > maxWidth && w > 0.f)
		em = std::max(minEmMm, em * maxWidth / w);
	float px = em * 2.952756f;
	nvgFontFaceId(args.vg, font->handle);
	nvgFontSize(args.vg, px);
	nvgTextLetterSpacing(args.vg, px * 0.06f);
	nvgTextAlign(args.vg, align | NVG_ALIGN_BASELINE);
	nvgFillColor(args.vg, nvgRGBf(r, g, b));
	nvgText(args.vg, x, mm2px(yMm + 0.35f * em), text.c_str(), NULL);
}

// Display text inside the recess, in two lines of the same size. Line one: the name on the left and the main figure on
// the right. Line two: a note, in a darker grey than before so it stays readable when the rack is zoomed out.
inline void drawDisplay(const Widget::DrawArgs& args, Rect box, const std::string& name, const std::string& figure, const std::string& note) {
	std::shared_ptr<window::Font> medium = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Montserrat-Medium.ttf"));
	std::shared_ptr<window::Font> regular = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Montserrat-Regular.ttf"));
	if (!medium || !regular)
		return;
	const float em = 4.4f, minEm = 3.0f;
	float pad = mm2px(3.0f);
	float avail = box.size.x - 2.f * pad;
	float gap = mm2px(2.5f);
	float figW = textWidth(args, medium, em * 2.952756f, figure.c_str());
	drawFitted(args, medium, box.size.x - pad, NVG_ALIGN_RIGHT, 4.4f, em, minEm, avail, 0.247f, 0.353f, 0.451f, figure);
	drawFitted(args, medium, pad, NVG_ALIGN_LEFT, 4.4f, em, minEm, avail - figW - gap, 0.176f, 0.172f, 0.161f, upper(name));
	drawFitted(args, regular, pad, NVG_ALIGN_LEFT, 10.6f, em, minEm, avail, 0.353f, 0.341f, 0.310f, note);
}

}  // namespace hemiola_ui
