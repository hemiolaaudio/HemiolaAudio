# Cycle Clock: user manual

Version 2.0.0 | 14 HP | Hemiola Audio | Free and open source (GPL-3.0-or-later)

Cycle Clock is a clock that counts a whole rhythm cycle, such as a 16-beat thalam or a 7/8 bar, and tells the rest of your patch where it is inside the cycle. It has 55 cycles built in: 13 ordinary bars, 7 Hindustani thalams and 35 Carnatic thalams.

You do not need to know Indian music to use it. Everything is explained from the start.

Other manuals: [R&T manual](RhythmTalam.md) | [All Hemiola Audio manuals](../MANUAL.md)

**What is in this manual**

1. Ideas in plain words
2. The simple maths
3. The panel, how it counts, the outputs, CV, tempo and swing, specifications
4. Patch recipes
5. Questions and answers
6. Appendix A: all 55 cycles, and sources

## Quick start

1. Add Cycle Clock to a patch. It runs by itself at 120 BPM in Tintal, a 16-beat cycle.
2. Patch **BEAT** (top row of the output block, first jack) into the trigger input of a drum or hi-hat. You hear a steady pulse.
3. Patch **CYCLE** (same row, third jack) into a kick. It fires on beat one only, so you hear the start of every lap.
4. Patch **SECTION** (same row, second jack) into a clap. It fires at the start of each group of four.
5. Plug **CLICK** into a mixer to count by ear. The high click is beat one.
6. Turn the **CYCLE** knob and read the name on the display. The new cycle begins when the current lap ends.


## 1. Ideas in plain words

### Beat, tempo and cycle

- A **beat** is one tap of your foot.
- **Tempo** is how fast the beats come, in beats per minute (BPM). At 120 BPM there are two beats every second.
- A **cycle** is a fixed number of beats that repeats. In Western music this is a **bar**: four beats in 4/4, three beats in 3/4. In Indian music it is called a **thalam** (say "TAH-la"). The word means "clap" or "beat".

Think of a cycle as a lap on a running track. The beats are the markers on the track. When you reach the last marker, you start the next lap.

### Beat one, called "sam"

The first beat of the cycle is the most important. Musicians feel it as "home". Hindustani (North Indian) music calls it **sam** (say "sum"). In Western music it is "the one". Everything lines up on it: the singer, the drummer, the dancer.

### Sections

A cycle is usually split into smaller groups, like words in a sentence. Hindustani music calls them **vibhag**. Carnatic (South Indian) music builds them from small units called **angas**. In this manual they are simply called **sections**.

Example: Tintal has 16 beats in four sections of four: `4 + 4 + 4 + 4`.

### A walk through Tintal

Tintal (also spelt Teental or Teentaal) is the most common thalam in Hindustani music. It has 16 beats in four sections of four. Musicians keep the cycle with their hands:

| Beats | What the hands do |
| --- | --- |
| 1 (sam) | Clap |
| 5 | Clap |
| 9 | Wave (this beat is called **khali**, meaning "empty") |
| 13 | Clap |

So: **clap** 2 3 4, **clap** 6 7 8, **wave** 10 11 12, **clap** 14 15 16, and then back to the clap on 1. (Source: Wikipedia, Teental.)

Cycle Clock knows this structure. It gives you a pulse on every beat, a pulse at the start of every section (beats 1, 5, 9, 13), and a pulse at the start of the whole cycle (beat 1 only).

### Carnatic thalams, built from three blocks

Carnatic (South Indian) thalams are built from three blocks:

| Block | Name | Length in beats |
| --- | --- | --- |
| Laghu | "light" | 3, 4, 5, 7 or 9 (this choice is called the **jati**) |
| Drutam | "fast" | 2 |
| Anudrutam | "very fast" | 1 |

(Source: karnatik.com, Carnatic taala.) A drutam is a clap and a wave of the hand. An anudrutam is a single clap. A laghu is a clap followed by finger counts.

The seven standard thalams are made by joining those blocks in a fixed order. L is laghu, D is drutam, U is anudrutam:

| Thalam | Blocks |
| --- | --- |
| Dhruva | L D L L |
| Matya | L D L |
| Rupaka | D L |
| Jhampa | L U D |
| Triputa | L D D |
| Ata | L L D D |
| Eka | L |

