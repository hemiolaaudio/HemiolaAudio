# Hemiola Audio manuals

Version 2.0.0. Free and open source (GPL-3.0-or-later).

Hemiola Audio makes modules for people who think about rhythm in cycles. This pack has two. Each has its own manual. Pick the one you need:

| Module | Width | Manual | In one sentence |
| --- | --- | --- | --- |
| **Cycle Clock** | 14 HP | [Cycle Clock manual](docs/CycleClock.md) | A clock that counts a whole rhythm cycle, such as a 16-beat thalam or a 7/8 bar, and tells you where you are inside it. |
| **R&T** (Rhythm & Thalam) | 10 HP | [R&T manual](docs/RhythmTalam.md) | Splits every beat into 1 to 9 equal parts, the way Carnatic music counts nadai, and can divide those parts down again so fast nadai are easy to count. |

You do not need to know Indian music to use them. Both manuals explain everything from the start.

## Using the two together

Patch the **BEAT** trigger of Cycle Clock into **BEAT IN** on R&T. Cycle Clock counts the thalam, and R&T fills each beat with 3, 4, 5, 7 or 9 parts. Patch the **CYCLE** trigger of Cycle Clock into **RESET IN** on R&T to keep R&T's divided pulses on the bar. Both manuals have patch recipes for this.

## In every Hemiola Audio module

- **Triggers** are 10 V for 1 millisecond. Use them for drums.
- **Gates** are 10 V for a share of the beat, part or step, set by GATE LEN, and are always shorter than the step so envelopes retrigger cleanly. Use them for envelopes.
- **CLICK** is an audio output that ticks on every beat or part, with an accent, so you can count by ear.
- Every pulse input looks for a rising edge above about 2 V.

## Getting help

Open an issue on [GitHub](https://github.com/hemiolaaudio/HemiolaAudio/issues) or write to hemiolaaudio@gmail.com.

[Changelog](CHANGELOG.md)


## About Hemiola Audio

Hemiola Audio is a new developer, and a very small one: one person. I make music in VCV Rack most days, and these modules came from things I wanted on my own patches and could not find. I am not a trained programmer. I write the code with an AI assistant and decide everything else myself: what a module should do, how it should look, and whether it feels right under the hands.

The timing rules in the modules are covered by automatic tests. Whether a module is musical is decided in a patch, by ear, and I would like to hear what you find.

More modules are planned, all in the same spirit: tools for people who think about rhythm in cycles. They are free and open source. If something is wrong, unclear or missing, please open an issue on GitHub or write to hemiolaaudio@gmail.com.

## Credits and licence

Hemiola Audio is free software under the GNU General Public License, version 3 or later. The panel lettering is Montserrat, used under the SIL Open Font Licence 1.1 (the licence text is in `res/fonts/OFL.txt`). The Hemiola Audio name and logo belong to Hemiola Audio.

