# About Hemiola Audio

Hemiola Audio is one person. I make music in VCV Rack most days, and I kept wanting modules that did not exist. One was a clock that knows which beat of a 16-beat cycle it is on, instead of only ticking. Another was a way to divide a beat into five, seven or nine equal parts, as Carnatic musicians do. So I started building them.

The name comes from the hemiola, a rhythm where three beats are played across two, or two across three. Cycles and divisions that do not line up neatly are what the first two modules deal with.

I am not a trained programmer. I build the modules with an AI assistant, which writes the code. I decide what each module is for, how it looks and what happens when you turn a knob. Automatic tests check every timing rule. Whether a module feels right is checked in a patch, with headphones on.

The modules are free and open source, and they will stay that way. The panels are pale and plain, and every label sits beside the control it names.

The first two work as a pair. The third, Whoa, is a voice.

**Cycle Clock** counts a whole rhythm cycle. In Indian music a cycle is a thalam, and Cycle Clock has 55 of them, from a plain bar of four to the 16-beat Tintal and the long Carnatic cycles. It tells your patch where each lap begins, where each section starts, and where you are in between.

**R&T**, short for Rhythm & Thalam, divides each beat. In Carnatic music the way a beat is divided is called nadai, and R&T gives you 1 to 9 equal parts. It can divide those parts down again, so nine to a beat is still easy to follow, and it can drop some of them at random while the beat stays where it is.

Patch one into the other and the clock follows the thalam while the divisions follow the beat.

**WHOA!** is an FM voice for techno, psytrance and dark ambient. It has one modulator and one carrier. Each has its own gate and envelope, with CV on every stage, so you can hold a note and move only the tone. It has a clocked delay and a reverb built in. It is the first Hemiola module that makes sound itself.

More modules are planned. Each one will start from something I wanted in a patch and could not find. If something feels wrong or missing, write to hemiolaaudio@gmail.com or open an issue on GitHub.

Hemiola Audio
