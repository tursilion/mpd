This is the original GROM except that the selection menu starts with 0 instead of 1, so that the MPD Config option is always available without interfering with the default '1' for TI BASIC. Just a user convenience thing, that.

The hack:

Display initialization here:

024F : ST @>8358,>30 Number
>30 changed to >2F so first value is 0.

Next the input detection:
02d6 : SUB @>8375,>31 Integer from ASCII
>31 changed to >30 to allow 0 as input

The TI BASIC GROMs are identical to the 99/4A TI BASIC, so they are used unchanged.

------------------

This is a fix being made to all GROM0s to make sure the VDP registers come up correctly
on the 9938 and 9958.

The patch function is in unused space at >1FD0 in config space:

39 00 08 08 19 88	MOVE >0008 to REG>08 FROM GROM@>1FC8
39 00 08 00 19 80	MOVE >0008 to REG>00 FROM GROM@>1FC0
00					RTN

Next, we need to call this, overwriting the MOVE at >006E:
39 00 07 01 04 2A  becomes:
06 1F D0		CALL G@>1FD0	do the move
05 00 74		B G@>0074		essentially a NOP