Seven thalams times five laghu sizes gives 35 thalams. Cycle Clock has all 35. Their names read "thalam, then laghu size", for example **Rupaka 4** (a drutam of 2 beats, then a laghu of 4 beats: 6 beats, `2+4`).

**Adi** thalam is the best known. It is Triputa with a laghu of 4: 4 + 2 + 2 = 8 beats. Cycle Clock lists it as **Adi**.

### Trigger and gate

Modular synths use two kinds of rhythm signal. Both are on/off signals.

- A **trigger** is a very short blip, about one thousandth of a second. It says "now!". Use it for drums, which make their own sound length.
- A **gate** stays on for a while and then goes off. It says "now, and for this long". Use it for **envelopes**, which need to know when to start and when to let go.

Every rhythm output in this pack comes as both. All pulses are 10 volts when on, 0 volts when off.

## 2. The simple maths

You do not need maths to use the module. This is here if you want to know what it is doing.

### Seconds per beat

```
seconds per beat = 60 / BPM
```

At 120 BPM, one beat is 60 / 120 = 0.5 seconds. At 90 BPM it is 0.667 seconds.

### How long is a cycle?

```
cycle length = beats in the cycle x seconds per beat
```

| Cycle | Beats | At 60 BPM | At 120 BPM | At 180 BPM |
| --- | --- | --- | --- | --- |
| 4 (ordinary bar) | 4 | 4 s | 2 s | 1.33 s |
| Adi | 8 | 8 s | 4 s | 2.67 s |
| Rupak | 7 | 7 s | 3.5 s | 2.33 s |
| Tintal | 16 | 16 s | 8 s | 5.33 s |
| Dhamar | 14 | 14 s | 7 s | 4.67 s |

### The cycle position voltage

Cycle Clock can send a voltage that rises smoothly from 0 V at the start of the cycle to nearly 10 V at the end:

```
voltage = 10 x (beats gone by + part of the current beat) / beats in the cycle
```

In Tintal (16 beats), the start of beat 5 is 10 x 4 / 16 = **2.5 V**, the start of beat 9 (the khali) is **5 V**, and the start of beat 13 is **7.5 V**.

### A beat count that never drifts

A 3-beat cycle against a 4-beat cycle lines up again only every 12 beats (3 x 4). This is the lowest common multiple. It is why polymetre feels like it "comes back home" after a long journey. Recipe 7 uses this.

## 3. The panel

**Display.** Shows the cycle name, the current beat out of the total (for example `5 / 16`), and the sections (for example `4+4+4+4`). When a new cycle has been chosen but has not started yet, the bottom line says `next:` and the new name.

**Row of four knobs**

| Control | What it does |
| --- | --- |
| TEMPO | 30 to 300 beats per minute. Used by the built-in clock. If a clock cable is plugged into CLOCK IN, this knob is ignored. With a cable in TEMPO CV the knob sets the starting tempo. |
| CYCLE | Chooses the cycle, 1 to 55 in the order of Appendix A. Hover over the knob to see the name and number of beats. |
| CV AMT | How strongly the CYCLE CV jack moves the cycle choice. In LEVEL mode it is "steps per volt", from -6 to +6. The tick at the top is zero. Default is +1. It has no effect in COUNT mode. |
| GATE LEN | How long the gates stay on, as a share of one beat, from 5% to 95%. Default 50%. |

**Row of buttons and switches**

| Control | What it does |
| --- | --- |
| RUN | Starts and stops the clock. The button is blue when running. When stopped, Cycle Clock ignores outside clock pulses too, and it carries on from the same place when you start again. |
| RESET | Press to make the next beat beat one. |
| ACCENT | ON: the click is higher and louder on beat one of the cycle. OFF: every beat gets the same click. |
| CLICK (jack) | An audio output. It plays a short tick on every beat, so you can hear where you are. Plug it into a mixer or headphones. It is not needed by the rest of the patch. |

**Row of three inputs and one switch**

| Control | What it does |
| --- | --- |
| CLOCK IN | An outside beat clock. One pulse is one beat. When a cable is plugged in, Cycle Clock follows it and the built-in clock is switched off. |
| RESET IN | A trigger. The next beat becomes beat one. |
| CYCLE CV | Chooses the cycle from a voltage or pulses. See "Choosing the cycle with CV" below. |
| CV MODE | Switch for the CYCLE CV jack. Down is LEVEL, up is COUNT. |

