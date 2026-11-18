# WHOA!, dark ambient: patch guide

Patch file: [Hemiola Whoa Dark Ambient.vcv](../patches/Hemiola%20Whoa%20Dark%20Ambient.vcv) | Made with [WHOA!](Whoa.md) and [Cycle Clock](CycleClock.md) | Hemiola Audio | Free and open source (GPL-3.0-or-later)

A slow bass line in a very large space. Four notes, each held for a whole section, with the tail never quite finishing before the next one arrives.

**What you need.** Rack 2.6.6 or newer, Hemiola Audio 2.1.0, and the Fundamental and Core modules that come with Rack.

**To open it.** In Rack, choose File, then Open, and pick the patch file. Check the audio output on the Audio module. Give it a minute: at 40 BPM the notes are far apart.

## What is happening

1. **Cycle Clock** runs at 40 BPM in 4/4. Its SECTION trigger clocks a four-step **SEQ3** (C2, Eb2, Bb1, G2). Its SECTION gate is the note gate for WHOA!, so each note is held for the length of a section.
2. **WHOA!** plays at ratio 1.5, with a low index, a little feedback and slow attacks and releases on both envelopes. SPREAD is high, so the two sides drift against each other.
3. Three very slow **LFOs** go into INDEX CV, MOD D CV and the delay MIX CV. The tone brightens and darkens, the notes change shape, and the echoes swell in and out. CV depth is 50% (right-click menu). A 250 ms reverb PRE DELAY keeps each note clear of its tail.
4. The **delay** is a dotted quarter, clocked by the Cycle Clock BEAT trigger at CLOCK IN, with feedback at 60%.
5. The **reverb** is on Vast, with size and decay high.

## Things to try

- Turn REV DECAY to the top. The tail holds and builds. Turn it back down to let it clear.
- Switch REV TYPE to Shimmer for an octave-up wash.
- Change DLY TIME to 1/2 or one bar for longer echoes.
- Slow the Cycle Clock to 30 BPM.

This patch has not yet been heard in Rack by its author. If something sounds wrong, write to hemiolaaudio@gmail.com.
