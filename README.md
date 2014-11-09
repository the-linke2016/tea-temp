tea-temp
========

A tea temperature measurement and prep machine software. The idea behind this project is to end up with a nice little
enclosure that houses a microcontroller and a display of some kind (currently, an 8-digit, common cathode, 7-segment
display panel driven by a MAX7219).

Currently the software is an Arduino sketch. This is because I started prototyping on an Arduino Due (the only Arduino
even worth using, in my opinion). However, I am thinking of porting it over to some TI micro, either an MSP430 (likely
the F5529), a Tiva C (TM4C129XNCZAD or whatever chip is on the smaller LaunchPad), or maybe even the Hercules chip 
I have the LaunchPad for. I'd like to stay 32-bit, but the MSP is 16-bit with some 32-bit multiply instructions, 
so that's not too bad.

The system uses a good old DS18B20 1-wire temperature probe from Dallas (now Maxim). Sometimes I like 1-wire, and 
sometimes I freaking hate it. It depends on whether or not it works, really.

I'm having major problems implementing the 1-wire read function. The DS18B20 is sending the correct data back, but
for some reason the Due isn't reading the level correctly. Have no clue why this is. Timing seems to be right, my logic
analyzer is showing that the 1-wire bus is valid, and the write functions are fine. This all points to some 
weird problem with the Arduino libraries. My other option is to drop the Arduino software like the dirty pile 
of crap that it is, and switch to the AVR Studio... which isn't a whole lot better, but at least I might 
be able to debug. 

The nice thing about switching to the TI products is they have debugging built-in on-board, something the Arduino 
makers forgot in a glaring reminder of how amateurish Arduino really is. I mean, seriously, anyone with ANY experience
programming for a microcontroller realizes how invaluable debugging is. You can't do much beyond blinking a freakin'
LED without it. 

Anyway. This little machine will sense the temperature of the water in the kettle (or mug) and will display this
temperature on the display. I will implement different tea types. The impetus behind this project is the fact that
I love green tea. Green tea, unlike black tea, cannot be steeped at near boiling temperatures. So you have to bring the
water to a boil and then let it cool back down to between 70-85 degrees Celcius. If you're wondering why you don't just
stop the heating on the way UP instead of letting it cool back down, well, clearly you've never made or drank tea.

Genmaicha is the exception to this rule. It's the only green that you steep at higher temperature, but because of this 
it only needs to be steeped for 30-45 seconds. The project will have a menu letting you choose the type of tea you're 
making, and will eventually have a way to add new teas. It will monitor the temperature of the kettle, and instead of
me walking away for 45 minutes and forgetting to go make my tea, once the temperature falls back to the correct point,
it will sound a buzzer to remind me the tea is ready to be made.

Some of the challenges in writing this software, as outlined above, have been writing my own 1-wire implementation.
Yes, I'm aware there's an Arduino 1-wire library, but what's the point of programming if you just use everyone else's
code all the time? Of course, there are times when libraries are important, but for 1-wire, pfft. I wrote my 'driver'
for it in about 15 minutes. And I know for a fact the algorithm works, I'm pretty sure that the timing is being thrown
off by the Arduino library taking it's sweet time to switch the pin from an output back to a high-impedance input. The
interesting thing is that the data looks fine. The Arduino pin is not dragging the line down to ground, or anything.
But it still just reads it as all zeroes, or sometimes a 1 followed by all zeroes. I admit, it's possible my code is 
the problem, but I'm almost sure it's not this time.
