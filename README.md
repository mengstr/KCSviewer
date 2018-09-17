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

# Sep 02 - FSK→UART

### Design & Schematics 

The first part of the process to convert the FSK-audio into a plain UART-like serial stream would be to shape it up into a nice square wave with full swing between GND and VCC.

I'm sure there's many ways of doing this, but I just made a long tailed differential amplifier with the second side fixed to the same bias voltage as is set on the fist side.  This basically makes it into a comparator. Maybe some hysteresis would be a good thing to have as well, but I'll leave that out for the time being.

The now digital output (DIGITAL in the schematic) is used as a trigger for a monostable set at 300us.  The 300us is longer than the 2400Hz pulse width so while 2400Hz is present the monoflop will continuously be re-triggered and will never time out.  As soon as the input frequency changes to 1200Hz the timer will timeout and thus output a pulse before the next re-trigger some 10's of microseconds later.

At this point (LONGS in the schematic) there will be a pulse train whenever 1200Hz is present and a steady level whenever 2400Hz is present.

If this is fed into another monostable set at a even longer timeout (1100us) then the pulses in the pulse train will re-trigger the monostable and keep it active as long as the pulses are available.

The output from this last monostable is a plain and simple UART serial stream.

![Schematics of the Audio-to-Serial converter](Images/Audio_Serial-Schematics.png?raw=true)

### Prototype

I first built this on a solderless breadboard to verify my previous LTspice simulations. It worked like a charm!

![Breadboard of the Audio-to-Serial converter](Images/Audio_Serial-Breadboard.jpg?raw=true)

### PCB

Lately I've started to actually plan and design whenever I solder something on veroboard or donut-boards.  For this I use [DiyLC](http://diy-fever.com/software/diylc/)

So after a bit of trying out a few different layouts I came up with this design.

![Layout of the Audio-to-Serial converter PCB](Images/Audio_Serial-Layout.png?raw=true)

I find it easier to follow the layout during soldering if I first mark the tracks-to-be on the top with a pen like this:

![Tracks on the Audio-to-Serial converter PCB](Images/Audio_Serial-PCB-empty.jpg?raw=true)

After this it's easy enough to just plonk down the parts, bend the component leads at the bottom in the right directions and solder&snip.

![Soldered Audio-to-Serial converter PCB](Images/Audio_Serial-PCB-built.jpg?raw=true)

### Testing

Feeding it a short FSK sequence from the Arduino KCS simulator I built/wrote yesterday it looks very good.

![Audio-to-Serial converter oscilloscope curves](Images/Audio_Serial-Oscilloscope.png?raw=true)

On the top in orange color we have the AUDIO input starting out at a (slightly attenuated) 2400 Hz followed by four cycles of 1200Hz, eight cycles 2400 and then some more 1200.

The next trace is the green DIGITAL output from the comparator.

The yellow trace is the LONGS pulse train from the output of the first 300us monostable.

And finally the blue trace which is the SERIAL output from the second 1100us monostable.

---

## Sep 17 - Baud clock & Ring counter

No noes! Real life happened and made me work basically around the clock for two weeks, but now I got some breathing room to play with my #retrochallenge project for a bit again. 

### Done this time

I was running out of 2.2k resistors and also started to get log on NPN's in TO-92 so I ordered 500 each of 2k2 and 10k plus a hundred each of NPN & PNP transistors. I hope they will not run out while doing this project..

![More parts](Images/MoreParts.jpg?raw=true)

My original plan was to do the startbit detection mono- and flipflops, but I realized that I needed the ring counter first in order to test that part of the design.  So I went ahead with the clock generator, ring-bias and ring-stages this week.

![Block diagram of the Clock/Bias/Ring modules](Images/RingCounter_Clock_Bias-Block.png?raw=true)


### Baudrate generator

In order to be able to collect the bits as they come in at 300 bits per second I need a 300Hz clock pulse that can be synched to the leading edge of the startbit. Without the syncing the sample point might end up anywhere in each bitslot and we want to sample as close to the middle as possible to have as much margin as possible if the playback tape speed is a tad slower or faster than the recording tape deck. Also the frequency of the clock generator here will vary a bit in response to the temperature.

