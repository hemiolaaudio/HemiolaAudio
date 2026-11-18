# Minimal Techno, 131 BPM: patch guide

Patch file: [Hemiola Minimal Techno 131 BPM.vcv](../patches/Hemiola%20Minimal%20Techno%20131%20BPM.vcv) | Made with [Cycle Clock](CycleClock.md) and [R&T](RhythmTalam.md) | Hemiola Audio | Free and open source (GPL-3.0-or-later)

A four-on-the-floor groove in which Cycle Clock holds the bar, R&T splits the beat, and one chord stab slides against the bar in a three-against-four pattern (a hemiola). Nothing is sampled. Every sound is built from Rack's own Fundamental modules.

**What you need.** Rack 2.6.6 or newer, Hemiola Audio 2.0.0 (Cycle Clock and R&T), and the Fundamental and Core modules that come with Rack.

**To open it.** In Rack, choose File, then Open, and pick the patch file. Check the audio output on the Audio module at the bottom right of the patch. The first bar settles; the groove locks from bar 2.

## 1. What you hear

| Sound | When it plays |
| --- | --- |
| Kick | On every beat, four to the bar |
| Clap | On beats 2 and 4 |
| Closed hat | On every sixteenth |
| Open hat | On the off-beat eighths (the "and" of each beat) |
| Bass | A short repeating pattern, three notes in every eight sixteenths |
| Chord stab | Every three sixteenths (a dotted eighth), so it slides against the bar and comes home every three bars. A two-note chord that changes at every stab. |

The stab is the Hemiola part. The bar is 16 sixteenths long. The stab repeats every 3 sixteenths. Three does not divide 16, so each stab lands in a new place in the bar. After 48 sixteenths (3 bars, 5.5 seconds) the stab pattern and the bar are back together.

## 2. The timing at 131 BPM

| | Length |
| --- | --- |
| One beat | 458 ms |
| One eighth | 229 ms |
| One sixteenth | 114.5 ms |
| One dotted eighth (the stab) | 343.5 ms |
| One bar (4 beats) | 1.83 s |
| Tintal, 16 beats (4 bars) | 7.33 s |
| Stab and bar back together (3 bars) | 5.50 s |

## 3. The Hemiola modules in the patch

Row 1 of the patch, left to right.

| Module | Setting | Job |
| --- | --- | --- |
| **Cycle Clock 1** | TEMPO 131, CYCLE Tintal (4+4+4+4), GATE LEN 60% | The master clock. Its BEAT gate plays the kick. Its SECTION trigger (once a bar) resets the three sequencers so they always restart on the downbeat. Its BEAT trigger drives R&T 1, R&T 2 and Cycle Clock 2. Its CYCLE trigger resets Cycle Clock 2 every 16 beats. |
| **R&T 1** | DIVISION 4 (Chatusra, sixteenths), GATE LEN 30% | Splits each beat into four. Its PARTS gate plays the closed hat. Its PARTS trigger clocks the bass sequencer and Cycle Clock 3. |
| **R&T 2** | DIVISION 2 (eighths), GATE LEN 40% | Splits each beat into two. Its PARTS trigger clocks the open-hat sequencer and the delay, so the echoes follow the tempo. |
| **Cycle Clock 2** | CYCLE 2 | Counts beats in a cycle of two. Its END trigger lands on beats 2 and 4: the clap. |
| **Cycle Clock 3** | CYCLE 3, GATE LEN 50% | Counts R&T 1's sixteenths in a cycle of three. Its CYCLE gate opens the stab's envelope. Its CYCLE trigger steps the chord sequencer. |

## 4. Every Hemiola cable