**Row of four: TEMPO CV, RUN IN, POSITION and SWING**

| Control | What it does |
| --- | --- |
| TEMPO CV | Tempo from a voltage, 1 volt per octave, the same as pitch. +1 V doubles the tempo and -1 V halves it. The result is kept between 20 and 400 BPM. Built-in clock only. |
| RUN IN | A trigger. Each pulse switches between running and stopped, the same as pressing RUN. |
| POSITION (jack) | A voltage from 0 to 10 V that rises across the cycle. |
| SWING | Delays every second beat of the built-in clock, from 0% (straight) to 100% (half a beat late). About 67% is triplet swing. Every pair of beats still lasts exactly two beats, so the cycle never drifts. It does not touch an outside clock, so swing that before it reaches CLOCK IN. |

**The output block** is two rows of four jacks. The top row is **triggers**, labelled with names. The bottom row is **gates**, labelled GATE. Each gate sits under the trigger it belongs to.

| Column | Trigger (top) and gate (bottom) fire on |
| --- | --- |
| BEAT | Every beat |
| SECTION | The first beat of every section (this includes beat one) |
| CYCLE | The first beat of the cycle only (beat one, "sam") |
| END | The last beat of the cycle only |

### How it counts

- **Built-in clock.** With nothing plugged into CLOCK IN and RUN on, the first beat fires the moment it starts. After that, beats come at the TEMPO.
- **Outside clock.** With a cable in CLOCK IN, every pulse (a rising edge above about 2 V) is one beat. Send it one pulse per beat, not per sixteenth note. If your clock runs faster, divide it first.
- **Reset.** RESET or RESET IN makes the next beat beat one. With the built-in clock that happens straight away. With an outside clock, it happens on the next clock pulse.
- **Changing the cycle.** When you turn the CYCLE knob or the CV changes, the new cycle waits until the current cycle finishes and starts on beat one of the next lap. This keeps the music from breaking in the middle. (If the clock is waiting on beat one, for example just after a reset, it changes at once.) The display shows `next:` while it waits.

### The outputs in detail

**Triggers** are 10 V for 1 millisecond.

**Gates** are 10 V for a share of one beat, set by GATE LEN. At 120 BPM with GATE LEN at 50%, each gate lasts 250 ms. The gate is always shorter than the beat (95% at most), so there is always a small gap between gates. This is what an envelope needs to retrigger cleanly. With an outside clock, the gate length is worked out from the length of the previous beat, so it follows tempo changes one beat late.

**POSITION** is `10 x (beats gone + part of this beat) / beats in cycle`. It is smooth, but with an outside clock it is estimated from the previous beat's length, so if you speed up or slow down suddenly it will run a little ahead or behind for one beat. It goes back to 0 V on reset.

**CLICK** is a short sine tick. The normal click is about 1.1 kHz at moderate level. The accented click is about 1.8 kHz at full level (5 V peak). Both die away in about 0.1 seconds. Accent falls on beat one of the cycle only.

### Choosing the cycle with CV

The CYCLE CV jack has two modes. Flip the CV MODE switch to choose.

**LEVEL mode (switch down).** The voltage moves the choice away from where the CYCLE knob points.

```
chosen position = CYCLE knob position + (CV in volts x CV AMT)
```

The result is rounded to the nearest whole position and kept between 1 and 55.

| CYCLE knob | CV in | CV AMT | Result |
| --- | --- | --- | --- |
| 14 (Tintal) | 0 V | +1 | 14 (Tintal) |
| 14 | 2 V | +1 | 16 (Dadra) |
| 14 | 1 V | +3 | 17 (Rupak) |
| 14 | 1 V | -1 | 13 (12 beats, 3+3+3+3) |
| 1 | 5 V | +4 | 21 (Dhruva 3) |

Good for: a sequencer with a different note per step (each note picks a cycle), a slow LFO that wanders through neighbouring cycles, an envelope, or a random voltage. A gate works too: 0 V keeps the knob choice, and 10 V with CV AMT at +0.4 adds 4 positions. That gives you two cycles that you flip between.

