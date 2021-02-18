# arduino-projects
Arduino stuff I want to make public.

## Pocket RNG

I'm frequently indecisive and also a bit of a lover of chaos, so I like the idea of making as many choices as possible at random (I could go on a long rant about simulated annealing as a life philosophy, but that's a topic for another time). Unfortunately, I know that the human mind is incapable of anything resembling true randomness, it isn't always convenient to find a surface on which to roll dice, and I do enough work with software to not trust any random number generation phone apps (or phone apps in general, really).

However, I also do enough work with cryptographic hardware to be vaguely aware of the available TRNG offerings, and just enough hobbyist electronics tinkering to be able to work with them. So, after far too many software and soldering errors, I now have a pocket-sized device capable of answering any questions you might encounter in life with true hardware-derived randomness (some translation from die rolls or hex blobs into words may be required).

Right now it'll let you roll dice with 4, 6, 8, 10, 12, 20, or 100 sides, or flip a coin. In addition, when you first open it up, it'll display 40 bytes of random hex (as that's the most that'll fit on the screen). It's powered by a 3V coin-cell battery, but as the power use is minimal and it has a cutoff switch to automatically turn it off if the lid is closed, that should last it quite a while (and it's replaceable when it does eventually run out).

The underlying cryptographic module generates randomness as bytes, not specific integers, so there's a translation layer in the software. For the dice that aren't powers of 2, this means there's a conundrum: the translation layer can either be slightly unbalanced, or nondeterministic (that is, not guaranteed to finish in a finite amount of time). I went with nondeterministic because the odds of it taking longer than the unbalanced approach are minimal[1]. It's still orders of magnitude faster than the tiny animation the device displays before showing the random number, which I added because it just feels a bit weird when it returns instantly as you press the button.

So far it's only had a hand in deciding dinner plans, but I haven't done anything all that interesting the past few days so it hasn't had any real chances to shine.

