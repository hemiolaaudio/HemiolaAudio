#!/usr/bin/env python3
"""Generates every piece of artwork from ONE description, so the panels, the control art
and the positions used by the C++ code can never drift apart.

Design: warm paper-white panel, charcoal lettering, one muted blue accent.
Lettering is Montserrat (SIL Open Font Licence), converted to outlines here because
Rack's SVG renderer cannot draw text. The Hemiola Audio logo is the traced vector in
docs/brand/logo_paths.json and sits in the same place on every module.

Writes:
  res/<Module>.svg            panel artwork
  res/components/*.svg        knob, jacks, buttons, switches
  src/PanelLayout.hpp         control positions (mm) used by the C++ code
  docs/preview/<Module>.svg   preview with sample display text (review only)

Run:  python3 tools/make_panels.py      (needs: pip install fonttools)
"""
import json
import os

try:
    import pathops  # optional: removes overlapping outlines so rotated text renders cleanly in Rack
except ImportError:
    pathops = None
from fontTools.pens.svgPathPen import SVGPathPen
from fontTools.pens.transformPen import TransformPen
from fontTools.ttLib import TTFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MM = 75.0 / 25.4          # Rack panel units: 1 mm = 2.952756 px
H = 128.5                 # panel height in mm
HP = 5.08

# Palette
PANEL = "#EEEAE2"
INK = "#2D2C29"
MUTED = "#8A857A"
LINE = "#CBC5B8"
RECESS = "#E3DED2"
ACCENT = "#3F5A73"
NOTE = "#5A574F"
DISC = "#3A3935"

# Shared vertical grid (mm). Every module uses the same rows.
ROW_KNOB = 42.0
ROW_2 = 56.5
ROW_3 = 69.5
ROW_4 = 82.0
ROW_TRIG = 95.0
ROW_GATE = 107.5
FOOTER_RULE = 114.4
LOGO_TOP = 116.0
LOGO_W = 21.0             # same size on every module

LABEL_EM = 2.75           # was 2.35: raised after forum feedback on small text (18 Nov 2026)
LABEL_TRACK = 0.03
LABEL_EM_MIN = 2.4        # long labels shrink to fit their cell, but not below this
LABEL_FACE = "SemiBold"   # heavier than Medium so thin strokes survive when the rack is zoomed out
TITLE_EM = 3.3
TITLE_TRACK = 0.2
SMALL_EM = 2.0            # switch position labels: was 1.85 in pale grey, now darker and heavier
LABEL_DY = {"knob": -8.2, "button": -4.8, "toggle": -5.8, "input": -6.4, "output": -6.4}
# Each control sits in its own pale cell with its label inside the same cell, so a label can
# only ever belong to the control in its own cell. Cell height follows the control.
CAP_HALF = 0.98           # half the cap height of a label, mm
CELL_PAD = 0.5   # default; a module can set its own with "pad"
CELL_GAP = 1.0            # clear space between cells
CELL_FILL = "#E6E2D8"
CELL_W_GAP = 2.4          # side-to-side gap between cells (also the margin at the panel edge)
DN = {"knob": 4.9, "button": 2.6, "toggle": 3.5, "input": 4.0, "output": 4.0}


SIZES = {  # component sizes in mm (width, height)
    "knob": (9.8, 9.8), "input": (8.6, 8.6), "output": (8.6, 8.6),
    "button": (6.4, 6.4), "toggle": (5.0, 8.6),
}

# ---------------------------------------------------------------- modules

def cols(hp, n):
    w = hp * HP
    return [round(w * (2 * i + 1) / (2 * n), 2) for i in range(n)]


C4 = cols(14, 4)
C3 = cols(10, 3)
CLOCK_MID = round(14 * HP / 2, 2)

