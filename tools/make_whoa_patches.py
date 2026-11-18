#!/usr/bin/env python3
"""Builds the two Whoa showcase patches into patches/. Plain JSON, no Rack needed.
Whoa port ids: inputs VOCT0 GATE_M1 GATE_C2 GATE3 CV_INDEX4 CV_FB5 ACCENT6 CV_MA7 CV_MD8 CV_MS9 CV_MR10
CV_CA11 CV_CD12 CV_CS13 CV_CR14 CLOCK15 CV_DFB16 CV_DMIX17; outputs L0 R1 GATE_OUT2.
Whoa params: 0 RATIO idx, 1 FINE, 2 INDEX, 3 FEEDBACK, 4-7 MOD A D S R, 8-11 CAR A D S R, 12 DRIVE, 13 SPREAD,
14 ENVAMT, 15 REV PRE-DELAY, 16 DLY TIME idx, 17 DLY FB, 18 DLY MIX, 19 REV TYPE, 20 SIZE, 21 DECAY, 22 TONE, 23 MIX.
"""
import json, os
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
COLORS = ['#ffb437', '#f3374b', '#00b56e', '#3695ef', '#a924bb']
V = {'HemiolaAudio': '2.1.0', 'Fundamental': '2.6.4', 'Core': '2.6.6'}

class Patch:
    def __init__(s): s.mods = []; s.cables = []; s.nid = 1
    def add(s, plugin, model, pos, params=None, data=None):
        m = {'id': s.nid, 'plugin': plugin, 'model': model, 'version': V[plugin],
             'params': [{'id': k, 'value': v} for k, v in (params or {}).items()], 'pos': list(pos)}
        if data is not None: m['data'] = data
        s.mods.append(m); s.nid += 1; return m['id']
    def cable(s, a, ao, b, bi):
        s.cables.append({'id': len(s.cables), 'outputModuleId': a, 'outputId': ao, 'inputModuleId': b, 'inputId': bi,
                         'color': COLORS[len(s.cables) % len(COLORS)]})
    def save(s, name, master):
        d = {'version': '2.6.6', 'zoom': 0.55, 'gridOffset': [0.0, 0.0], 'modules': s.mods, 'cables': s.cables, 'masterModuleId': master}
        with open(os.path.join(ROOT, 'patches', name), 'w') as f: json.dump(d, f, indent=2)

CVD = {'cvVoice': 0.5, 'cvEnv': 0.5, 'cvDelay': 0.5}


def seq3(p, pos, steps, vals, gates):
    prm = {0: 1.0, 3: steps}
    for i, v in enumerate(vals): prm[4 + i] = v
    g = [1 if (i < len(gates) and gates[i]) else 0 for i in range(8)]
    return p.add('Fundamental', 'SEQ3', pos, prm, {'running': True, 'gates': g, 'clockPassthrough': False})

def knob_time(sec): return __import__('math').log(sec / 0.001) / __import__('math').log(5000.0)

# ------------------------------------------------------------------ 1. Dark Ambient
def dark():
    p = Patch()
    cc = p.add('HemiolaAudio', 'CycleClock', (0, 0), {0: 40, 1: 3, 3: 0.6, 7: 0})
    sq = seq3(p, (15, 0), 4, [-2.0, -1.75, -2.1667, -1.5833], [1, 1, 1, 1])
    l1 = p.add('Fundamental', 'LFO', (39, 0), {2: -5.2, 0: 0})
    l2 = p.add('Fundamental', 'LFO', (49, 0), {2: -4.3, 0: 0})
    l3 = p.add('Fundamental', 'LFO', (59, 0), {2: -3.6, 0: 0})
    w = p.add('HemiolaAudio', 'Whoa', (0, 1), {
        0: 2, 1: 0.0, 2: 1.6, 3: 0.2, 4: 0.8, 5: 0.9, 6: 0.35, 7: 0.9, 8: 0.85, 9: 0.9, 10: 0.7, 11: 0.93,
        12: 0.1, 13: 0.8, 14: 0.6, 15: 0.5,
        16: 11, 17: 0.6, 18: 0.3, 19: 2, 20: 0.85, 21: 0.92, 22: 0.35, 23: 0.55}, data=CVD)
    au = p.add('Core', 'AudioInterface2', (27, 1))
    notes = p.add('Core', 'Notes', (38, 1), data={'text':
        "HEMIOLA AUDIO: WHOA!  DARK AMBIENT\n\n"
        "Cycle Clock runs at 40 BPM. Its SECTION trigger steps a four-note bass line in a Fundamental SEQ3 (C2, Eb2, Bb1, G2) and its SECTION gate holds each note open, so every note has time to bloom.\n"
        "Whoa! plays ratio 1.5 with slow attacks and releases. Three slow LFOs move INDEX, MOD D and the delay mix through the CV inputs. CV depth is 50 % (right-click Whoa!). Reverb pre-delay is about 250 ms.\n"
        "Delay: dotted quarter, clocked by the Cycle Clock BEAT trigger. Reverb: Vast, near-full decay. \n\n"
        "Turn REV DECAY to the top to hold the tail. Try REV TYPE on Shimmer for an octave-up wash. Not yet heard in Rack when written."})
    p.cable(cc, 1, sq, 1)      # SECTION trig -> SEQ3 clock
    p.cable(sq, 1, w, 0)       # pitch -> V/OCT
    p.cable(cc, 5, w, 3)       # SECTION gate -> GATE
    p.cable(cc, 0, w, 15)      # BEAT trig -> CLOCK IN
    p.cable(l1, 1, w, 4)       # LFO1 -> INDEX CV
    p.cable(l2, 1, w, 8)       # LFO2 -> MOD D CV
    p.cable(l3, 1, w, 17)      # LFO3 -> MIX CV
    p.cable(w, 0, au, 0); p.cable(w, 1, au, 1)
    p.save('Hemiola Whoa Dark Ambient.vcv', au)

