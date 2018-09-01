![KCSviever banner](Images/PageHeader.png?raw=true)

#### A discrete transistor implemenation of a Kansas City Standard data tape decoder and viewer. This is my entry for the [RetroChallenge RC2018/09](http://www.retrochallenge.org/).

The entries/blog here will be kept in chronological order with the newest post at the bottom.

[![HitCount](http://hits.dwyl.io/SmallRoomLabs/KCSviewer.svg)](http://hits.dwyl.io/SmallRoomLabs/KCSviewer)

---

## Sep 01 - Intro

For this challenge I'll attempt to make a decoder/viewer for the old [Kansas City Standard](https://en.wikipedia.org/wiki/Kansas_City_standard) tapes that once was the standard for storing data and programs on regular cassette tapes.

Since I have a thing for using discrete transistors instead of ICs I'll make it with just Q's R's and C's. Maybe a D and some LEDs will appear as well, but definitely no ICs.

The KCS is a simple low speed (300 bps) [FSK](https://en.wikipedia.org/wiki/Frequency-shift_keying) (Frequency Shift Keying) that are using 1200 and 2400 Hz to represent the zeros and ones in the data stream.  A zero is four cycles of 1200Hz, and a one is eight cycles of 2400Hz.

Every byte are, just as in a regular asynchronous UART, prepended with a startbit that is a zero, followed by the eight databits, and then finally one or two stopbits that are high.

The next byte can then start directly after that (with its own startbit of course) or after an arbitrary number of high bits.

This fully asynchronous signalling makes it rather easy to decode since there's no real need to make a PLL to recover the clock.  Just as in an UART it's ok to just have a freerunning clock that is re-synchronized to the leading edge of each startbit. If the tape speed doesn't vary more than 5% the data should be decoded correctly.

---

## Sep 01 - Plan of attack

The final goal is to be able to decode the FSK audio coming from a cassette tape or a sound file into a 8-bit parallel data with a data-valid strobe so it can be read by a computer to load software into it.

As an extended more fun goal would be to connect the outputs to a 7-segment display to display the data in real time. Considering the asynchronous nature of the KCS the audio stream could have just a single byte every second and fill the rest with just idle/mark data.  Then a tape would display a message character by character on the display. Or to be a bit more fancy the eight databits could drive a standard HD44780 display in 4-bit mode to display a message on the 2x16 characters there.

Alternatively I could modify the standard a bit and have 14 databits instead of 8 and collect them all to be shown on a startbust display instead of the 7-segment. But that would require a higher degree of match between the freerunning clock and the actual speed of the tape.

So the high level overview of the design looks something like this

![Block Diagram](Images/BlockDiagram1.png?raw=true)

The pulse shaper will take the analog audio from the tape recorders line output, I guess something between 100 and 700mV, and shape it up into a nice digital square wave with logic (0/5 volt) levels.

This digital audio will still be modulated with the 1200/2400 Hz carrier so the next order of business will be to demodulate that so we get something that looks just like the output of a serial port.

The serial stream will enter both a shift register as well as a startbit detector.

The startbit detector will trigger (or resynchronize?) the oscillator that runs at 300 Hz, driving the SIPO (Serial In Parallel Out) shiftregister.

When all eight bits are shifted into the shiftregister it needs to be latched and a strobe pulse should be asserted. (This is not shown in the diagram above)

---

# Sep 01 - Simulating the tape

I recently got hold of a working casette tape deck, but it will be a bit of a hassle to use that during development so I made an Arduino sketch that outputs any FSK bit sequence I need.

The Arduino doesn't have any analog out so it's emulated with a PWM followed by a simple lowpass filter to get rid of most of the PWM frequency.

![Lowpass filter schematic](Images/FilterSchematics.png?raw=true)

I just soldered it up directly between a pinheader and a screw terminal.  The pinheader connects to GND, D2 and D3 which conveniently are located next to each other on a Nano.  D3 is used for the PWM and D2 is a reference output of the zeros and ones sent on the audio stream. This makes it easy to see what data bit that is actually being sent and how it passes through the circuit.

![Built lowpass filter ](Images/Filter.png?raw=true)

After writing some quick and dirty code (what else can be done using an Ardunio? ^__^ ) the output of a few bits looks like this

![Oscilloscope image of the simulated FSK](Images/ArduinoOutput1.png?raw=true)

The code is available at [GenerateKCS](https://github.com/SmallRoomLabs/KCSviewer/tree/master/GenerateKCS)

As can be seen the lowpass filter isn't the best since the 2400 Hz of the ones are getting attenuated a bit more than I had wished for, but it's good enough for the time being.  Maybe I'll make a steeper filter with a bit higher center frequency someday, we'll see....

---