MODULES = {
    "CycleClock": {
        "hp": 14,
        "title": "CYCLE CLOCK",
        "display": (4.0, 11.5, 14 * HP - 8.0, 15.0),
        "pitch": 14 * HP / 4, "gap": 0.9,
        "sample": ("Tintal", "5 / 16", "4+4+4+4"),
        "items": [
            # name, kind, x, y, label, options
            ("tempo", "knob", C4[0], "A", "TEMPO", {}),
            ("cycle", "knob", C4[1], "A", "CYCLE", {}),
            ("cvAmount", "knob", C4[2], "A", "CV AMT", {"bipolar": True}),
            ("gateLen", "knob", C4[3], "A", "GATE LEN", {}),
            ("run", "button", C4[0], "B", "RUN", {"on": ACCENT}),
            ("reset", "button", C4[1], "B", "RESET", {"on": INK}),
            ("accent", "toggle", C4[2], "B", "ACCENT", {"top": "ON", "bottom": "OFF", "side": "R", "dx": -2.3}),
            ("click", "output", C4[3], "B", "CLICK", {}),
            ("clkIn", "input", C4[0], "C", "CLOCK IN", {}),
            ("rstIn", "input", C4[1], "C", "RESET IN", {}),
            ("cvIn", "input", C4[2], "C", "CYCLE CV", {}),
            ("cvMode", "toggle", C4[3], "C", "CV MODE", {"top": "COUNT", "bottom": "LEVEL", "side": "R", "dx": -3.9}),
            ("tempoCv", "input", C4[0], "D", "TEMPO CV", {}),
            ("runIn", "input", C4[1], "D", "RUN IN", {}),
            ("posOut", "output", C4[2], "D", "POSITION", {}),
            ("swing", "knob", C4[3], "D", "SWING", {}),
            ("beatTrig", "output", C4[0], "E", "BEAT", {}),
            ("sectionTrig", "output", C4[1], "E", "SECTION", {}),
            ("cycleTrig", "output", C4[2], "E", "CYCLE", {}),
            ("endTrig", "output", C4[3], "E", "END", {}),
            ("beatGate", "output", C4[0], "F", "GATE", {}),
            ("sectionGate", "output", C4[1], "F", "GATE", {}),
            ("cycleGate", "output", C4[2], "F", "GATE", {}),
            ("endGate", "output", C4[3], "F", "GATE", {}),
        ],
    },
    "RhythmTalam": {
        "hp": 10,
        "title": "R&T",
        "display": (4.0, 11.5, 10 * HP - 8.0, 15.0),
        "pitch": 10 * HP / 3, "gap": 0.9,
        "sample": ("Chatusra", "4", "per beat"),
        "items": [
            ("division", "knob", C3[0], "A", "DIVISION", {}),
            ("cvAmount", "knob", C3[1], "A", "CV AMT", {"bipolar": True}),
            ("gateLen", "knob", C3[2], "A", "GATE LEN", {}),
            ("divide", "knob", C3[0], "B", "DIVIDE", {}),
            ("prob", "knob", C3[1], "B", "CHANCE", {}),
            ("cvMode", "toggle", C3[2], "B", "CV MODE", {"top": "COUNT", "bottom": "LEVEL", "side": "R", "dx": -3.9}),
            ("clkIn", "input", C3[0], "C", "BEAT IN", {}),
            ("cvIn", "input", C3[1], "C", "CV IN", {}),
            ("divCv", "input", C3[2], "C", "DIVIDE CV", {}),
            ("rstIn", "input", C3[0], "D", "RESET IN", {}),
            ("accent", "toggle", C3[1], "D", "ACCENT", {"top": "ON", "bottom": "OFF", "side": "R", "dx": -2.3}),
            ("click", "output", C3[2], "D", "CLICK", {}),
            ("partsTrig", "output", C3[0], "E", "PARTS", {}),
            ("firstTrig", "output", C3[1], "E", "FIRST", {}),
            ("divTrig", "output", C3[2], "E", "DIVIDED", {}),
            ("partsGate", "output", C3[0], "F", "GATE", {}),
            ("firstGate", "output", C3[1], "F", "GATE", {}),
            ("divGate", "output", C3[2], "F", "GATE", {}),
        ],
    },
}



def resolve(m):
    """Turns row letters into millimetre positions and builds the cells."""
    pad = m.get("pad", CELL_PAD)
    up = {"knob": 8.2, "button": 4.8, "toggle": 5.8, "input": 6.4, "output": 6.4}
    rows = sorted(set(it[3] for it in m["items"]))
    top = m["display"][1] + m["display"][3] + CELL_GAP
    ys, lys, cells = {}, {}, []
    for r in rows:
        its = [it for it in m["items"] if it[3] == r]
        u = max(up[it[1]] for it in its) + CAP_HALF + pad
        d = max(DN[it[1]] for it in its) + pad + 0.3
        ys[r] = round(top + u, 2)
        lys[r] = round(top + pad + CAP_HALF, 2)
        for it in its:
            cells.append((it[2] - (m["pitch"] - CELL_W_GAP) / 2, top, m["pitch"] - CELL_W_GAP, u + d))
        top += u + d + m["gap"]
    m["items"] = [(n, k, x + (o.get("dx", 0.0) if k == "toggle" else 0.0), ys[r], l, dict(o, lx=x, ly=lys[r])) for (n, k, x, r, l, o) in m["items"]]
    m["cells"] = cells
    m["end"] = top - m["gap"]