# ------------------------------------------------------------------ 2. Hypnotic Techno
def techno():
    p = Patch()
    cc = p.add('HemiolaAudio', 'CycleClock', (0, 0), {0: 130, 1: 3, 3: 0.6, 7: 0})
    rt = p.add('HemiolaAudio', 'RhythmTalam', (15, 0), {0: 2, 2: 0.4})
    sq = seq3(p, (26, 0), 8, [-1.0, -1.0, -0.75, -1.0, -0.5833, -1.0, -0.4167, -0.75], [1, 1, 0, 1, 0, 1, 1, 0])
    l1 = p.add('Fundamental', 'LFO', (49, 0), {2: -3.84, 0: 0})
    l2 = p.add('Fundamental', 'LFO', (59, 0), {2: -5.06, 0: 0})
    w = p.add('HemiolaAudio', 'Whoa', (0, 1), {
        0: 4, 1: 0.0, 2: 3.2, 3: 0.35, 4: 0.0, 5: 0.55, 6: 0.0, 7: 0.55, 8: 0.0, 9: 0.6, 10: 0.0, 11: 0.62,
        12: 0.35, 13: 0.3, 14: 0.8, 15: 0.0,
        16: 8, 17: 0.5, 18: 0.28, 19: 0, 20: 0.35, 21: 0.3, 22: 0.5, 23: 0.15}, data=CVD)
    # kick
    ka = p.add('Fundamental', 'ADSR', (26, 1), {0: 0, 1: 0.4005, 2: 0, 3: 0.4005})
    kb = p.add('Fundamental', 'ADSR', (35, 1), {0: 0, 1: 0.6296, 2: 0, 3: 0.6296})
    ko = p.add('Fundamental', 'VCO', (44, 1), {2: -29.0, 4: 0.3})
    kv = p.add('Fundamental', 'VCA-1', (53, 1), {0: 1.0})
    # hat
    nz = p.add('Fundamental', 'Noise', (58, 1))
    hf = p.add('Fundamental', 'VCF', (63, 1), {0: 0.9742, 2: 0.2})
    ha = p.add('Fundamental', 'ADSR', (71, 1), {0: 0, 1: 0.3693, 2: 0, 3: 0.3693})
    hv = p.add('Fundamental', 'VCA-1', (80, 1), {0: 0.35})
    mx = p.add('Fundamental', 'Mixer', (0, 2), {0: 0.55})
    au = p.add('Core', 'AudioInterface2', (10, 2))
    notes = p.add('Core', 'Notes', (21, 2), data={'text':
        "HEMIOLA AUDIO: WHOA!  HYPNOTIC TECHNO, 130 BPM\n\n"
        "Cycle Clock (4/4) is the master. BEAT gate = kick and Whoa ACCENT. SECTION trigger resets the SEQ3 each bar so the loop never drifts.\n"
        "R&T divides the beat into eighths. That clock drives the SEQ3 (eight steps, one repeated riff) and the hats.\n"
        "Whoa! plays ratio 3 as a short FM pluck: sustain at zero, the release knobs set the note length. Slow LFOs sweep MOD D and INDEX, so the same riff keeps changing.\n"
        "Delay: dotted eighth from the BEAT clock. Reverb: small Room.\n\n"
        "Kick and hat are Fundamental modules going straight to the Mixer. Not yet heard in Rack when written."})
    p.cable(cc, 0, rt, 0)
    p.cable(rt, 0, sq, 1)
    p.cable(cc, 1, sq, 2)      # SECTION trig -> SEQ3 reset
    p.cable(sq, 1, w, 0); p.cable(sq, 0, w, 3)
    p.cable(cc, 0, w, 15); p.cable(cc, 4, w, 6)
    p.cable(l1, 1, w, 8); p.cable(l2, 1, w, 4)
    p.cable(cc, 4, ka, 4); p.cable(cc, 4, kb, 4)
    p.cable(ka, 0, ko, 1); p.cable(ko, 0, kv, 1); p.cable(kb, 0, kv, 0)
    p.cable(rt, 0, ha, 4)
    p.cable(nz, 0, hf, 3); p.cable(hf, 1, hv, 1); p.cable(ha, 0, hv, 0)
    p.cable(kv, 0, mx, 0); p.cable(hv, 0, mx, 1)
    p.cable(mx, 0, au, 0); p.cable(mx, 0, au, 1)
    p.cable(w, 0, au, 0); p.cable(w, 1, au, 1)
    p.save('Hemiola Whoa Hypnotic Techno.vcv', au)

if __name__ == '__main__':
    dark(); techno(); print('written')
