This is an adaptation of Tony Knell's GROM0, which was modified for multiple GROM bases on the menu, lowercase text here and there, and a fix for the 9938/9958 VDPs. However, it's a full 8k GROM which caused some issues here, since for GROM0 I only allow 6k to leave room for the Config tool.

Tony had separate GROMs for the 9918 and the 9938, differing only in how many VDP registers they initialized, and whether a little screensaver application was loaded (80 column only!). The first task is to unify the two GROMs.

The problem comes down to the only MOVE statement that is changed, at startup.

MOVE >0008 to REG>00 FROM GROM@>1980 -- in the 40 column version
MOVE >0010 to REG>00 FROM GROM@>1980 -- in the 80 column version

I presume this is because the later VDP has 16 VDP registers (I could check, but too lazy). The problem is on the 9918, the last 8 registers in the 80-column version (8-15) get masked down to register 0-7 again, overwriting the valid values with invalid ones.

So, I'm going to jump ahead to some unused space, and replace the one MOVE with two MOVEs:

MOVE >0008 to REG>08 FROM GROM@>1988 -- load the 80 column registers
MOVE >0008 to REG>00 FROM GROM@>1980 -- load the 40 column registers

This way, the 40 column device will overwrite the bad register values with the good.

The data fits nicely in unused space at >1FD0 in config space:

39 00 08 08 19 88	MOVE >0008 to REG>08 FROM GROM@>1FC8
39 00 08 00 19 80	MOVE >0008 to REG>00 FROM GROM@>1FC0
00					RTN

Next, we need to call this, overwriting the MOVE at >006E:
39 00 10 00 19 80  becomes:
06 12 C0		CALL G@>1FD0	do the move
05 00 74		B G@>0074		essentially a NOP

------------------------------------------------------------
Now, we need to get the file down to 6k, if we can (alternately, we can store part of it in the config block, as there is still some space there). To do this, we need to understand the code in the additional 2k block.

1800: 00 00 18 18 11 Enable Gram Pages 00 00 -- seemingly unused program linkage?
1818: MOVE >001C to @>A000 FROM G@>1830		 -- load assembly program
1820: ST @>8300,>A000						 -- set start address
1824: XML >F0								 -- execute it

1826 - 1830 - unused
1830 - 184B - assembly program code
184C - 1853 - unused? (zeroed)

*****
1854: >0A 2000  TONY KNERR SOFTWARE >00 >00 (10 count ends after 'TONY') - used on title screen MOVE at >016F

Keyscan subroutine called from >01AF

1870: CALL >03CE		Accept Tone
1873: RAND >FF			Random number
1875: SCAN				Keyboard scan
1876: BR >1873			40 column mode (no key pressed)
	  BR >1884			80 column mode (no key pressed)
1878: ALL >20			Clear screen
187A: MOVE >0300 to VDP@>0900 FROM >1A00	load character set
1881: B G@>1890

* 80 column mode
1884: CEQ @>83D6,>2A	Screensaver time?
1888: BR G@>1873		If no match, loop back to keyboard
188A: B G@>18AA			Otherwise jump to >18AA

188E: DATA >00,>00

* back to 40 column
1890: ST VDP@>0383,>17						set color sets 3-15 to black on cyan
1894: MOVE >000C TO VDP@>0384 FROM VDP@>0383
189B: B G@>01B4								back to normal processing

189E - 18A9 - unused? (zeroed)

18AA: MOVE >02F0 to @>A000 FROM G@>1D00 	Copy screensaver code to RAM
18B2: ST @>8300,>A000						store boot address
18B6: XML >F0								jump to assembly

18B8 - 18CB - unused? (zeroed)

Turns on card at CRU >0F08 (GRAM Access it says?)

18CC: ST @>8372,>0080						???
18D0: MOVE >000C TO @>8300 FROM G@>18E4		Copy asm and link to scratchpad
18D6: XML >F0								execute assembly
18D8: B G@>0193