for _m in MODULES.values():
    resolve(_m)

# ---------------------------------------------------------------- helpers

def px(v):
    return v * MM


def f(v):
    return ("%.2f" % v).rstrip("0").rstrip(".")


def fl(v):
    t = f(v)
    return t if "." in t else t + ".0"


class Face:
    """Turns text into SVG outlines using a TTF file."""

    def __init__(self, path):
        self.font = TTFont(path)
        self.glyphs = self.font.getGlyphSet()
        self.cmap = self.font.getBestCmap()
        self.upem = self.font["head"].unitsPerEm
        self.cap = self.font["OS/2"].sCapHeight

    def width(self, text, em_mm, track):
        s = px(em_mm) / self.upem
        total = 0.0
        for ch in text:
            g = self.cmap.get(ord(ch))
            if g:
                total += self.font["hmtx"][g][0] * s + track * px(em_mm)
        return (total - track * px(em_mm)) / MM

    def path(self, text, x_mm, y_mm, em_mm, anchor="c", track=0.0, fill=INK):
        s = px(em_mm) / self.upem
        w_px = self.width(text, em_mm, track) * MM
        x = px(x_mm)
        if anchor == "c":
            x -= w_px / 2
        elif anchor == "r":
            x -= w_px
        base = px(y_mm) + self.cap * s / 2
        # One <path> per letter. Rack decides what is a hole by counting crossings with every other contour in the
        # same shape (even-odd, no winding), so letters sharing a path can turn each other's counters solid.
        out = []
        for ch in text:
            g = self.cmap.get(ord(ch))
            if not g:
                continue
            pen = SVGPathPen(self.glyphs, ntos=lambda v: ("%.2f" % v).rstrip("0").rstrip("."))
            tp = TransformPen(pen, (s, 0, 0, -s, x, base))
            self._draw_clean(g, tp)
            cmds = pen.getCommands()
            if cmds:
                out.append('<path d="%s" fill="%s" fill-rule="evenodd"/>' % (cmds, fill))
            x += self.font["hmtx"][g][0] * s + track * px(em_mm)
        return "\n".join(out)

    def _draw_clean(self, g, tp):
        if not pathops:
            raise SystemExit('skia-pathops is missing. Run: pip3 install --user skia-pathops (letters fill in in Rack without it).')
        if pathops:
            clean = pathops.Path()
            self.glyphs[g].draw(clean.getPen(glyphSet=self.glyphs))
            clean.simplify(fix_winding=True, keep_starting_points=False)
            clean.draw(tp)
        else:
            self.glyphs[g].draw(tp)


    def width_px(self, text, em_mm, track):
        return self.width(text, em_mm, track) * MM

    def path_up(self, text, cx_mm, y_start_mm, em_mm, track=0.0, fill=INK):
        """Text turned a quarter turn, reading from the bottom upwards. cx is the middle of the capital letters,
        y_start is where the first letter begins (the bottom end)."""
        s = px(em_mm) / self.upem
        e = px(cx_mm) + self.cap * s / 2
        y = px(y_start_mm)
        out = []
        for ch in text:
            g = self.cmap.get(ord(ch))
            if not g:
                continue
            pen = SVGPathPen(self.glyphs, ntos=lambda v: ("%.2f" % v).rstrip("0").rstrip("."))
            tp = TransformPen(pen, (0, -s, -s, 0, e, y))
            self._draw_clean(g, tp)
            cmds = pen.getCommands()
            if cmds:
                out.append('<path d="%s" fill="%s" fill-rule="evenodd"/>' % (cmds, fill))
            y -= self.font["hmtx"][g][0] * s + track * px(em_mm)
        return "\n".join(out)


FACES = {}


def face(name):
    if name not in FACES:
        FACES[name] = Face(os.path.join(ROOT, "res", "fonts", "Montserrat-%s.ttf" % name))
    return FACES[name]


def circle(cx, cy, r, fill="none", stroke=None, sw=0.3):
    s = ' stroke="%s" stroke-width="%s"' % (stroke, f(px(sw))) if stroke else ""
    return '<circle cx="%s" cy="%s" r="%s" fill="%s"%s/>' % (f(px(cx)), f(px(cy)), f(px(r)), fill, s)


