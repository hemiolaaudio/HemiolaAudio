# R&T (Rhythm & Thalam): user manual

Version 2.0.0 | 10 HP | Hemiola Audio | Free and open source (GPL-3.0-or-later)

R&T takes a steady beat and splits every beat into 1 to 9 equal parts, the way Carnatic music counts **nadai** (also called **gati**). It can then divide those parts down again, from 1 to 64, so a fast nadai such as Sankirna is easy to count. A chance control drops parts at random while the beat stays in place.

You do not need to know Indian music to use it. Everything is explained from the start.

Other manuals: [Cycle Clock manual](CycleClock.md) | [All Hemiola Audio manuals](../MANUAL.md)

**What is in this manual**

1. Ideas in plain words
2. The simple maths
3. The panel, dividing the parts, how the splitting works, the outputs, chance, CV, specifications
4. Patch recipes
5. Questions and answers
6. Appendix A: nadai table, and sources

## Quick start

1. Add R&T. Send it a beat clock: patch **BEAT** from a Cycle Clock, or any clock that gives one pulse per beat, into **BEAT IN**.
2. Set **DIVISION** to 4 (Chatusra). The display shows the name and the number of parts per beat.
3. Patch **PARTS** (row of triggers, first jack) into a hi-hat or shaker. You hear four hits per beat.
4. Patch **FIRST** (second jack) into a kick. It fires only on the beat.
5. Turn **DIVISION** to 3, 5, 7 or 9. The beat stays where it is. Only the filling changes.
6. Plug **CLICK** into a mixer to count by ear.

R&T needs one beat to learn the tempo, so the very first beat gives you only the first part.


## 1. Ideas in plain words

### Beat and tempo

- A **beat** is one tap of your foot.
- **Tempo** is how fast the beats come, in beats per minute (BPM). At 120 BPM there are two beats every second.

R&T takes your beats and fills each one with smaller, equal parts.

### Nadai: how a beat is divided

So far we counted beats. **Nadai** (also called **gati**) says how many equal parts you play inside each beat. Carnatic music uses five:

| Name | Parts per beat | In Western words |
| --- | --- | --- |
| Tisra | 3 | triplets |
| Chatusra | 4 | sixteenth notes (the usual default) |
| Khanda | 5 | quintuplets |
| Misra | 7 | septuplets |
| Sankirna | 9 | nonuplets |

(Source: Narthaki, "the nadai can be one of 3, 4, 5, 7 or 9".) R&T does these five and also 1, 2, 6 and 8, which are plain even splits.

### Why "Hemiola"?

A hemiola is a rhythm where three units are fitted against two, a 3:2 ratio. The word comes from the Greek for "half" and "whole". (Source: Songtive glossary.) Cycles and divisions that do not fit neatly into each other are exactly what this pack is for.

### Trigger and gate

Modular synths use two kinds of rhythm signal. Both are on/off signals.

- A **trigger** is a very short blip, about one thousandth of a second. It says "now!". Use it for drums, which make their own sound length.
- A **gate** stays on for a while and then goes off. It says "now, and for this long". Use it for **envelopes**, which need to know when to start and when to let go.

Every rhythm output in this pack comes as both. All pulses are 10 volts when on, 0 volts when off.

## 2. The simple maths

You do not need maths to use the module. This is here if you want to know what it is doing.

### Splitting a beat into parts

If one beat is split into **n** equal parts:

```
part length = beat length / n
part number k starts at k / n of the way through the beat   (k = 0, 1, 2 ... n-1)
```

At 120 BPM a beat is 0.5 s, so:

