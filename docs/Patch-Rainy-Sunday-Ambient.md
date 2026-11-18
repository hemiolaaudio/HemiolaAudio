# Rainy Sunday, grey skies: patch guide

Patch file: [Hemiola Rainy Sunday Ambient.vcv](../patches/Hemiola%20Rainy%20Sunday%20Ambient.vcv) | Made with [Cycle Clock](CycleClock.md) and [R&T](RhythmTalam.md) | Hemiola Audio | Free and open source (GPL-3.0-or-later)

Slow and open, with nothing on a steady beat. Rain drops, glass bells, a slow pad, soft mist and a faint city hum, held together by a Cycle Clock and two R&Ts that are counting an irregular pulse. Every sound is built from Rack's own Fundamental modules.

**What you need.** Rack 2.6.6 or newer, Hemiola Audio 2.0.0 (Cycle Clock and R&T), and the Fundamental and Core modules that come with Rack.

**Names.** The Random modules are numbered left to right along the top row of the patch (Random 1 is the irregular pulse, Random 2 picks R&T 1's division, Random 3 and 4 pick the notes for the drops and bells, Random 5 and 6 move the level of the mist and the patter).

**To open it.** In Rack, choose File, then Open, and pick the patch file. Check the audio output on the Audio module near the bottom of the patch. Give it a minute. The cycles are long, so the shape of the piece takes a while to show.

## 1. What is happening

1. A slow LFO speeds a **Random** module's trigger rate up and down. The Random module also skips about a quarter of its triggers. Those triggers are the beat for the Cycle Clock, so the beat is irregular and never settles.
2. The **Cycle Clock** counts these beats as a cycle. When a cycle ends, its CYCLE trigger goes back into its own CYCLE CV input, set to COUNT mode. Every lap, it moves one place along its list of 55 cycles. It starts on 7 beats counted 2+2+3, then 7 counted 3+2+2, then 8, 9, 11, 12, Tintal (16) and on through the table, and wraps back to the start. The pattern of sections never repeats for a long time.
3. **R&T 1** splits each irregular beat into parts. A second Random module picks its DIVISION anew in every section, so the rain sometimes pitters quickly and sometimes barely falls.
4. **R&T 2** listens to the cycle's sections. Its DIVISION steps up by one every lap (COUNT mode) and wraps from 9 back to 1.

## 2. The Hemiola modules in the patch

| Module | Setting | Job |
| --- | --- | --- |
| **Cycle Clock** | CYCLE 7 (2+2+3), CV MODE Count, GATE LEN 90% | Takes its beat from the Random module. Its BEAT trigger drives R&T 1. Its SECTION trigger drives R&T 2, picks a new division for R&T 1 (through a Random module) and clocks the pad's chord sequencer. Its SECTION gate swells the pad. Its CYCLE trigger feeds its own CYCLE CV and steps R&T 2's division. |
| **R&T 1** | DIVISION 4 to start, CV AMT 0.6 (Level mode), GATE LEN 20% | Plays the rain drops. A random level from the Random module picks its division in every section. |
| **R&T 2** | DIVISION 2 to start, CV MODE Count, GATE LEN 20% | Plays the glass bells. Steps up by one division every lap. |

## 3. Every Hemiola cable

| From | To | What it does |
| --- | --- | --- |
| Random 1 (trigger output) | Cycle Clock CLOCK IN | The irregular beat |
| Cycle Clock CYCLE trigger | Cycle Clock CYCLE CV | Counts the cycle forward, one place per lap |
| Cycle Clock BEAT trigger | R&T 1 BEAT IN | Each beat feeds the rain |
| Cycle Clock SECTION trigger | R&T 2 BEAT IN | Each section feeds the bells |
| Cycle Clock SECTION trigger | Random 2 trigger | A new random value for each section |
| Random 2 output | R&T 1 CV IN | Picks R&T 1's DIVISION |
| Cycle Clock CYCLE trigger | R&T 2 CV IN | Steps R&T 2 up by one division per lap |
| R&T 1 PARTS trigger | Random 3 trigger | A new random note for each drop |
| R&T 1 PARTS gate | Drop envelope | The drop itself |
| R&T 2 PARTS trigger | Random 4 trigger | A new random note for each bell |
| R&T 2 PARTS gate | Bell envelope | The bell itself |
| Cycle Clock SECTION trigger | Pad chord sequencer clock | A new chord in every section |
| Cycle Clock SECTION gate | Pad envelope | The pad swells in each section |

## 4. The sounds

| Sound | How it is made |
| --- | --- |
| Rain drops | A high sine oscillator with a short envelope. A random note from R&T 1's parts is snapped to a scale by a Quantizer. |
| Glass bells | Two oscillators on the same quantised note, one tuned high and one very high, with a short envelope. The notes come from a second Quantizer. |
| Pad | A 7-step chord sequencer, clocked by the sections. Two detuned oscillators into a low-pass filter that a slow LFO opens and closes, with a long attack and a long release. |
| Mist | Soft pink noise through a filter that a slow LFO moves. A Random module raises and lowers its level, so it breathes. |
| Rain patter | High-pass filtered noise at a low level, which a Random module raises and lowers. |
| City hum | Low, rumbling red noise through a low-pass filter at a steady low level, with two very low oscillators, one slightly detuned and slowly modulated by an LFO. |
| Space | Two delays in a row blur the drops, bells and pad into a wash. |

**The scale.** Both Quantizers allow C, D, F, G and A: the notes of D minor pentatonic. Any note that falls out of the scale is snapped to the nearest allowed one.

## 5. Things to try

1. **A drier room.** Lower the level of the mist VCA.
2. **A steadier pulse.** Turn the Random 1 PROB to 100%. The irregular beat then never skips.
3. **Hold the cycle.** Unplug the cable from Cycle Clock CYCLE CV. The cycle stays at the knob setting (7 beats, 2+2+3) and stops moving through the table.
4. **Another scale.** Click different notes on the two Quantizers.
5. **Pick a cycle.** Turn Cycle Clock CYCLE to Tintal or Rupak before unplugging CYCLE CV, and listen to the sections of a Hindustani cycle played from an irregular pulse.
6. **Count instead of follow.** Set R&T 1's CV MODE to Count. It then steps its division up by one each time the random voltage rises past about 2 volts, instead of following the voltage's level.

## 6. If something is wrong

- **Nothing plays.** Check the Audio module and your computer's output. The beat is irregular and can leave long gaps.
- **Cycle Clock or R&T look like empty boxes.** Hemiola Audio is not installed. Install it from the release page and restart Rack.
- **It sounds too busy or too thin.** The Random modules decide. Wait a lap or two. It changes.

## 7. Rebuild the heart of it

1. A Random module's trigger output into Cycle Clock **CLOCK IN**.
2. Cycle Clock **CYCLE** trigger into its own **CYCLE CV**, with CV MODE on **Count**.
3. Cycle Clock **BEAT** trigger into R&T **BEAT IN**.
4. R&T **PARTS** trigger into a Random module's trigger input, its output into a Quantizer, and the Quantizer into an oscillator.
5. R&T **PARTS** gate into an envelope and a VCA on the oscillator.

Every lap the cycle changes, and the drops change with it.