def rect(x, y, w, h, fill="none", rx=0.0, stroke=None, sw=0.3):
    s = ' stroke="%s" stroke-width="%s"' % (stroke, f(px(sw))) if stroke else ""
    r = ' rx="%s"' % f(px(rx)) if rx else ""
    return '<rect x="%s" y="%s" width="%s" height="%s"%s fill="%s"%s/>' % (f(px(x)), f(px(y)), f(px(w)), f(px(h)), r, fill, s)


def line(x1, y1, x2, y2, stroke=LINE, sw=0.25):
    return '<line x1="%s" y1="%s" x2="%s" y2="%s" stroke="%s" stroke-width="%s"/>' % (
        f(px(x1)), f(px(y1)), f(px(x2)), f(px(y2)), stroke, f(px(sw)))


def svg_wrap(w_mm, h_mm, body):
    w, h = px(w_mm), px(h_mm)
    return ('<?xml version="1.0" encoding="UTF-8"?>\n<svg xmlns="http://www.w3.org/2000/svg" width="%spx" height="%spx" '
            'viewBox="0 0 %s %s">\n%s\n</svg>\n' % (f(w), f(h), f(w), f(h), "\n".join(body)))

# ---------------------------------------------------------------- component art (centred at ox, oy in mm)

def art_knob(ox, oy, turn=0.0):
    import math
    out = [circle(ox, oy, 4.75, fill="#D9D3C6"), circle(ox, oy, 4.2, fill=DISC),
           circle(ox, oy, 3.0, stroke="#4C4B46", sw=0.2)]
    a = math.radians(turn)
    x1, y1 = ox + 1.4 * math.sin(a), oy - 1.4 * math.cos(a)
    x2, y2 = ox + 3.7 * math.sin(a), oy - 3.7 * math.cos(a)
    out.append('<line x1="%s" y1="%s" x2="%s" y2="%s" stroke="%s" stroke-width="%s" stroke-linecap="round"/>' % (
        f(px(x1)), f(px(y1)), f(px(x2)), f(px(y2)), PANEL, f(px(0.75))))
    return out


def art_input(ox, oy):
    return [circle(ox, oy, 3.9, fill=PANEL, stroke=INK, sw=0.35), circle(ox, oy, 2.8, stroke=MUTED, sw=0.25),
            circle(ox, oy, 1.9, fill=INK)]


def art_output(ox, oy):
    return [circle(ox, oy, 4.0, fill=INK), circle(ox, oy, 2.7, stroke="#CFC9BB", sw=0.35),
            circle(ox, oy, 1.8, fill="#151412")]


def art_button(ox, oy, on=None):
    if on:
        return [circle(ox, oy, 2.6, fill=on, stroke=on, sw=0.35)]
    return [circle(ox, oy, 2.6, fill="#E7E2D8", stroke=INK, sw=0.35)]


def art_toggle(ox, oy, up):
    out = [rect(ox - 1.6, oy - 3.5, 3.2, 7.0, fill="#DAD5C8", rx=1.6, stroke=LINE, sw=0.25)]
    if up:
        out.append(circle(ox, oy - 1.95, 1.25, fill=ACCENT))
    else:
        out.append(circle(ox, oy + 1.95, 1.25, fill=INK))
    return out


def component_files():
    files = {}
    w, h = SIZES["knob"]
    files["components/Knob.svg"] = svg_wrap(w, h, art_knob(w / 2, h / 2))
    w, h = SIZES["input"]
    files["components/PortIn.svg"] = svg_wrap(w, h, art_input(w / 2, h / 2))
    w, h = SIZES["output"]
    files["components/PortOut.svg"] = svg_wrap(w, h, art_output(w / 2, h / 2))
    w, h = SIZES["button"]
    files["components/ButtonOff.svg"] = svg_wrap(w, h, art_button(w / 2, h / 2))
    files["components/ButtonAccent.svg"] = svg_wrap(w, h, art_button(w / 2, h / 2, ACCENT))
    files["components/ButtonInk.svg"] = svg_wrap(w, h, art_button(w / 2, h / 2, INK))
    w, h = SIZES["toggle"]
    files["components/ToggleDown.svg"] = svg_wrap(w, h, art_toggle(w / 2, h / 2, False))
    files["components/ToggleUp.svg"] = svg_wrap(w, h, art_toggle(w / 2, h / 2, True))
    return files

# ---------------------------------------------------------------- panel

