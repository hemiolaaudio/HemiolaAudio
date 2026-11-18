#!/usr/bin/env python3
"""WHOA! panel (25 HP, 7 columns) in the Hemiola style. Reuses the helpers in make_panels.py.
Five sections, each with its own tint and a coloured title tab in the side margin:
VOICE, ENVELOPES, DELAY, REVERB, OUT. Every cell is used. Writes res/Whoa.svg, docs/preview/Whoa.svg, src/WhoaLayout.hpp"""
import sys, os, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
import make_panels as mp
from make_panels import *

HPN = 25
W = HPN * HP                      # 127.0 mm
PITCH = 17.0
CW = 16.4                         # cell width (gap 0.6 mm)
cx = [W / 2 + (i - 3) * PITCH for i in range(7)]
KH, JH, GAP, TOP = 14.7, 13.0, 0.9, 10.6

# section: (title, cell tint, tab tint, side)
SEC = {
    "VOICE":  ("VOICE",     "#E6E2D8", "#D5CFBF", "L"),
    "ENV":    ("ENVELOPES", "#DDE4D2", "#C6D3B8", "L"),
    "DELAY":  ("DELAY",     "#D9E3E8", "#BED2DC", "R"),
    "REVERB": ("REVERB",    "#E0DEEE", "#C9C5E0", "R"),
    "OUT":    ("OUT",       "#D9D5CB", "#BDB7A6", "R"),
}

def stack(kinds, top, dk=0.0, dj=0.0):
    """Return [(kind, centre_y, top_y, height)] for a column of cell kinds. dk, dj stretch K and J cells."""
    out, t = [], top
    for k in kinds:
        if k == "K": out.append((k, t + 9.3 + dk / 2, t, KH + dk)); t += KH + dk + GAP
        else:        out.append((k, t + 8.5 + dj / 2, t, JH + dj)); t += JH + dj + GAP
    return out

# left side: Voice (2 rows) then a 5-row stack. Right zone has a different row pattern but the same total height.
# The envelope rows (r2..r5) are stretched so the left column ends level with the right zone (no empty cells).
_top = stack(["K", "J"], TOP)
_t2 = _top[1][2] + JH + GAP
LEFT = _top + stack(["K", "J", "K", "J"], _t2, 3.9, 3.9)         # r0..r5
RZ = stack(["K", "J", "K", "K", "J"], LEFT[2][2])               # right zone, starts level with r2

# name, kind, col, (side, row), label, section
def L(r): return ("L", r)
def R(r): return ("R", r)
CELLS = [
 ("RATIO", "K", 0, L(0), "RATIO", "VOICE"), ("FINE", "K", 1, L(0), "FINE", "VOICE"), ("INDEX", "K", 2, L(0), "INDEX", "VOICE"),
 ("FEEDBACK", "K", 3, L(0), "FEEDBACK", "VOICE"), ("ENVAMT", "K", 4, L(0), "ENV AMT", "VOICE"),
 ("DRIVE", "K", 5, L(0), "DRIVE", "VOICE"), ("SPREAD", "K", 6, L(0), "SPREAD", "VOICE"),
 ("VOCT", "J", 0, L(1), "V/OCT", "VOICE"), ("GATE", "J", 1, L(1), "GATE", "VOICE"), ("GATE_M", "J", 2, L(1), "MOD GATE", "VOICE"),
 ("GATE_C", "J", 3, L(1), "CAR GATE", "VOICE"), ("CV_INDEX", "J", 4, L(1), "INDEX CV", "VOICE"),
 ("CV_FB", "J", 5, L(1), "FB CV", "VOICE"), ("ACCENT", "J", 6, L(1), "ACCENT", "VOICE"),
 ("MOD_A", "K", 0, L(2), "MOD A", "ENV"), ("MOD_D", "K", 1, L(2), "MOD D", "ENV"), ("MOD_S", "K", 2, L(2), "MOD S", "ENV"), ("MOD_R", "K", 3, L(2), "MOD R", "ENV"),
 ("CV_MA", "J", 0, L(3), "A CV", "ENV"), ("CV_MD", "J", 1, L(3), "D CV", "ENV"), ("CV_MS", "J", 2, L(3), "S CV", "ENV"), ("CV_MR", "J", 3, L(3), "R CV", "ENV"),
 ("CAR_A", "K", 0, L(4), "CAR A", "ENV"), ("CAR_D", "K", 1, L(4), "CAR D", "ENV"), ("CAR_S", "K", 2, L(4), "CAR S", "ENV"), ("CAR_R", "K", 3, L(4), "CAR R", "ENV"),
 ("CV_CA", "J", 0, L(5), "A CV", "ENV"), ("CV_CD", "J", 1, L(5), "D CV", "ENV"), ("CV_CS", "J", 2, L(5), "S CV", "ENV"), ("CV_CR", "J", 3, L(5), "R CV", "ENV"),
 ("DLY_TIME", "K", 4, R(0), "DLY TIME", "DELAY"), ("DLY_FB", "K", 5, R(0), "DLY FDBK", "DELAY"), ("DLY_MIX", "K", 6, R(0), "DLY MIX", "DELAY"),
 ("CLOCK", "J", 4, R(1), "CLOCK IN", "DELAY"), ("CV_DFB", "J", 5, R(1), "FDBK CV", "DELAY"), ("CV_DMIX", "J", 6, R(1), "MIX CV", "DELAY"),
 ("REV_TYPE", "K", 4, R(2), "REV TYPE", "REVERB"), ("REV_SIZE", "K", 5, R(2), "REV SIZE", "REVERB"), ("REV_DECAY", "K", 6, R(2), "REV DECAY", "REVERB"),
 ("REV_TONE", "K", 4, R(3), "REV TONE", "REVERB"), ("REV_MIX", "K", 5, R(3), "REV MIX", "REVERB"), ("REV_PRE", "K", 6, R(3), "PRE DELAY", "REVERB"),
 ("OUT_L", "J", 4, R(4), "LEFT", "OUT"), ("OUT_R", "J", 5, R(4), "RIGHT", "OUT"), ("GATE_OUT", "J", 6, R(4), "GATE OUT", "OUT"),
]
K_NAMES = [c for c in CELLS if c[1] == "K"]
J_OUT = {"OUT_L", "OUT_R", "GATE_OUT"}