| From | To | What it does |
| --- | --- | --- |
| Cycle Clock 1 BEAT trigger | R&T 1 BEAT IN, R&T 2 BEAT IN, Cycle Clock 2 CLOCK IN | One beat clock for everything |
| Cycle Clock 1 CYCLE trigger | Cycle Clock 2 RESET IN | Restarts the clap count with the 16-beat cycle |
| Cycle Clock 1 SECTION trigger | Bass sequencer reset, open-hat sequencer reset | Each bar starts on the downbeat |
| Cycle Clock 1 BEAT gate | Kick envelopes | Kick on every beat |
| R&T 1 PARTS trigger | Bass sequencer clock, Cycle Clock 3 CLOCK IN | Sixteenth-note clock |
| R&T 1 PARTS gate | Closed-hat envelope | Hat on every sixteenth |
| R&T 2 PARTS trigger | Open-hat sequencer clock, Delay clock | Eighth-note clock |
| Cycle Clock 2 END trigger | Clap envelope | Beats 2 and 4 |
| Cycle Clock 3 CYCLE trigger | Chord sequencer clock | A new chord at every stab |
| Cycle Clock 3 CYCLE gate | Stab envelope | The stab itself |

## 5. The other modules

- **Three sequencers.** Fundamental SEQ3 modules. The first plays the bass (8 steps, gates on steps 3, 6 and 7). The second plays the open hat (8 steps, gates on steps 2, 4, 6 and 8). The third is the chord sequencer (4 steps, two pitch rows for a two-note chord).
- **Kick.** A sine oscillator with a pitch envelope and an amplitude envelope.
- **Bass.** A sawtooth and a sine, one octave apart, into a low-pass filter with an envelope and a slow LFO.
- **Clap and hats.** One white-noise source into two high-pass filters: one for the clap, one shared by both hats. Each of the three sounds has its own envelope, and the open hat rings longer than the closed hat.
- **Stab.** Two slightly detuned oscillators into a low-pass filter with an envelope and a slow LFO, then a delay.
- **Mutes.** Six switches, one for each of kick, bass, clap, closed hat, open hat and stab. They go into a mixer and then the audio output.

## 6. Things to try

1. **Change the feel.** Set R&T 1 DIVISION to 3 for triplets or to 6 for six parts per beat. The hats, the bass and the stab all follow R&T 1, so everything shifts together.
2. **Change the drift.** Set Cycle Clock 3 CYCLE to 5 (either grouping) and the stab returns to the bar every 5 bars. Set it to 7 (either grouping) and it takes 7 bars. Set it to 4 and the drift stops: the stab lands in the same place in every beat.
3. **Break it down.** Switch mutes on the Mutes module. Leave only the hats and the stab for a break.
4. **Rewrite the patterns.** Click the gate buttons under each sequencer to move the bass hits and the open hats.
5. **Break the beat.** Set Cycle Clock 1 CYCLE to 7 (2+2+3). The bar now restarts after 2, 2 and 3 beats, so the sequencers restart at uneven points. The kick keeps its steady pulse.
6. **Change the tempo.** Turn Cycle Clock 1 TEMPO. Everything follows, because every clock in the patch comes from it.
7. **Change the echo.** Set R&T 2 DIVISION to 3. The delay time follows, and the open-hat pattern follows with it.

## 7. If something is wrong

- **Nothing plays.** Check the Audio module at the bottom right of the patch and your computer's output. Check that none of the six mutes is on.
- **Cycle Clock or R&T look like empty boxes.** Hemiola Audio is not installed. Install it from the release page and restart Rack.
- **The groove feels loose in bar 1.** That is normal. The clocks need one bar to learn the tempo.

## 8. Rebuild the heart of it in five cables

1. Cycle Clock **BEAT** trigger into R&T **BEAT IN**.
2. R&T **PARTS** trigger into the clock input of a sequencer.
3. Cycle Clock **SECTION** trigger into the sequencer's reset input.
4. Cycle Clock **BEAT** gate into an envelope, and the envelope into a VCA with a sine oscillator: a kick on every beat.
5. R&T **PARTS** gate into a second envelope and a VCA with filtered noise: a hat on every part.

Then change DIVISION and CYCLE and listen to what moves.
