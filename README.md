![KCSviever banner](Images/PageHeader.png?raw=true)

#### A discrete transistor implemenation of a Kansas City Standard data tape decoder and viewer. This is my entry for the [RetroChallenge RC2018/09](http://www.retrochallenge.org/).

The entries/blog here will be kept in chronological order with the newest post at the bottom.

[![HitCount](http://hits.dwyl.io/SmallRoomLabs/KCSviewer.svg)](http://hits.dwyl.io/SmallRoomLabs/KCSviewer)

---

## Sep 01 - Intro

For this challenge I'll attempt to make a decoder/viewer for the old [Kansas City Standard](https://en.wikipedia.org/wiki/Kansas_City_standard) tapes that once was the standard for storing data and programs on regular cassette tapes.

Since I have a thing for using discrete transistors instead of ICs I'll make it with just Q's R's and C's. Maybe a D and some LEDs will appear as well, but definitely no IC's.

The KCS is a simple low speed [FSK](https://en.wikipedia.org/wiki/Frequency-shift_keying) (Frequency Shift Keying) that are using 1200 and 2400 Hz to represent the zeros and ones in the data stream.  A zero is four cycles of 1200Hz, and a one is eight cycles of 2400Hz.

Every byte sent are, just as in a regular asynchronous UART, are prepended with a startbit that is a zero, followed by the eight databits, and then finally one or two stopbits that are high.

The next byte can then start directly after that (with its own startbit of course) or after an arbitrary number of high bits.

This fully asynchronous signalling makes it rather easy to decode since there's no real need to make a PLL to recover the clock.  Just as in an UART it's ok to just have a freerunning clock that is re-synchronized to the leading edge of each startbit. If the tape speed doensn't vary more than 5% the data should be decoded correctly.