Because the change always waits for the next lap, a fast LFO is safe. You will hear the cycle that was chosen at the moment each lap ends.

**COUNT mode (switch up).** Every pulse moves one position further along the list. After position 55, it goes back to 1. CV AMT is ignored.

A pulse is anything that rises above about 2 V, so triggers, gates and square-wave LFOs all work. Start with the knob where you want to begin. The first pulse moves one step further.

Good for: stepping through thalams song by song with a trigger from a sequencer, or changing at every lap by patching the CYCLE trigger back into CYCLE CV (see the recipe "change the thalam every section").

The counter starts again from the knob position whenever you unplug the cable or switch back to LEVEL.

### Tempo CV, run and swing

**TEMPO CV** sets the tempo of the built-in clock from a voltage. It follows 1 volt per octave, the same as pitch, so +1 V doubles the tempo and -1 V halves it. The TEMPO knob is the starting point.

| TEMPO knob | CV in | Result |
| --- | --- | --- |
| 120 BPM | 0 V | 120 BPM |
| 120 BPM | +1 V | 240 BPM |
| 120 BPM | -1 V | 60 BPM |
| 120 BPM | +0.5 V | about 170 BPM |

The result is kept between 20 and 400 BPM. With an outside clock in CLOCK IN, the outside clock sets the tempo and TEMPO CV does nothing.

**RUN and RUN IN.** The RUN button and the RUN IN jack do the same job. Each press or pulse switches between running and stopped. While stopped, Cycle Clock makes no beats, whether the clock is the built-in one or an outside one. When you start again it carries on from the same place in the cycle. Press RESET to go back to beat one.

**SWING** delays every second beat of the built-in clock. At 0% the beats are evenly spaced. At 100% the second beat of each pair arrives half a beat late. About 67% gives triplet swing. At 120 BPM a beat lasts 500 ms, so 67% delays the second beat by about 167 ms and 100% delays it by 250 ms.

Every pair of beats still lasts exactly two beats, so the cycle never drifts. All outputs swing together: BEAT, SECTION, CYCLE, END and CLICK.

Things to know about swing:

- It works on the built-in clock only. If you use an outside clock, swing that clock before it reaches CLOCK IN.
- The clock swings every second clock beat, not every second beat of the cycle. In a cycle with an odd number of beats, the late beats fall on different beats from one lap to the next.
- POSITION is worked out for straight time, so while a beat is late it pauses briefly and then catches up.


### Specifications

| | |
| --- | --- |
| Width | 14 HP |
| Cycles | 55 (13 bars, 7 Hindustani, 35 Carnatic) |
| Beats per cycle | 2 to 29 in the presets |
| Tempo | 30 to 300 BPM on the knob, 20 to 400 BPM with TEMPO CV |
| TEMPO CV | 1 V per octave |
| Swing | 0% to 100% (built-in clock only) |
| Gate length | 5% to 95% of a beat |
| Trigger | 10 V, 1 ms |
| Gate | 10 V |
| Position | 0 to 10 V |
| Click | Audio, 5 V peak |
| Input pulse | Rising edge above about 2 V |

## 4. Patch recipes

Each recipe says what you plug into what, and what you should hear.

### Recipe 1: a thalam you can hear

*Goal: hear the shape of Tintal.*

1. Add Cycle Clock. Set CYCLE to Tintal (position 14), TEMPO to 100.
2. Patch BEAT (trigger) into a soft hi-hat or shaker.
3. Patch SECTION (trigger) into a clap or woodblock.
4. Patch CYCLE (trigger) into a kick or low drum.
5. Plug CLICK into a mixer channel and set ACCENT on.

You hear a steady hi-hat, a clap every four beats, and a deep hit every sixteen beats. The deep hit is sam, beat one.

Try Rupak (position 17). The sections are 3 + 2 + 2, so the section pulse lands on beats 1, 4 and 6 of every seven. This is a good way to feel a seven-beat cycle without counting.