def logo_body(cx_mm, top_mm, width_mm):
    d = json.load(open(os.path.join(ROOT, "docs", "brand", "logo_paths.json")))
    k = width_mm / d["w"]
    x0 = cx_mm - width_mm / 2
    parts = []
    for p in d["paths"]:
        pts = ["%s %s" % (f(px(x0 + a * k)), f(px(top_mm + b * k))) for a, b in p]
        parts.append("M" + " L".join(pts) + " Z")
    height = d["h"] * k
    return '<path d="%s" fill="%s" fill-rule="evenodd"/>' % (" ".join(parts), INK), height


def panel_body(m, preview=False):
    w = m["hp"] * HP
    out = [rect(0, 0, w, H, fill=PANEL)]
    out.append(face("Medium").path(m["title"], w / 2, 6.0, TITLE_EM, "c", TITLE_TRACK))
    out.append(line(4.0, 9.6, w - 4.0, 9.6))
    dx, dy, dw, dh = m["display"]
    out.append(rect(dx, dy, dw, dh, fill=RECESS, rx=1.2, stroke=LINE, sw=0.25))
    med = face("Medium")
    lab = face(LABEL_FACE)
    reg = face("Regular")
    import math
    for (cx_, cy_, cw_, ch_) in m["cells"]:
        out.append(rect(cx_, cy_, cw_, ch_, fill=CELL_FILL, rx=1.0))
    for (name, kind, x, y, label, opt) in m["items"]:
        if kind == "knob":
            r = 6.0
            s0, s1 = math.radians(-135), math.radians(135)
            sx, sy = x + r * math.sin(s0), y - r * math.cos(s0)
            ex, ey = x + r * math.sin(s1), y - r * math.cos(s1)
            out.append('<path d="M%s %s A%s %s 0 1 1 %s %s" fill="none" stroke="%s" stroke-width="%s"/>' % (
                f(px(sx)), f(px(sy)), f(px(r)), f(px(r)), f(px(ex)), f(px(ey)), MUTED, f(px(0.25))))
            out.append(circle(sx, sy, 0.35, fill=MUTED))
            out.append(circle(ex, ey, 0.35, fill=MUTED))
            if opt.get("bipolar"):
                out.append(line(x, y - r - 0.7, x, y - r + 0.7, stroke=MUTED, sw=0.3))
        if kind == "toggle":
            side = opt["side"]
            tx = x + 3.0 if side == "R" else x - 3.0
            anchor = "l" if side == "R" else "r"
            out.append(med.path(opt["top"], tx, y - 1.95, SMALL_EM, anchor, 0.08, NOTE))
            out.append(med.path(opt["bottom"], tx, y + 1.95, SMALL_EM, anchor, 0.08, NOTE))
        em = LABEL_EM
        host = [c for c in m["cells"] if c[0] <= x <= c[0] + c[2] and c[1] <= y <= c[1] + c[3]]
        if host:   # shrink a long label to fit its own cell, never below the floor
            lw = lab.width(label, LABEL_EM, LABEL_TRACK)
            if lw > host[0][2] - 0.8: em = max(LABEL_EM_MIN, LABEL_EM * (host[0][2] - 0.8) / lw)
        out.append(lab.path(label, opt.get("lx", x), opt.get("ly", y + LABEL_DY[kind]), em, "c", LABEL_TRACK))
    # locked brand footer: identical size and position on every module
    out.append(line(4.0, FOOTER_RULE, w - 4.0, FOOTER_RULE))
    body, _ = logo_body(w / 2, LOGO_TOP, LOGO_W)
    out.append(body)
    if preview:
        for (name, kind, x, y, label, opt) in m["items"]:
            if kind == "knob":
                out += art_knob(x, y, turn=-45 if opt.get("bipolar") is None else 0)
            elif kind == "input":
                out += art_input(x, y)
            elif kind == "output":
                out += art_output(x, y)
            elif kind == "button":
                out += art_button(x, y, opt["on"] if name == "run" else None)
            elif kind == "toggle":
                out += art_toggle(x, y, name == "accent")
        l1, l2, l3 = m["sample"]
        out.append(med.path(l1.upper(), dx + 3.0, dy + 4.4, 4.4, "l", 0.06, INK))
        out.append(med.path(l2, dx + dw - 3.0, dy + 4.4, 4.4, "r", 0.06, ACCENT))
        out.append(reg.path(l3, dx + 3.0, dy + 10.6, 4.4, "l", 0.06, NOTE))
    return out



# ---------------------------------------------------------------- Snap (2 HP utility, hand-laid)

SNAP = {"hp": 2, "buttonY": 15.0, "nameCentreY": 62.0, "nameEm": 5.2, "nameTrack": 0.32}