def fit(face_, label):
    """Label size: the shared size, shrunk if needed to fit the cell, never below the shared floor."""
    w = face_.width(label, LABEL_EM, LABEL_TRACK)
    return LABEL_EM if w <= CW - 0.8 else max(mp.LABEL_EM_MIN, LABEL_EM * (CW - 0.8) / w)

def pos(c):
    name, kind, col, (side, r), label, sec = c
    row = (LEFT if side == "L" else RZ)[r]
    return cx[col], row[1], row[2], row[3]

def body(preview):
    out = [rect(0, 0, W, H, fill=PANEL)]
    med = face("Medium")
    lab = face(mp.LABEL_FACE)
    out.append(med.path("WHOA!", W / 2, 6.0, TITLE_EM, "c", TITLE_TRACK))
    out.append(line(4.0, 9.6, W - 4.0, 9.6))
    widths = []
    spans = {}
    for c in CELLS:
        name, kind, col, src, label, sec = c
        x, y, top, h = pos(c)
        tint = SEC[sec][1]
        out.append(rect(x - CW / 2, top, CW, h, fill=tint, rx=1.0))
        if src[0] == SEC[sec][3]:
            s = spans.setdefault(sec, [1e9, -1e9]); s[0] = min(s[0], top); s[1] = max(s[1], top + h)
        if kind == "K":
            rr = 6.0
            s0, s1 = math.radians(-135), math.radians(135)
            sx, sy = x + rr * math.sin(s0), y - rr * math.cos(s0)
            ex, ey = x + rr * math.sin(s1), y - rr * math.cos(s1)
            out.append('<path d="M%s %s A%s %s 0 1 1 %s %s" fill="none" stroke="%s" stroke-width="%s"/>' % (
                f(px(sx)), f(px(sy)), f(px(rr)), f(px(rr)), f(px(ex)), f(px(ey)), MUTED, f(px(0.25))))
            out.append(lab.path(label, x, y - 7.9, fit(lab, label), "c", LABEL_TRACK))
            if preview: out += art_knob(x, y, turn=-45)
        else:
            out.append(lab.path(label, x, y - 6.4, fit(lab, label), "c", LABEL_TRACK))
            if preview: out += (art_output(x, y) if name in J_OUT else art_input(x, y))
        widths.append((label, lab.width(label, fit(lab, label), LABEL_TRACK)))
    # section tabs in the side margins
    tabw = 3.0
    for sec, (title, tint, tab, side) in SEC.items():
        a, b = spans[sec]
        x0 = 0.9 if side == "L" else W - 0.9 - tabw
        out.append(rect(x0, a, tabw, b - a, fill=tab, rx=1.0))
        tw = lab.width(title, LABEL_EM, LABEL_TRACK)
        out.append(lab.path_up(title, x0 + tabw / 2, (a + b) / 2 + tw / 2, LABEL_EM, LABEL_TRACK, INK))
        if tw > (b - a) - 1.0: print("TAB TOO LONG", title, round(tw, 1), round(b - a, 1))
    out.append(line(4.0, FOOTER_RULE, W - 4.0, FOOTER_RULE))
    b, _ = logo_body(W / 2, LOGO_TOP, LOGO_W)
    out.append(b)
    body.widths = widths; body.spans = spans
    return out

def hpp():
    o = ["// GENERATED by make_whoa_panel.py. Do not edit by hand.", "#pragma once", "namespace whoa_layout {"]
    for c in CELLS:
        x, y, top, h = pos(c)
        o.append("constexpr float %sX = %.2ff, %sY = %.2ff;" % (c[0], x, c[0], y))
    o.append("}")
    return "\n".join(o) + "\n"

if __name__ == "__main__":
    os.chdir(ROOT_DIR)
    os.makedirs("docs/preview", exist_ok=True)
    open("res/Whoa.svg", "w").write(svg_wrap(W, H, body(False)))
    open("docs/preview/Whoa.svg", "w").write(svg_wrap(W, H, body(True)))
    open("src/WhoaLayout.hpp", "w").write(hpp())
    wide = [(l, round(w, 1)) for l, w in body.widths if w > CW - 0.8]
    print("labels wider than cell-0.8mm:", wide)
    print("left bottom", round(LEFT[5][2] + LEFT[5][3], 2), "right bottom", round(RZ[4][2] + RZ[4][3], 2), "footer", FOOTER_RULE)
    print("cells", len(CELLS), "knobs", len(K_NAMES), {k: [round(v, 1) for v in s] for k, s in body.spans.items()})
