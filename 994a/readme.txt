This is the original GROM except that the selection menu starts with 0 instead of 1, so that the MPD Config option is always available without interfering with the default '1' for TI BASIC. Just a user convenience thing, that.

The hack:

Display initialization here:

0276 : ST @>8358,>30 Number
>30 changed to >2F so first value is 0.

Next the input detection:
02FD : SUB @>8375,>31 Integer from ASCII
>31 changed to >30 to allow 0 as input

------------------

This is a fix being made to all GROM0s to make sure the VDP registers come up correctly
on the 9938 and 9958.

The patch function is in unused space at >1FD0 in config space:

39 00 08 08 1F C8	MOVE >0008 to REG>08 FROM GROM@>1FC8
39 00 08 00 1F C0	MOVE >0008 to REG>00 FROM GROM@>1FC0
00					RTN

Next, we need to call this, overwriting the MOVE at >006E:
39 00 07 01 04 02  becomes:
06 1F D0		CALL G@>1FD0	do the move
05 00 74		B G@>0074		essentially a NOP

------------------

This patch resets the F18A, if present, by resetting the GPU and palette.
THIS IS NOT IN THE ROM YET!!! NEED TO TEST AND ADD IT! OR VICE VERSA!
If the extended registers can not be written by the GROM, then we should
just load assembly to scratchpad and run that.

There's so little space left in the Config block, so this fills a few holes.
It builds on the patch above:

ACTUALLY: This code will not work. It sets the VDP address every time, so it
won't be able to load the palette. Write a small ASM function instead, you
can copy it to >8300 and execute with XML >F0 (example in the Bit Reversal
Routine in TI Intern.)

1FDC:		over write the 00 with:
	58 F0		BR GROM@>18F0
	(empty block at 1FDD becomes 4 bytes starting at 1FDE)

18F0:
	39 00 01 38 1F 7E 	MOVE >0001 TO REG>38 FROM GROM@>1F7E		* Write >00 to stop the GPU (uses RTN byte below)
	39 00 01 18 1F 7E 	MOVE >0001 TO REG>18 FROM GROM@>1F7E		* Write >00 to reset the palette sets
	5F 70				BR GROM@>1F70								* next patch
	(Empty block at 18F0 becomes 2 bytes at 18FE)

1F70:
	31 00 20 AF AF C0 XX XX 	MOVE >0020 TO VDP@>AFC0 FROM GROM@>xxxx		* write the palette using DPM (trick on VDP address) THIS /MIGHT/ WORK!
	39 00 01 2F 1F 7E 			MOVE >0001 TO REG>2F FROM GROM@>1F7E		* write >00 to turn off palette DPM
	00							RTN
	(Empty block at 1F70 becomes 1 byte at 1F7F)

And we need 32-bytes in each GROM0 somewhere to store the new palette data.
This will probably differ by GROM:
	DATA >0000,>0000,>02C3,>05D6,>054F,>076F,>0D54,>04EF
	DATA >0F54,>0F76,>0DC3,>0ED6,>02B2,>0C5C,>0CCC,>0FFF

TI Intern can search for "empty space till" to find spots for 99/4A and version 2.2, if it's not used in 99/8 (might be). 
Might also just nuke the stupid large capitals set, but that will break how it looks. We'll find room.

This is the wrong place to put it, but I'll read this. I should change the 99/8 accept and error tones from beeps into bells. :)