def logo_mark_body(cx_mm, top_mm, width_mm):
    """Only the crescent mark of the logo (the first four shapes). The wordmark would be too small to read at 2 HP."""
    d = json.load(open(os.path.join(ROOT, "docs", "brand", "logo_paths.json")))
    marks = d["paths"][:4]
    x_lo = min(a for p in marks for a, b in p)
    x_hi = max(a for p in marks for a, b in p)
    y_lo = min(b for p in marks for a, b in p)
    y_hi = max(b for p in marks for a, b in p)
    k = width_mm / (x_hi - x_lo)
    parts = []
    for p in marks:
        pts = ["%s %s" % (f(px(cx_mm - width_mm / 2 + (a - x_lo) * k)), f(px(top_mm + (b - y_lo) * k))) for a, b in p]
        parts.append("M" + " L".join(pts) + " Z")
    return '<path d="%s" fill="%s" fill-rule="evenodd"/>' % (" ".join(parts), INK), (y_hi - y_lo) * k


def snap_panel_body(preview=False):
    w = SNAP["hp"] * HP
    cx = w / 2
    out = [rect(0, 0, w, H, fill=PANEL)]
    by = SNAP["buttonY"]
    out.append(rect(1.2, by - 7.2, w - 2.4, 14.4, fill=CELL_FILL, rx=1.0))
    out.append(line(2.4, 27.0, w - 2.4, 27.0))
    name = "SNAP"
    med = face("Medium")
    length = med.width(name, SNAP["nameEm"], SNAP["nameTrack"])
    out.append(med.path_up(name, cx, SNAP["nameCentreY"] + length / 2, SNAP["nameEm"], SNAP["nameTrack"]))
    brand = "HEMIOLA AUDIO"
    reg = face("Regular")
    out.append(reg.path_up(brand, cx, 109.5, 1.9, 0.14, MUTED))
    out.append(line(2.4, FOOTER_RULE, w - 2.4, FOOTER_RULE))
    body, _ = logo_mark_body(cx, 117.6, 6.6)
    out.append(body)
    if preview:
        out += art_button(cx, by)
    return out



def layout_header():
    o = ["// GENERATED by tools/make_panels.py. Do not edit by hand.",
         "// Control positions are in millimetres from the top-left of the panel.",
         "#pragma once", ""]
    for name, m in MODULES.items():
        o.append("namespace %sLayout {" % name)
        o.append("const float panelWidthMm = %sf;" % fl(m["hp"] * HP))
        dx, dy, dw, dh = m["display"]
        o.append("const float displayX = %sf, displayY = %sf, displayW = %sf, displayH = %sf;" % (fl(dx), fl(dy), fl(dw), fl(dh)))
        for (iname, kind, x, y, label, opt) in m["items"]:
            o.append("const float %sX = %sf, %sY = %sf;" % (iname, fl(x), iname, fl(y)))
        o.append("}  // namespace %sLayout" % name)
        o.append("")
    o.append("namespace SnapLayout {")
    o.append("const float panelWidthMm = %sf;" % fl(SNAP["hp"] * HP))
    o.append("const float buttonX = %sf, buttonY = %sf;" % (fl(SNAP["hp"] * HP / 2), fl(SNAP["buttonY"])))
    o.append("}  // namespace SnapLayout")
    o.append("")
    return "\n".join(o)


def main():
    for d in ("res/components", "docs/preview", "src", "unreleased/snap"):
        os.makedirs(os.path.join(ROOT, d), exist_ok=True)
    for rel, text in component_files().items():
        open(os.path.join(ROOT, "res", rel), "w").write(text)
    for name, m in MODULES.items():
        w = m["hp"] * HP
        open(os.path.join(ROOT, "res", name + ".svg"), "w").write(svg_wrap(w, H, panel_body(m, False)))
        open(os.path.join(ROOT, "docs", "preview", name + ".svg"), "w").write(svg_wrap(w, H, panel_body(m, True)))
    sw = SNAP["hp"] * HP
    open(os.path.join(ROOT, "unreleased", "snap", "Snap.svg"), "w").write(svg_wrap(sw, H, snap_panel_body(False)))
    open(os.path.join(ROOT, "unreleased", "snap", "Snap-preview.svg"), "w").write(svg_wrap(sw, H, snap_panel_body(True)))
    open(os.path.join(ROOT, "src", "PanelLayout.hpp"), "w").write(layout_header())
    print("artwork written")


if __name__ == "__main__":
    main()