| Parts (n) | Name | Each part lasts | Parts per second | Parts start at (fraction of the beat) |
| --- | --- | --- | --- | --- |
| 1 | whole beat | 500 ms | 2 | 0 |
| 2 | Even | 250 ms | 4 | 0, 1/2 |
| 3 | Tisra | 167 ms | 6 | 0, 1/3, 2/3 |
| 4 | Chatusra | 125 ms | 8 | 0, 1/4, 2/4, 3/4 |
| 5 | Khanda | 100 ms | 10 | 0, 1/5, 2/5, 3/5, 4/5 |
| 6 | Even | 83 ms | 12 | 0, 1/6 ... 5/6 |
| 7 | Misra | 71 ms | 14 | 0, 1/7 ... 6/7 |
| 8 | Even | 62.5 ms | 16 | 0, 1/8 ... 7/8 |
| 9 | Sankirna | 56 ms | 18 | 0, 1/9 ... 8/9 |

The first part always starts exactly on the beat (k = 0).

### Dividing the parts

If the parts are divided by **N**, only every Nth part is passed on:

```
DIVIDED pulses per beat = parts per beat / N
```

| Parts per beat | DIVIDE (N) | DIVIDED pulses | What you get |
| --- | --- | --- | --- |
| 9 (Sankirna) | 9 | 1 per beat | One pulse on every beat |
| 9 | 3 | 3 per beat | Evenly spaced thirds of the beat |
| 9 | 18 | 1 per 2 beats | One pulse every second beat |
| 4 (Chatusra) | 2 | 2 per beat | Eighth notes |
| 9 | 4 | not a whole number | A pattern of 4 against 9 |

The count carries on from beat to beat. When N does not fit the number of parts, the pulses walk across the beat. Four against nine lines up again after 36 parts, which is 4 beats (4 x 9 is 36, and 36 is the lowest number both divide into).


## 3. The panel

**Display.** Shows the name (Tisra, Chatusra, Khanda, Misra or Sankirna for 3, 4, 5, 7 and 9, Even for 2, 6 and 8, and Beat for 1), the number of parts per beat, and the words "per beat". When DIVIDE is above 1 it adds "divided by" and the number.

**Row 1: three knobs**

| Control | What it does |
| --- | --- |
| DIVISION | Parts per beat, 1 to 9. |
| CV AMT | How strongly the CV IN jack moves the division. In LEVEL mode it is "parts per volt", from -1 to +1. The tick at the top is zero. Default is +1. No effect in COUNT mode. |
| GATE LEN | How long the gates stay on, as a share of one part, 5% to 95%. Default 50%. |

**Row 2: two knobs and a switch**

| Control | What it does |
| --- | --- |
| DIVIDE | Passes on every Nth part to the DIVIDED outputs, 1 to 64. At 1 every part passes. Default 1. |
| CHANCE | The chance that each part sounds, 0% to 100%. Default 100%. See "Chance" below. |
| CV MODE | Switch for the CV IN jack. Down is LEVEL, up is COUNT. |

**Row 3: three inputs**

| Control | What it does |
| --- | --- |
| BEAT IN | The beat clock. One pulse is one beat. |
| CV IN | Changes the division from a voltage or from pulses. |
| DIVIDE CV | Moves DIVIDE from a voltage. 0 to 10 V sweeps the whole range (6.3 steps per volt). Negative voltages go down from the knob. |

**Row 4: an input, a switch and an output**

| Control | What it does |
| --- | --- |
| RESET IN | A trigger that restarts the DIVIDE count, so the next part is the first of a new count. Patch a Cycle Clock CYCLE trigger here to keep the DIVIDED outputs lined up with the bar. |
| ACCENT | ON: the click is higher and louder on the part that lands on the beat. OFF: all clicks are the same. |
| CLICK (jack) | An audio output. It ticks on every part, with the on-beat part accented. |

**Rows 5 and 6: the output block.** The top row is **triggers**, the bottom row is **gates**, labelled GATE. Each gate sits under the trigger it belongs to.

| Column | Fires on |
| --- | --- |
| PARTS | Every part, including the one on the beat |
| FIRST | Only the part that lands on the beat |
| DIVIDED | Every Nth part, set by DIVIDE. The first one lands on a part, and the count carries on across beats |


### Dividing the parts (DIVIDE)

At Sankirna (9 parts per beat) the PARTS pulses come too fast to count. DIVIDE gives you a slower pulse on the same grid. It counts the parts and passes on every Nth one to the DIVIDED outputs. PARTS and FIRST keep running at full speed, so you can use both at once.

