# starter-pistol — a visual starting signal for deaf and hard-of-hearing young runners

## Why

Sprint and race starts are announced by sound: a starter's pistol (often
electronic these days) fires, and every runner reacts to the bang. For deaf
and hard-of-hearing kids, that signal simply isn't there. They're left
watching the other runners' bodies for the first sign of motion, which costs
them a real, measurable head start every single race.

Plenty of races for the general public have no visual start signal at all —
there's no flashing light built into the event, because the field has never
needed one. Where a visual cue does exist at the professional/Paralympic
level, the equipment is expensive, closed, and not something a club or a
school can just add to their local meet.

The obvious "fix" — wire something directly into the starter pistol — turns
out not to be allowed, for two separate reasons:

1. **Proprietary systems.** Modern starter pistols are often electronic and
   tied into the race's official timing system. They come with licences,
   warranties, and a defined way of operating that a third-party device
   cannot simply tap into.
2. **Sports regulations.** Even where the hardware would technically allow
   it, regulations don't permit plugging directly into the timing/starting
   system — anything wired in is a potential source of interference with
   race timing, which is a non-starter (no pun intended) for an official
   competition.

So this project takes the only remaining path: **listen for the shot
acoustically, exactly like a runner would, and turn it into a visual
signal.** Nothing is wired into the official equipment; it's a standalone,
non-interfering device that reacts to the same bang everyone else reacts to.

## What it is

An ESP32/ESP8266-based device (this repository's implementation runs on a
NodeMCU ESP-12E) with an I2S microphone. It listens continuously, filters
and analyses the incoming audio for the acoustic signature of a starter
shot, and — on detection — drives an LED output so deaf and hard-of-hearing
runners get their own start signal, at the same instant as everyone else's
ears do.

## Technical challenges

Building this turned out to involve more than "record audio, look for a
loud noise":

- **Regulations (as above).** No electrical connection to the starter
  pistol is possible, which is *why* this has to be an acoustic, microphone-based
  solution in the first place rather than a simple wired trigger.
- **Microphone choice / acoustics.** Cardioid (directional) microphones
  reject off-axis noise much better, but they're significantly more
  expensive. Went with a cheaper, omnidirectional I2S MEMS microphone
  instead, which means the detection logic has to do more work to reject
  everything that isn't the shot.
- **Filtering background noise.** Crowds, wind, PA systems, other races
  running nearby — the detector needs to pick a genuine shot out of all of
  that, reliably, without false triggers.
- **Validation on a PC/laptop.** It's hard to just *look* at the device and
  know whether signal levels, thresholds, and filtering are actually
  correct. Getting real sample data off the device to inspect on a laptop
  meant reusing another interesting side project rather than building that
  tooling from scratch.
- **A visible-enough LED display.** Not as obvious as it sounds — an LED
  that looks bright indoors can be nearly invisible in direct outdoor
  sunlight, which is exactly the lighting condition most races happen in.
- **Physical packaging.** The enclosure itself wasn't the hard part —
  finding a good stand/mount for the LED and the right LEDs to put in it
  took more iteration than expected.
- **Weatherproofing.** Open question: will it work in the rain? The current
  prototype isn't well sealed against moisture, so as it stands today, I
  would not use it in the rain.

## Open questions

- Will the end consumer — the runner it's actually for — be happy with it?
  The first prototype's bits-and-pieces electronics didn't look very
  appealing, and it wasn't obvious how to use it either. Both of those were
  addressed in the improved prototype, but "we fixed the two problems we
  noticed" isn't the same as "it's confirmed to work well for the people
  using it."
- Will it be met with satisfaction and approval — or is there something else
  about it that still needs to change?
- The detection-to-LED delay still needs to be measured. A device that's
  visually late is arguably worse than no device at all for something as
  time-critical as a race start.

## Conclusion

This still needs real-world validation — with actual runners, at an actual
race, in actual outdoor light. And even with AI assistance available
throughout, a lot of the work here simply had to be done by hand: choosing
and testing a microphone, tuning detection against real background noise,
figuring out an LED that's visible in sunlight, and building and packaging
the physical enclosure. AI doesn't get any of the construction done — it
still comes down to building and testing the thing itself.