A regular UART-chip usually have a clock that is 16 times the bit rate so it can digitally select the correct number of clock pulses to wait before reaching the mid-point, and they can also make multiple samples in each bit to ignore glitches.  That is fine and dandy when you do it digitally and can use huge number of transistors inside the chip, but this project is more of less analog and have to be a bit frugal on the parts count so something else has to be done.

The idea is to have the clock generator stopped until a flipflop is "set" by the rising edge of the startbit in the serial bitstream.  When the flipflop is set then the clock generator is started and will be free-running until all data bits are collected. At the stopbit the flipflop will be "reset" and the clock will be stopped.

The clock circuit looks like this:

![Schematics of the Clock generator](Images/Clockgen-Schematics.png?raw=true)

The C1 and the resistors R8 and R9 (together with D1 ) controls the frequency as well as the pulse width ratio.  The diode makes sure that only the charging cycle of C1 will be affected by the low resistance of R9 (making it fast).  The discharge will be done thru the much higher resistance of R8 making it slow.

This gives me pulses at 300 Hz with a very short low-going output that will be used to advance the ring counter one step.


The RESET/stop input of this module merely keeps the C1 capacitor in a permanent discharged state as long as it is asserted.

I just now discovered that there's a direct path between V+ and GND via Q3 and Q5 when the circuit is in the reset state - not good. The simulation says only 36 mA will flow though - that's only 180mW. But that is if one can trust the simulation to 100%. 

I guess I have to come up with some better solution to the reset. I'd probably be better to hoo kQ5 directly up to C1 - I think my original thought was that I wanted to force the output transistor off directly with the reset, but that's probably not really necessary.


### The bias generator

The chain of ring counter modules needs two things to operate. A the clock pulse (which is merely a short interruption of the VCC supply current into the modules), and a adjustable bias voltage.

The bias module is as simple as it gets - just a simple emitter follower controlled by a pot to adjust the voltage.

In the schematics (from the LTspice simulations) below I replaced the pot with the two R1 & R2 transistors.

![Schematics of the Bias generator](Images/Bias-Schematics.png?raw=true)

Both the clockgen and the bias got fitted onto a single PCB without any issues.

![DIYlc layout of the Clock & Bias PCB](Images/Clock_Bias-Layout.png?raw=true)

Ending up in real life as this:

![Clock & Bias PCB](Images/Clock_Bias-PCB.jpg?raw=true)

The picture was taken before I changed the R1/R2 voltage divider into a pot to be able to tweak the bias voltage.

### Ring counter

Each step of the ring counter is rather easy. They are being based on on a discrete BJT version of the usual thyristor-based ring counters.

The thyristors are build out of a tightly coupled NPN/PNP pair so they almost end up a 4-layer N/P device.

![Single stage of the ring counter](Images/Ringcounter_Stage-Schematics.png?raw=true)

The RI and RO connects from/to the previous/next stage by a 100nF capacitor. The STEP is the power supply with brief interruptions coming from the baudrate/clock generator.  Each dip in the power moves the active stage one step to the right.  It works almost by magic ;-)

I hooked up three staked on the breadboard and connected the clock/bias PCB to it and the pulses shifted around beautifully just as planned.

![Ring counter breadboard test](Images/Ringcounter-Breadboard.jpg?raw=true)

Screenshot from the oscilloscope showing the incoming STEP pulses and the outputs from each of the three stages.

![Oscilloscope screenshot of a three stage ring](Images/Ringcounter-Oscilloscope.png?raw=true)

I did a quick layout with five stages on one of the usual PCBs - easy peasy.

![DIYlc layout of a 5 stage ring PCB](Images/Ring-Layout.png?raw=true)
 

 By some reason the soldering took a while to do and I might have fscked up something because it didn't behave properly at 5 volts. I got a lot of double pulses at the outputs or all just toggled in tandem.  Fiddling a bit with the bias voltage and raising the input voltage up to 12 made it better - but something is just simply wrong. 

![DIYlc layout of a 5 stage ring PCB](Images/5stageRing-PCB.jpg?raw=true)

I didn't have time last weekend (when I snuck a few hours to solder all of this up) to debug the issue further. I didn't even have time to make this blogpost back then.

But tomorrow I'll spend some time to get to the bottom with this and also then solder up the second 5-stage ring counter.

---
