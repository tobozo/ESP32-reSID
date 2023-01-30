## ESP32-reSID library

This project is an attempt to get [reSID](https://en.wikipedia.org/wiki/ReSID) emulation engine to run on ESP32 as an Arduino library.

It is inspired by (and uses bits of) [SIDKick](https://github.com/frntc/SIDKick), a Teensy 4.1 implementation.


Roadmap:
- Convert AudioStream i/o calls syntax from Teensy to ESP32
- Migrate AudioStreamSID to ESP32 I2S
- Figure out how the ESP32 can also emulate the C64 CPU


Credits:

- http://www.zimmers.net/anonftp/pub/cbm/crossplatform/emulators/resid/index.html
- https://www.pjrc.com/teensy/teensyduino.html
- https://github.com/frntc/SIDKick