- DIVIDE 9 at nadai 9 gives one pulse per beat. DIVIDE 18 gives one pulse every two beats.
- DIVIDE 3 at nadai 9 gives three pulses per beat. DIVIDE 2 at nadai 4 gives two pulses per beat.
- The count carries on across beats, so a DIVIDE that does not fit the beat (for example 4 at nadai 9) walks across the beat. That is the point: it makes a pattern of 4 against 9.
- When you change DIVIDE or the first beat starts, the next part is the first of the new count.
- The DIVIDED gate lasts a share of the divided step, set by GATE LEN, and never runs into the next pulse.

### How the splitting works

1. A pulse arrives at BEAT IN. R&T fires the first part at once, so it is exactly on the beat.
2. R&T measures how long that beat was, using the time between this pulse and the one before.
3. It places the other parts evenly inside the next beat, using that length.

Things to know:

- **R&T needs one beat to learn the tempo.** On the very first beat after you start the clock, you will only get the first part. From the second beat on, you get all of them.
- **A new division starts on the next beat.** If you turn the knob or the CV changes in the middle of a beat, the beat finishes the way it started. The new number takes over on the next beat. This avoids stumbles.
- **Steady clocks work best.** Parts are placed using the length of the previous beat. If your tempo changes suddenly, the parts in the next beat may sit a little early or late. Smooth tempo changes are fine.
- **If a beat comes early**, any parts still waiting are dropped, so nothing falls behind.
- **If the clock stops**, the parts of the last beat finish and then everything is quiet. Nothing runs away.

### The outputs in detail

**Triggers** are 10 V for 1 millisecond.

**PARTS gate.** 10 V for a share of **one part**. At 120 BPM, division 4, each part is 125 ms. With GATE LEN at 50%, each gate lasts 62.5 ms.

**FIRST gate.** 10 V for a share of **the whole beat**. At 120 BPM with GATE LEN at 50%, this gate lasts 250 ms, however you split the beat. Use FIRST for something that should breathe with the beat, such as a bass note or a pad.

**DIVIDED gate.** 10 V for a share of **one divided step**, which is the time from one DIVIDED pulse to the next. At 120 BPM, division 4 and DIVIDE 2, a step is 250 ms, so a gate at 50% lasts 125 ms.

Gates never run into the next pulse, because the share is limited to 95%.

**CLICK** is a short tick: about 1.1 kHz and moderate level for ordinary parts, about 1.8 kHz at full level (5 V peak) for the part on the beat. R&T has no idea where the bar starts, so "1" means the first part of each beat. Patch R&T after a Cycle Clock to get the full picture (see recipe 1).

### Chance: dropping parts

CHANCE decides, for every part, whether it sounds. At 100% every part sounds. At 60% about six in ten do. The choice is random each time.

- **The part on the beat always sounds.** FIRST never drops, so the beat stays in place however low CHANCE is.
- **CHANCE affects PARTS and DIVIDED.** Both their triggers and their gates.
- **The DIVIDE count still counts dropped parts.** The grid underneath does not move. Only whether a pulse is heard changes.
- **CLICK ignores CHANCE.** You can still count by ear.
- At 0%, PARTS and DIVIDED are silent except where they land on the beat.


### Changing the division with CV

The CV IN jack has two modes. Flip the CV MODE switch to choose.

**LEVEL mode (switch down).** The voltage sets how many parts per beat.

```
parts per beat = DIVISION knob + (CV in volts x CV AMT)
```

The result is rounded to the nearest whole number and kept between 1 and 9. R&T looks at the CV at the moment each beat arrives, so what you hear is the value at the start of every beat.

