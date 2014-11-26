tea-temp
========

A tea temperature measurement and prep machine software. The idea behind this project is to end up with a nice little
enclosure that houses a microcontroller and an LCD display.

This branch reflects a feature change, to an LCD display instead of a 7-segment display. Also, the overall
architecture was changed to an MSP430F5529.

The system uses a good old DS18B20 1-wire temperature probe from Dallas (now Maxim). Sometimes I like 1-wire, and 
sometimes I freaking hate it. It depends on whether or not it works, really.

The problems I was having with the OneWire library seems to be fixed, but more debugging is still necessary.
This branch (msp_lcd_feature) will be merged into msp430 soon.

The nice thing about switching to the TI products is they have debugging built-in on-board, something the Arduino 
makers forgot in a glaring reminder of how amateurish Arduino really is. I mean, seriously, anyone with ANY experience
programming for a microcontroller realizes how invaluable debugging is. You can't do much beyond blinking a freakin'
LED without it. 

This little machine will sense the temperature of the water in the kettle (or mug) and will display this
temperature on the display. I will implement different tea types. The impetus behind this project is the fact that
I love green tea. Green tea, unlike black tea, cannot be steeped at near boiling temperatures. So you have to bring the
water to a boil and then let it cool back down to between 70-85 degrees Celcius. If you're wondering why you don't just
stop the heating on the way UP instead of letting it cool back down, well, clearly you've never made or drank tea.

Genmaicha is the exception to this rule. It's the only green that you steep at higher temperature, but because of this 
it only needs to be steeped for 30-45 seconds. The project will have a menu letting you choose the type of tea you're 
making, and will eventually have a way to add new teas. It will monitor the temperature of the kettle, and instead of
me walking away for 45 minutes and forgetting to go make my tea, once the temperature falls back to the correct point,
it will sound a buzzer to remind me the tea is ready to be made.
