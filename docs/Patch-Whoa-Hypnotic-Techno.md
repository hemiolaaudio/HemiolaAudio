# WHOA!, hypnotic techno: patch guide

Patch file: [Hemiola Whoa Hypnotic Techno.vcv](../patches/Hemiola%20Whoa%20Hypnotic%20Techno.vcv) | Made with [WHOA!](Whoa.md), [Cycle Clock](CycleClock.md) and [R&T](RhythmTalam.md) | Hemiola Audio | Free and open source (GPL-3.0-or-later)

One short riff that repeats, with the tone slowly changing underneath it. A kick and a closed hat keep the floor.

**What you need.** Rack 2.6.6 or newer, Hemiola Audio 2.1.0, and the Fundamental and Core modules that come with Rack.

**To open it.** In Rack, choose File, then Open, and pick the patch file. Check the audio output on the Audio module.

## What is happening

1. **Cycle Clock** runs at 130 BPM in 4/4. Its BEAT gate plays the kick and is also the ACCENT on WHOA!, so notes on the beat are louder and brighter. Its SECTION trigger resets the sequencer each bar so the loop cannot drift.
2. **R&T** divides each beat into eighths. That clock drives an eight-step **SEQ3** (the riff) and the closed hat.
3. **WHOA!** plays the riff as a short pluck: ratio 3, sustain at zero on both envelopes, release about 0.2 s. A short gate makes a short note.
4. Two slow **LFOs** move MOD D CV and INDEX CV, so the pluck opens and closes over about a minute while the notes stay the same.
5. The **delay** is a dotted eighth from the Cycle Clock BEAT trigger. The **reverb** is a small Room with a little mix.
6. The kick and hat are Fundamental modules that go to a mixer. They do not go through WHOA!'s effects.

## Things to try

- Edit the SEQ3 gate row to change the rhythm. Keep the same riff for a while: repetition is the point.
- Turn DLY FDBK up for longer trails, or switch DLY TIME to 1/8.
- Raise INDEX CV depth by raising the Voice CV depth in the right-click menu.
- Put REV TYPE on Hall and raise REV MIX for a bigger sound.

This patch has not yet been heard in Rack by its author. If something sounds wrong, write to hemiolaaudio@gmail.com.