18DB-18E3 - unused (zeroed)
18E4-18EF - asm program and link for scratchpad:
	DATA >8302
	LI R12,>0F08
	SBO >0000
	NOP
	B *R11
	
18F0: MOVE >0011 TO VDP@>0128 FROM GROM@>0496		TEXAS INSTRUMENTS
18F7: MOVE >0018 TO VDP@>01C4 FROM GROM@>1908		8 BIT RAM ACCESS ENABLED
18FE: B G@>016F

1901-1907 - unused? (zeroed)
1908-1920 - 8 BIT RAM ACCESS ENABLED

1920: RAND >FF			random number
1922: SCAN
1923: BR G@>1920
1925: CEQ @>8375,>20	space?
1928: BR G@>192D		no, branch
192A: B G@>01B4			yes, go change base
192D: CEQ @>8375,>0F	char 15?
1930: BR G@>02FA		no, go parse the key
1932: EXIT				yes, Reset

1933-194F - unused (zeroed)

1950: Press Space bar For Next Page 00 00 00
1970: INSERT CARTRIDGE

1980-198F - VDP REGISTER Settings

1990-19FF - unused (zeroed)

1A00-1CFF - character set

1D00 - 1FF0 - Screensaver code
1FF0 - 1FFF - Unused (except for a small tag that says Tony)

-------------------------------------

This is what we need to keep:

1854: >0A 2000  TONY KNERR SOFTWARE >00 >00 - used on title screen MOVE at >016F
Excepting the >00 bytes, moved to >12CD-12E6

Keyscan subroutine called from >01AF, moved to >12E7

12E7 1870: CALL >03CE		Accept Tone
12EA 1873: RAND >FF			Random number
12EC 1875: SCAN				Keyboard scan
12ED 1876: BR >1873 (12EA)	40 column mode (no key pressed) <--- FORCE TO THIS JUMP
12EF 1878: ALL >20			Clear screen

xxxREPLACE:187A: MOVE >0300 to VDP@>0900 FROM >1A00	load character set 
12F1: DST @>834A,>0400    
12F5: CALL G@>0018		* load uppercase
12F8: DST @>834A,>0600
12FC: CALL G@>004A		* load lowercase
xxxEND REPLACE, then skip some

* back to 40 column  xxxxx RELOCATE down
12FF 1890: ST VDP@>0383,>17						set color sets 3-15 to black on cyan
1203 1894: MOVE >000C TO VDP@>0384 FROM VDP@>0383
130A 189B: B G@>01B4								back to normal processing
130C:

18CC: ST @>8372,>0080						<<<- move this back to 018F (4 bytes)

18F0: MOVE >0011 TO VDP@>0128 FROM GROM@>0496	<<<- move this back to 0168

----the rest of these go into the Config ROM----

---Called from 02F7 (relocate)----
1F80 1920: RAND >FF			random number
1F82 1922: SCAN
1F83 1923: BR G@>1920 (1F80)
1F85 1925: CEQ @>8375,>20	space?
1F88 1928: BR G@>192D (1F8D)no, branch
1F8A 192A: B G@>01B4		yes, go change base
1F8D 192D: CEQ @>8375,>0F	back?
1F90 1930: BR G@>02FA		no, go parse the key
1F92 1932: EXIT				yes, Reset

----MOVE at 025B----
1F93 1950: Press Space bar For Next Page 00 00 00

---MOVE at 027C---
1FB0 1970: INSERT CARTRIDGE

---patched moves at >12C0---
1FC0 1980-198F - VDP REGISTER Settings

---Moving the VDP Register setup here lets us more easily fix it in all GROMs---
39 00 08 08 19 88	MOVE >0008 to REG>08 FROM GROM@>1FC8
39 00 08 00 19 80	MOVE >0008 to REG>00 FROM GROM@>1FC0
00					RTN


-------------------------------------

Finally, replace SNUG with _MPD - so we don't misrepresent SNUG
This is at >04A7