| Knob | CV source | CV AMT | What happens |
| --- | --- | --- | --- |
| 4 | none | any | Always 4. |
| 3 | A sequencer playing 0 V, 1 V, 2 V, 3 V | +1 | 3, 4, 5, 6 |
| 5 | A slow LFO that swings -5 V to +5 V | +0.4 | The LFO swings 5 by plus or minus 2: 3, 4, 5, 6, 7 and back |
| 2 | An envelope 0 V to 10 V | +0.5 | Starts at 2, rises to 7 as the envelope opens |
| 3 | A gate (0 V off, 10 V on) | +0.2 | Gate off: 3. Gate on: 5 |
| 5 | A gate | -0.2 | Gate off: 5. Gate on: 3 |
| 4 | A note from a keyboard at 1 V per octave | +1 | Each octave up adds 1 part |

The number at the end of each row is always held between 1 and 9.

Good for: sequencers, LFOs, envelopes, random voltages, gates, and keyboards. The CV AMT knob sets how far the CV can reach. Turn it down to narrow the range, or below zero to flip the CV upside down.

**COUNT mode (switch up).** Every pulse moves the division one further. After 9, it goes back to 1. CV AMT is ignored.

A pulse is anything that rises above about 2 V, so triggers, gates and square-wave LFOs all work. Set the knob where you want to begin. The first pulse then moves one further.

| Knob | Pulses so far | Division |
| --- | --- | --- |
| 4 | 0 | 4 |
| 4 | 1 | 5 |
| 4 | 5 | 9 |
| 4 | 6 | 1 |
| 4 | 9 | 4 (one full lap) |

Good for: stepping the nadai up on every cycle, for example by patching the Cycle Clock CYCLE trigger into CV IN (see the recipe "change the nadai every cycle"), or changing it every few bars from a clock divider.

The counter starts again from the knob position when you unplug the cable or switch back to LEVEL.

### Specifications

| | |
| --- | --- |
| Width | 10 HP |
| Parts per beat | 1 to 9 |
| DIVIDE | 1 to 64 (6.3 steps per volt on DIVIDE CV, 0 to 10 V for the whole range) |
| CHANCE | 0% to 100% (the part on the beat always sounds) |
| Gate length | 5% to 95% (of a part, of a beat for FIRST, of a divided step for DIVIDED) |
| Trigger | 10 V, 1 ms |
| Gate | 10 V |
| Click | Audio, 5 V peak |
| Input pulse | Rising edge above about 2 V |
| Needs | One beat to learn the tempo |

## 4. Patch recipes

Each recipe says what you plug into what, and what you should hear.

### Recipe 1: Carnatic nadai over a thalam

*Goal: a Carnatic-style subdivision on top of a cycle.*

1. Cycle Clock: choose Adi (position 42), TEMPO 70.
2. Patch Cycle Clock BEAT (trigger) into R&T BEAT IN.
3. Set R&T DIVISION to 4 (Chatusra). Patch PARTS (trigger) into a snare or shaker.
4. Patch R&T FIRST (trigger) into a kick.
5. Patch R&T CLICK into a mixer. Patch Cycle Clock CLICK in as well, with a lower level.

You get four parts per beat, with the kick exactly on each beat. Turn DIVISION to 5 (Khanda) or 7 (Misra) without touching anything else. The beat stays in place, and only the filling changes. That is nadai.

### Recipe 2: change the nadai every cycle

*Goal: the nadai steps up at the start of every lap.*

1. Cycle Clock BEAT (trigger) goes to R&T BEAT IN, as in recipe 1.
2. Patch Cycle Clock CYCLE (trigger) into R&T CV IN.
3. Set R&T CV MODE to COUNT (switch up) and DIVISION to 3.

The first sam steps the division up from the knob, so lap one plays 4 parts per beat. Every time sam arrives again, it steps up once more: 5, 6, 7, 8, 9, then back to 1, 2, 3, and round again. A long, slowly changing piece with no sequencer.

### Recipe 3: count a fast nadai

*Goal: keep hold of Sankirna, nine parts per beat.*

1. Set DIVISION to 9 and DIVIDE to 9.
2. Patch PARTS (trigger) into a quiet shaker. Patch DIVIDED (trigger) into a clap.

You hear nine soft hits per beat and one clap exactly on the beat. Change DIVIDE to 3 and you hear the clap three times per beat, a count you can follow. DIVIDE 18 gives a clap every second beat.