Rupak is unusual. Beat 1, the sam, is also the khali: musicians wave on it instead of clapping. The claps fall on beats 4 and 6. (Sources: [Octaves Online](https://www.octavesonline.com/post/rupak-taal-the-7-beat-riddle-you-need-to-solve), [Chandrakantha](https://chandrakantha.com/music-and-dance/i-class-music/index-of-tals/rupak/).)

### Recipe 2: envelopes on every beat

*Goal: a plucked sound that opens on every beat.*

1. Patch Cycle Clock BEAT (gate) into an ADSR envelope.
2. Patch the envelope into a VCA (volume control) that your oscillator goes through.
3. Set GATE LEN to about 30% for short plucks, or 90% for held notes.

Because every gate is shorter than the beat, the envelope always restarts cleanly. A gate is what an envelope needs. A trigger would not hold it open.

### Recipe 3: a sweep that spans a whole cycle

*Goal: a filter that opens slowly across 16 beats and snaps shut at sam.*

1. Set Tintal.
2. Patch POSITION into the cutoff of a filter, with a bit of attenuation.

POSITION is 0 V on beat one, 2.5 V on beat five, 5 V on beat nine (the khali), 7.5 V on beat thirteen, then back to 0 V. You hear the sound open for the whole lap and reset.

### Recipe 4: open the door on the last beat

*Goal: a fill or an effect only on the last beat of every cycle.*

1. Patch END (gate) into an envelope or an attenuator that controls a reverb send, a delay feedback, or a filter.

The gate is on for part of the last beat only. You get a build-up into sam every lap.

### Recipe 5: change the thalam every section

*Goal: a new cycle on every lap, stepping through the list.*

1. Patch Cycle Clock CYCLE (trigger) into its own CYCLE CV jack.
2. Set CV MODE to COUNT.
3. Set the CYCLE knob to Tintal (14).

The first lap plays Tintal. After that, each lap the clock steps to the next cycle in the list: Keherwa, Dadra, Rupak and so on. Because a change always waits for the end of the lap, nothing breaks in the middle.

### Recipe 6: odd bars with a clear shape

*Goal: a rhythm in 7 that is easy to follow.*

1. Choose `7 (2+2+3)` (position 8).
2. SECTION (trigger) into a clap, BEAT (trigger) into a hi-hat, CYCLE (trigger) into a kick.

You hear 2, 2, 3. Compare it with `7 (3+2+2)` (position 9). Same seven beats, different shape.

### Recipe 7: two cycles that grow apart and come back

*Goal: a polymetre, where two patterns of different length drift and then meet.*

1. Add two Cycle Clocks. Feed both from the same clock, or give both the same TEMPO.
2. Set one to `3` (position 2) and the other to `4` (position 3).
3. Patch each CYCLE (trigger) into a different drum.

The two drums line up again only every 12 beats, which is 3 x 4. Try 5 against 4 (every 20 beats), or 7 against 4 (every 28 beats). Patch each CYCLE trigger into the RESET IN of a step sequencer if you want sequences of different lengths to realign.

### Recipe 8: practise with the click

*Goal: learn a thalam by ear.*

1. Cycle Clock: choose your thalam, with TEMPO on a slow 50.
2. ACCENT on. Plug CLICK into headphones.

You hear one high click on beat one and low clicks on the rest. Count along with your hands (see the Tintal table in section 1). Raise the tempo slowly as you get comfortable. Switch ACCENT off, and see whether you can still find sam.

### Recipe 9: a swung groove

*Goal: a laid-back feel from the built-in clock.*

1. Choose the cycle `4` (position 3). Leave CLOCK IN empty.
2. Turn SWING to about 67%.
3. Patch BEAT (trigger) into a hi-hat and CYCLE (trigger) into a kick.

Every second hi-hat arrives late, and the kick still lands exactly on beat one of each lap.

### Recipe 10: a tempo that breathes

*Goal: a clock that drifts a little faster and slower.*

1. Patch a slow LFO into TEMPO CV. Turn its level down so it swings about plus and minus 0.2 V.
2. Set TEMPO to 120.

The tempo wanders between about 104 and 138 BPM. Every Hemiola Audio output follows it.

### Recipe 11: stop and start from a button

*Goal: pause the whole patch without touching the mouse.*

1. Patch a button or a sequencer trigger into RUN IN.

The first pulse stops the beats. The next pulse starts them again, from the same place in the cycle.


## 5. Questions and answers

**Nothing is coming out of Cycle Clock.** Check that RUN is on (blue). If a cable is in CLOCK IN, the built-in clock is off, and the module waits for pulses.

**The CYCLE knob moved but the display still shows the old cycle.** The change waits for the end of the current lap. The bottom line says `next:`. Press RESET to switch at once.

**The TEMPO knob does nothing.** A cable in CLOCK IN switches the built-in clock off, so the outside clock sets the tempo.

**SWING does nothing.** Swing works on the built-in clock only. Remove the cable from CLOCK IN, or swing the outside clock.

**The cycle stopped when I stopped my outside clock.** That is right. Cycle Clock follows whatever clock it is given. It also stops when RUN is off, even with an outside clock.

**What is the difference between a trigger and a gate?** A trigger is a short blip for drums. A gate stays on for a while, for envelopes. See section 1.

**Which output should I use for a drum?** The trigger. **For an envelope?** The gate.

**My sequencer's CV does not select the cycle I expect.** In LEVEL mode, the choice is `CYCLE knob + volts x CV AMT`. With CV AMT at +1, one volt moves one position. For wider jumps, turn CV AMT up. For a different starting point, move the CYCLE knob.

**Can I use CV that is not a clean pulse in COUNT mode?** Yes. It counts every time the signal rises above about 2 V. A noisy signal can count more than once, so use a clean pulse, a gate or a square-wave LFO.

**Does it work at any sample rate?** Yes. All timing is calculated from the sample rate Rack is set to.

---


## Appendix A: all 55 cycles

Position is the number on the CYCLE knob, from 1 to 55. "Beats" is the number of beats. "Sections" shows how the beats are grouped.

Names with a number after them (such as Rupaka 4) are Carnatic thalams, and the number is the laghu size (the jati). Ektal is counted 2+2+2+2+2+2 here, as most sources give it, although some teachers group it 4+4+4 or 4+4+2+2. Some thalam names have more than one common spelling, for example Keherwa and Keharwa, or Dhamar and Dhammar.

| Position | Name | Beats | Sections | Family |
| --- | --- | --- | --- | --- |
| 1 | 2 | 2 | 2 | Meter |
| 2 | 3 | 3 | 3 | Meter |
| 3 | 4 | 4 | 4 | Meter |
| 4 | 5 (3+2) | 5 | 3+2 | Meter |
| 5 | 5 (2+3) | 5 | 2+3 | Meter |
| 6 | 6 (3+3) | 6 | 3+3 | Meter |
| 7 | 6 (2+2+2) | 6 | 2+2+2 | Meter |
| 8 | 7 (2+2+3) | 7 | 2+2+3 | Meter |
| 9 | 7 (3+2+2) | 7 | 3+2+2 | Meter |
| 10 | 8 (4+4) | 8 | 4+4 | Meter |
| 11 | 9 (3+3+3) | 9 | 3+3+3 | Meter |
| 12 | 11 (3+3+3+2) | 11 | 3+3+3+2 | Meter |
| 13 | 12 (3+3+3+3) | 12 | 3+3+3+3 | Meter |
| 14 | Tintal | 16 | 4+4+4+4 | Hindustani |
| 15 | Keherwa | 8 | 4+4 | Hindustani |
| 16 | Dadra | 6 | 3+3 | Hindustani |
| 17 | Rupak | 7 | 3+2+2 | Hindustani |
| 18 | Jhaptal | 10 | 2+3+2+3 | Hindustani |
| 19 | Ektal | 12 | 2+2+2+2+2+2 | Hindustani |
| 20 | Dhamar | 14 | 5+2+3+4 | Hindustani |
| 21 | Dhruva 3 | 11 | 3+2+3+3 | Carnatic |
| 22 | Dhruva 4 | 14 | 4+2+4+4 | Carnatic |
| 23 | Dhruva 5 | 17 | 5+2+5+5 | Carnatic |
| 24 | Dhruva 7 | 23 | 7+2+7+7 | Carnatic |
| 25 | Dhruva 9 | 29 | 9+2+9+9 | Carnatic |
| 26 | Matya 3 | 8 | 3+2+3 | Carnatic |
| 27 | Matya 4 | 10 | 4+2+4 | Carnatic |
| 28 | Matya 5 | 12 | 5+2+5 | Carnatic |
| 29 | Matya 7 | 16 | 7+2+7 | Carnatic |
| 30 | Matya 9 | 20 | 9+2+9 | Carnatic |
| 31 | Rupaka 3 | 5 | 2+3 | Carnatic |
| 32 | Rupaka 4 | 6 | 2+4 | Carnatic |
| 33 | Rupaka 5 | 7 | 2+5 | Carnatic |
| 34 | Rupaka 7 | 9 | 2+7 | Carnatic |
| 35 | Rupaka 9 | 11 | 2+9 | Carnatic |
| 36 | Jhampa 3 | 6 | 3+1+2 | Carnatic |
| 37 | Jhampa 4 | 7 | 4+1+2 | Carnatic |
| 38 | Jhampa 5 | 8 | 5+1+2 | Carnatic |
| 39 | Jhampa 7 | 10 | 7+1+2 | Carnatic |
| 40 | Jhampa 9 | 12 | 9+1+2 | Carnatic |
| 41 | Triputa 3 | 7 | 3+2+2 | Carnatic |
| 42 | Adi | 8 | 4+2+2 | Carnatic |
| 43 | Triputa 5 | 9 | 5+2+2 | Carnatic |
| 44 | Triputa 7 | 11 | 7+2+2 | Carnatic |
| 45 | Triputa 9 | 13 | 9+2+2 | Carnatic |
| 46 | Ata 3 | 10 | 3+3+2+2 | Carnatic |
| 47 | Ata 4 | 12 | 4+4+2+2 | Carnatic |
| 48 | Ata 5 | 14 | 5+5+2+2 | Carnatic |
| 49 | Ata 7 | 18 | 7+7+2+2 | Carnatic |
| 50 | Ata 9 | 22 | 9+9+2+2 | Carnatic |
| 51 | Eka 3 | 3 | 3 | Carnatic |
| 52 | Eka 4 | 4 | 4 | Carnatic |
| 53 | Eka 5 | 5 | 5 | Carnatic |
| 54 | Eka 7 | 7 | 7 | Carnatic |
| 55 | Eka 9 | 9 | 9 | Carnatic |

**Preset review.** The ordinary meters are plain counting. The Hindustani and Carnatic structures were checked against published sources (see Sources below). They are open for review, and corrections are welcome through the issues page of the project on GitHub.

### Sources

- Hindustani thalam structure (Tintal clap and wave positions): [Teental, Wikipedia](https://en.wikipedia.org/wiki/Teental)
- Hindustani thalam divisions: [North Indian Talas, Ancient Future](https://ancient-future.com/theka.html)
- Hindustani beat counts and groupings (all seven): [Vibhag, Wikipedia](https://en.wikipedia.org/wiki/Vibhag)
- Hindustani groupings, cross-check: [Tal, Kathak World](https://www.kathakworld.com/tal-2/) and [Thekas, University of Washington](https://sites.math.washington.edu/~gangolli/Music428Thekas.html)
- Rupak, sam and khali on the same beat: [Octaves Online](https://www.octavesonline.com/post/rupak-taal-the-7-beat-riddle-you-need-to-solve), [Chandrakantha](https://chandrakantha.com/music-and-dance/i-class-music/index-of-tals/rupak/)
- Carnatic laghu, drutam, anudrutam, jati sizes, the seven thalams and Adi: [Carnatic taala, karnatik.com](https://karnatik.com/ctaala.shtml)

---


## About Hemiola Audio

Hemiola Audio is a new developer, and a very small one: one person. I make music in VCV Rack most days, and these modules came from things I wanted on my own patches and could not find. I am not a trained programmer. I write the code with an AI assistant and decide everything else myself: what a module should do, how it should look, and whether it feels right under the hands.

The timing rules in the modules are covered by automatic tests. Whether a module is musical is decided in a patch, by ear, and I would like to hear what you find.

More modules are planned, and not all of them will be about rhythm: instruments to play as well as tools to keep time. They are free and open source. If something is wrong, unclear or missing, please open an issue on GitHub or write to hemiolaaudio@gmail.com.

## Credits and licence

Hemiola Audio is free software under the GNU General Public License, version 3 or later. The panel lettering is Montserrat, used under the SIL Open Font Licence 1.1 (the licence text is in `res/fonts/OFL.txt`). The Hemiola Audio name and logo belong to Hemiola Audio.
