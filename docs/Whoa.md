# WHOA!: user manual

Version 2.1.0 | 25 HP | Hemiola Audio | Free and open source (GPL-3.0-or-later)

WHOA! is an FM voice with two operators, a modulator and a carrier, and a complete effects section built in: a clocked delay, and a reverb with four types. Each operator has its own gate and its own ADSR envelope, and every envelope stage has a CV input. There is also one gate that fires both operators. It takes V/oct, gives a stereo output and a gate output, and plays up to 16 voices.

It was made for techno, psytrance and dark ambient, but it plays any note you send it. In Rack's module browser it is listed as Whoa.

Other manuals: [All Hemiola Audio manuals](../MANUAL.md)

## Quick start

1. Add WHOA!. Patch your sequencer or keyboard pitch into **V/OCT** and its gate into **GATE** (the one marked GATE, not MOD GATE or CAR GATE).
2. Patch **LEFT** and **RIGHT** into a mixer or your audio module.
3. Turn **INDEX** up. The tone gets brighter.
4. Turn **MOD D** (the modulator's decay) down. Each note now starts bright and settles to a plainer tone.
5. Turn **REV MIX** up to put the sound in a room. Turn **DLY MIX** up for echoes.

Two example patches are in the `patches` folder: `Hemiola Whoa Dark Ambient.vcv` and `Hemiola Whoa Hypnotic Techno.vcv`.

## 1. How the voice works

The pitch you send into V/OCT is the carrier's pitch. The modulator runs at that pitch times **RATIO**. The modulator's output moves the carrier's phase. How far it moves it is set by **INDEX**. A larger INDEX gives more harmonics, so a brighter tone.

Each operator has an envelope:

- The **carrier envelope** (CAR A, D, S, R) sets the loudness over time.
- The **modulator envelope** (MOD A, D, S, R) sets how bright the note is over time. **ENV AMT** decides how much the modulator envelope moves INDEX.

With ENV AMT at 100%, INDEX is zero when the modulator envelope is at zero, and INDEX is at its full setting when the envelope is at its top. With ENV AMT at 0%, the envelope does nothing and INDEX stays at the knob setting.

## 2. The gates

WHOA! has three gate inputs.

| Input | What it fires |
| --- | --- |
| **MOD GATE** | The modulator envelope only |
| **CAR GATE** | The carrier envelope only |
| **GATE** | Both envelopes together |

A gate is high at 1 V or above. **GATE OUT** is high (10 V) while any of the three is high.

Two uses of the separate gates:

- **Play the tone without a new note.** Hold CAR GATE high so the carrier keeps sounding, and send short pulses into MOD GATE. Each pulse restarts the modulator envelope, so the tone brightens and settles again. The carrier envelope is not touched, so there is no new attack.
- **Silence the modulator.** Send a gate to CAR GATE only. The modulator envelope stays at zero, so with ENV AMT at 100% you hear the plain carrier.

A gate into MOD GATE alone makes no sound, because the carrier is closed.

## 3. The panel

The panel is split into five sections. Each has its own colour and a title tab in the side margin: **VOICE** (sand), **ENVELOPES** (green), **DELAY** (blue), **REVERB** (lilac) and **OUT** (dark sand). The depth of the CV inputs is not a knob. It is set in the right-click menu (see CV depth below).

### Voice

| Control | What it does |
| --- | --- |
| **RATIO** | Modulator pitch as a multiple of the note: 0.5, 1, 1.5, 2, 3, 4, 5, 6, 7, 8, 12 or 16 |
| **FINE** | Adds between -0.5 and +0.5 to the ratio. Use it to detune the modulator from a whole ratio |
| **INDEX** | How far the modulator bends the carrier, from 0 to 10 |
| **FEEDBACK** | Sends the modulator back into itself. A higher setting gives a harsher, noisier tone |
| **ENV AMT** | How much the modulator envelope moves INDEX (see section 1) |
| **DRIVE** | Pushes the sound into a soft clip. The clip is uneven, which adds even harmonics. The level stays about the same as you turn it up |
| **SPREAD** | Detunes the left and right carriers against each other, by up to about 10 cents each way. At 0 the two sides are identical |

### Envelopes

**MOD A, D, S, R** (top row of the green block) and **CAR A, D, S, R** (third row of it) are attack, decay, sustain and release for the modulator and the carrier. The jack under each knob is its CV input: **A CV**, **D CV**, **S CV**, **R CV**.

- Attack, decay and release run from 1 millisecond to 5 seconds. Hover over a knob, or right-click it, to see the time.
- Sustain is a level from 0% to 100%.
- A CV moves the knob position. At the default CV depth of 50%, 10 V sweeps half the knob. The knob is already exponential in time, so a rising CV lengthens the stage smoothly from milliseconds to seconds.

### Delay

| Control | What it does |
| --- | --- |
| **DLY TIME** | The delay time as a note value of the clock beat. 16 steps, listed below |
| **DLY FDBK** | How much of the echo feeds back. The echoes bounce between left and right. The limit is just under 100%, so they always die away |
| **DLY MIX** | Dry to wet. At 0 you hear only the voice. At 100% you hear only the echoes |
| **CLOCK IN** | One pulse per beat. The delay measures the time between pulses. With nothing patched it uses 120 BPM |
| **FDBK CV**, **MIX CV** | CV for feedback and mix. 10 V at the Delay CV depth of 100% sweeps the whole knob |

The 16 steps of DLY TIME, shortest to longest: 1/32, 1/7 of a beat, 1/16 triplet, 1/5 of a beat, 1/16, dotted 1/16, 1/8 triplet, 1/8, dotted 1/8, 1/4 triplet, 1/4, dotted 1/4, 1/2 triplet, 1/2, dotted 1/2, one bar (four beats). The 1/7 and 1/5 steps are for odd divisions, in the manner of [R&T](RhythmTalam.md).

Patch the BEAT trigger of [Cycle Clock](CycleClock.md) into CLOCK IN and the delay follows the cycle's tempo. The longest echo is about eight seconds, so at very slow tempos the longest steps are cut short. Changing DLY TIME glides the echo to the new time over a moment, which gives a tape-like swoop instead of a click.

### Reverb

| Control | What it does |
| --- | --- |
| **REV TYPE** | Room, Hall, Vast or Shimmer |
| **REV SIZE** | The size of the space. The range depends on the type |
| **REV DECAY** | How long the tail lasts |
| **REV TONE** | How much high end the tail keeps. Low is dark and soft. High is bright |
| **PRE DELAY** | A gap before the reverb starts, from 0 to 1 second. The scale is finer at the short end. The dry sound and the delay stay in time. A little pre-delay keeps the front of a note clear of its tail |
| **REV MIX** | Dry to wet |

The four types:

- **Room.** A small space. The tail runs from about a fifth of a second to two and a half seconds.
- **Hall.** A large space. The tail runs from under a second to about nine seconds.
- **Vast.** An enormous space with a very long tail and deep, slow movement in the tail. From about four seconds up to a minute. With **REV DECAY** at the top of its range the tail does not fade. New sound keeps adding to it, so it builds. Turn REV DECAY down to let it go.
- **Shimmer.** A hall whose tail also rises an octave each time it goes round, so the tail climbs in pitch as it fades. It is a bright, airy wash. Keep REV TONE below full unless you want it very sharp.

Vast was written with the Eventide BlackHole pedal as the reference for the sound: huge size, near-endless decay, slow deep modulation, and a tail that builds under held input. Eventide has not published how BlackHole works inside, so WHOA! does not copy it. Vast uses a general reverb design (eight delay lines mixed together, with slow modulation), tuned by ear toward that kind of sound. It does not have BlackHole's reverse decay (its Gravity control), or its freeze switch.

### CV depth (right-click menu)

Right-click the panel to find **CV depth**. It has three groups, each with 25%, 50%, 75% or 100%:

- **Voice**: INDEX CV and FB CV.
- **Envelopes**: the eight A, D, S and R CVs.
- **Delay**: FDBK CV and MIX CV.

At 100%, 10 V sweeps the whole range of the knob the jack belongs to. At 50%, 10 V sweeps half of it. The default is 50% for all three groups. The choice is saved with the patch. INDEX CV is the exception: at 100%, 1 V moves INDEX by 1.

### Inputs and outputs

| Jack | What it does |
| --- | --- |
| **V/OCT** | Pitch, 1 V per octave. 0 V is C4 (261.6 Hz) |
| **MOD GATE**, **CAR GATE**, **GATE** | See section 2 |
| **INDEX CV** | Added to INDEX. At Voice depth 100%, 1 V moves INDEX by 1 |
| **FB CV** | Added to FEEDBACK. At Voice depth 100%, 10 V takes FEEDBACK from 0 to full |
| **ACCENT** | 0 to 10 V. Opens the modulator up to 60% wider and lifts the level a little. Send it a velocity or an accent lane |
| **A CV, D CV, S CV, R CV** (two rows) | CV for the modulator envelope (first row) and the carrier envelope (second row) |
| **CLOCK IN**, **FDBK CV**, **MIX CV** | See Delay |
| **LEFT**, **RIGHT** | Audio out, after the effects. Up to about 5 V peak |
| **GATE OUT** | 10 V while any gate is high. Polyphonic |

## 4. Polyphony

WHOA! plays as many voices as there are channels on V/OCT and on the gate inputs, up to 16. Patch a polyphonic keyboard into V/OCT and GATE and you get chords. The knobs apply to every voice. A single cable plays one voice. The CV inputs for the voice (INDEX CV, FB CV, accent and the envelope CVs) can be polyphonic, one channel per voice.

The voices are added together into a stereo pair before the effects, so there is one delay and one reverb for the whole module. LEFT and RIGHT are ordinary single cables. To keep chords from clipping, the sum is reduced in level as the number of voices goes up. The delay CV inputs are single channel. GATE OUT stays polyphonic.

## 5. Patch recipes

**A punchy bass note.** Sequencer pitch into V/OCT, sequencer gate into GATE. RATIO 1 or 2, INDEX 4 to 6, ENV AMT 100%. Set MOD D short, around 50 ms, and MOD S to 0. Each note opens with a burst of harmonics and settles. Set CAR A short and CAR R short so the notes stay tight.

**A tone that moves under a held note.** Send a long gate into CAR GATE, with CAR S at 100%. Send a 16th-note trigger into MOD GATE. Set MOD D to about 100 ms.

**A pluck for techno.** RATIO 3, INDEX 3, CAR S and MOD S at 0, and CAR R about 0.2 s. A short gate now makes a short note. Put DLY TIME on dotted 1/8, DLY MIX around 25%, REV TYPE on Room with a little REV MIX. This is the second example patch.

**A slow evolving pad.** RATIO 1.5, long attack and release on the carrier (above a second), INDEX around 1.5. Send a very slow LFO to INDEX CV and another to MOD D CV. Set REV TYPE to Vast with REV DECAY high, and DLY TIME to dotted 1/4. This is the first example patch.

**A harder edge.** Raise FEEDBACK a little, then DRIVE. Patch a slow LFO into FB CV and set the Voice CV depth to taste in the right-click menu.

**A wide stack.** Raise SPREAD, then send LEFT and RIGHT to two sides of your mixer.

## 6. Questions and answers

**There is no sound at all.** WHOA! needs a gate. With nothing patched into GATE, MOD GATE or CAR GATE, the carrier stays closed. V/OCT only sets the pitch. Patch a gate into GATE, and check that GATE OUT lights up.

**Why is there no sound with the modulator gate alone?** The modulator only bends the carrier. If the carrier is closed there is nothing to hear.

**Why do I hear a plain sine with CAR GATE alone?** With ENV AMT at 100%, INDEX follows the modulator envelope, and that envelope is closed. Lower ENV AMT, or send MOD GATE too.

**Is it polyphonic?** The voice is, up to 16 voices. The effects are shared by all voices, so LEFT and RIGHT are one stereo pair.

**The echoes are too fast or too slow.** The delay follows the clock at CLOCK IN. Check what is patched there. With nothing patched, it runs at 120 BPM.

**How do I get a tail that never ends?** REV TYPE on Vast, REV DECAY at the top. Turn it down to let the tail fade, or lower REV MIX.

**The level jumps when I switch reverb type.** Each type has its own size and decay range, so the tail changes. Adjust REV MIX after you choose the type.

**What does the ratio do?** Whole-number ratios give harmonic tones. Ratios that are not whole numbers, which FINE gives you, make tones whose overtones do not line up with the note.

## 7. Specifications

| | |
| --- | --- |
| Width | 25 HP |
| Operators | 2: one modulator, one carrier |
| Voices | up to 16 |
| Ratios | 0.5, 1, 1.5, 2, 3, 4, 5, 6, 7, 8, 12, 16, plus FINE of -0.5 to +0.5 |
| Envelopes | 2 ADSR, 1 ms to 5 s, each stage with CV |
| Oversampling | 4x inside the operator chain |
| Pitch | 1 V per octave, 0 V = C4 |
| Gate threshold | 1 V |
| Delay | stereo, ping-pong feedback, 16 note values, clocked from CLOCK IN (120 BPM if unpatched), longest echo about 8 s |
| Reverb | 8-line feedback network with slow modulation. Room, Hall, Vast, Shimmer. Pre-delay 0 to 1 s |
| CV depth | right-click menu, 25 / 50 / 75 / 100% for Voice, Envelopes and Delay CVs, default 50%, saved with the patch |
| Audio out | stereo, about 5 V peak |
| Gate out | 10 V, polyphonic |

## Sources

Eventide, BlackHole manual and MusicRadar's review of the BlackHole pedal, for the description of the BlackHole controls and sound used as the starting point for the Vast type.

## Licence

Hemiola Audio is free software under the GNU General Public License, version 3 or later. The panel lettering is Montserrat, used under the SIL Open Font Licence 1.1 (the licence text is in `res/fonts/OFL.txt`). The Hemiola Audio name and logo belong to Hemiola Audio.