### Recipe 4: four against nine

*Goal: a pattern that walks across the beat and comes home.*

1. Set DIVISION to 9 and DIVIDE to 4.
2. Patch FIRST (trigger) into a kick and DIVIDED (trigger) into a snare.

The snare falls on part 1, 5 and 9 of the first beat, then part 4 and 8 of the next, and so on. Kick and snare line up again every 4 beats.

### Recipe 5: ghost notes

*Goal: a hi-hat that breathes without losing the beat.*

1. Patch PARTS (trigger) into a hi-hat and FIRST (trigger) into a kick.
2. Turn CHANCE down to about 60%.

The kick never moves. Some hi-hats drop out each beat, a little differently every time.

### Recipe 6: keep DIVIDED on the bar

*Goal: a divided pulse that restarts with every lap of a thalam.*

1. Patch Cycle Clock BEAT (trigger) into R&T BEAT IN.
2. Patch Cycle Clock CYCLE (trigger) into R&T RESET IN.
3. Set DIVIDE to a number that does not fit the beat, such as 5.

Every lap the DIVIDED pulses start again from the same place, so the pattern repeats exactly each time sam arrives.


## 5. Questions and answers

**R&T only gives me one pulse per beat at first.** It needs one beat to learn the tempo. From the second beat it splits fully.

**R&T parts sound uneven when I change tempo.** R&T uses the length of the previous beat. A sudden tempo jump affects one beat. Smooth changes are fine.

**I turned DIVISION and nothing changed straight away.** A new division starts on the next beat, so the beat in progress finishes as it began.

**DIVIDED is silent.** Check that CHANCE is not at 0%, and that a clock is reaching BEAT IN. With DIVIDE at 1, DIVIDED matches PARTS.

**FIRST and PARTS do not drop with CHANCE low.** That is right. The part on the beat always sounds.

**What is the difference between a trigger and a gate?** A trigger is a short blip for drums. A gate stays on for a while, for envelopes. See section 1.

**Can I use CV that is not a clean pulse in COUNT mode?** Yes. It counts every time the signal rises above about 2 V. A noisy signal can count more than once, so use a clean pulse, a gate or a square-wave LFO.

**Does it work at any sample rate?** Yes. All timing is calculated from the sample rate Rack is set to.

---


## Appendix A: nadai, quick table

| Parts | Name | Western name |
| --- | --- | --- |
| 1 | Beat | quarter note |
| 2 | Even | eighth notes |
| 3 | Tisra | triplets |
| 4 | Chatusra | sixteenth notes |
| 5 | Khanda | quintuplets |
| 6 | Even | sixteenth-note triplets |
| 7 | Misra | septuplets |
| 8 | Even | thirty-second notes |
| 9 | Sankirna | nonuplets |

### Sources

- Nadai (gati) names and counts 3, 4, 5, 7, 9: [Narthaki, article on talas](https://www.narthaki.com/info/articles/art232.html)
- Hemiola (3:2 ratio, origin of the word): [Songtive glossary](https://www.songtive.com/en/glossary/hemiola)

---


## About Hemiola Audio

Hemiola Audio is a new developer, and a very small one: one person. I make music in VCV Rack most days, and these modules came from things I wanted on my own patches and could not find. I am not a trained programmer. I write the code with an AI assistant and decide everything else myself: what a module should do, how it should look, and whether it feels right under the hands.

The timing rules in the modules are covered by automatic tests. Whether a module is musical is decided in a patch, by ear, and I would like to hear what you find.

More modules are planned, and not all of them will be about rhythm: instruments to play as well as tools to keep time. They are free and open source. If something is wrong, unclear or missing, please open an issue on GitHub or write to hemiolaaudio@gmail.com.

## Credits and licence

Hemiola Audio is free software under the GNU General Public License, version 3 or later. The panel lettering is Montserrat, used under the SIL Open Font Licence 1.1 (the licence text is in `res/fonts/OFL.txt`). The Hemiola Audio name and logo belong to Hemiola Audio.