[1] If you're generating a number between 1 and n, the odds of it needing to re-roll with my current code are (2^24 mod n)/(2^24), which maxes out at approximately 0.0001% for a d20 or d100. And even if it does take two or three iterations, that's still an imperceptibly small amount of time. (And if I really wanted to optimize it even further, I could, but I got lazy and didn't want to have to think about int overflow errors.)

### Code and required libraries:

* Core code: [pocket_rng/pocket_rng.ino](pocket_rng/pocket_rng.ino)
* Library: [Adafruit_GFX.h](https://github.com/adafruit/Adafruit-GFX-Library)
* Library: [Adafruit_SSD1306.h](https://github.com/adafruit/Adafruit_SSD1306)
* Library: [OPTIGATrustM.h](https://github.com/Infineon/arduino-optiga-trust-m)

### Random hardware notes:

Initially I prototyped this with a cheaper microcontroller and ran into problems because the Arduino library provided by the crypto hardware manufacturer Infineon was too big to fit into the available flash and RAM. I had to switch to the final one (Seeeduino XIAO, which is based on the SAMD21) to get it to work. I could've forked the library and made a cut-down version that just includes the RNG bits, but given the time that'd take, spending an extra $2 for a beefier microcontroller was definitely more efficient.

The only component that's underneath the board instead of on top and visible is a CR2032 battery holder. I initially had a CR1220/1225 holder soldered on top, but those batteries (although providing the necessary 3 volts) couldn't handle the current requirements of the device for more than a second or two (and the current requirements are incredibly small; it's just that the batteries were made for stuff like motherboard RTCs that require the tiniest imaginable trickle of electrons). I'd prefer to have the battery visible so that everything (apart from the wires) is out in the open, but it makes the board layout really packed and less usable than it currently is.

I wasn't entirely sure what sort of power switch to use; luckily I realized the limit switches used for 3D printers (and probably other stuff) would work perfectly for flipping on the power when the tin is open, without pressing hard enough on the lid to risk popping it open while it's sitting around. I also ended up adding a second sliding switch above the limit switch, so that I can have it open but powered off when I'm taking pictures, disassembling it for fixes, or that sort of thing.

For the initial versions of this, I tried to solder it together using the 22AWG hookup wire I had lying around and kept running into space issues. Eventually switched to using 30AWG wrapping wire and I hhave no idea how I've survived as long as I have without it, given how much easier it makes laying out circuits (although it does have a higher rate of points that appear to be soldered but aren't fully connected, so it takes more careful checks during assembly).

The Adafruit Altoids-tin-sized prototyping PCBs are great, but unfortunately they're such a perfect fit for the tins that they don't fit inside if you have stuff underneath it that gets in the way of tilting and wiggling it in. As I wanted this on top of standoffs to keep it even-ish with the top of the case, I had to use a slightly larger mint tin I happened to have lying around instead of an actual Altoids tin (I also could've trimmed down the edges of the PCB and gotten it to fit in a traditional tin, but that would've taken longer and looked even jankier than it already does).

The PCB is held in place by 4 standoffs that are bolted to a cardboard+duct tape plate wedged into the base of the case. This keeps it from rattling around, while still keeping it easy to disassemble. My initial attempts used a 3D-printed insert for the tin instead; it ends up looking prettier, but not enough to justify the extra work required, and I kinda like the aesthetic of having as much of the guts visible as possible.

### Component list:

Here's all the stuff required to make your own. All the links are non-affiliate and all the Amazon links are the specific units I bought, so I can verify there's at least a chance of them not being complete garbage. However, there are probably cheaper options available for all of them if you're cool with slightly worse reviews, lower quantities, or more time spent searching. I usually have to go through a few iterations when building stuff so I always buy random components in bulk.

#### Brand-specific stuff:

* [Adafruit Infineon Trust M Breakout Board](https://www.adafruit.com/product/4351), which is a breakout board for the [Optiga Trust M](https://www.infineon.com/cms/en/product/security-smart-card-solutions/optiga-embedded-security-solutions/optiga-trust/optiga-trust-m-sls32aia/)
* [Seeeduino XIAO microcontroller](https://www.seeedstudio.com/Seeeduino-XIAO-Arduino-Microcontroller-SAMD21-Cortex-M0+-p-4426.html) (I purchased them on [Amazon](https://smile.amazon.com/gp/product/B08745JBRP) initially as I didn't realize they were half the price on the Seeeduino site; I expect the Amazon shipping is faster but unless you're in a massive hurry you should order directly from the manufacturer.)
* [Adafruit Perma-Proto Mint Tin Size Breadboard PCB](https://www.adafruit.com/product/723)

#### Generic stuff:

* [CR2032 batteries](https://smile.amazon.com/gp/product/B004AT21R2)
* [CR2032 battery case](https://smile.amazon.com/gp/product/B07BXDHT4B)
* [Standoffs](https://smile.amazon.com/gp/product/B07CNF3W7B)
* [Sliding power switch](https://smile.amazon.com/gp/product/B007QAJUUS)
* [Limit switch](https://smile.amazon.com/gp/product/B073TYWX86)
* [Buttons](https://smile.amazon.com/gp/product/B01N76KBC6)
* [Wire](https://smile.amazon.com/gp/product/B01LWI20M0)
* [Screen](https://smile.amazon.com/gp/product/B01MZ7YU8X)
 * Note about those screens: They don't come with pins (at least the ones I received didn't). If you're considering making this you probably have a spare 4-pin strip lying around, so no need to order more. If you don't, [this set](https://smile.amazon.com/gp/product/B07X23LQQF) likely has more than you'll ever need.
* Mint tin. One that's a tiny bit larger than an Altoids tin is easiest, but you can make that work if necessary. (You can also buy blank, empty tins, but unless you're buying a dozen or more it's probably cheaper to just swing by the nearest gas station and grab some Altoids.)

#### Random other stuff you'll need:

* Soldering iron + solder
* Duct tape + cardboard (for the base plate to hold the standoffs and to fit snugly into the mint tin to stop it all from rattling around)
* Hot glue + gun (for attaching the battery case to the back of the PCB, although you could also attach it a number of other ways or use one that solders directly to the PCB)
* Wire strippers (for 30AWG wire, which is smaller than most will handle)
* Flush cutters (for trimming wire ends after soldering)
* X-Acto knife (for random cutting and trimming, like making the holes to bolt the standoffs to the )
* Multimeter (for checking soldered connections to ensure they work)
* Graph paper and pencil (for planning out the exact part placement and wire paths in advance)
