This is the offending GPL code that makes the 99/4 GROM0 hang with the 99/4A ROM:

G0106  03		SCAN				This function wants to scan all keyboard modes from 0-5
       90 74	INC   @KBNO			But fails when the 4A ROM is active, because the 4A SCAN
       d6 74 06	CEQ   >06,@KBNO		zeros the keyboard mode when it's set to 5. In the 4A,
       41 06	BR    G0106			This loop uses >837E to hold the actual keyboard mode count.

I think this loop may be unnecessary. It seems to works well enough to scan only mode 5.
G0106  be 75 05	ST 	 >05,@>8375		Set mode to 5
	   03		SCAN				Read keyboard
	   87 7e	DCLR  @VROW			Padding
	   87 7e	DCLR  @VROW			Padding
       

When running in 99/4 mode only (99/4A and v2.2 ROM is identical), we need to bank in the 99/4A GROMs during SCAN. SCAN needs the 99/4A GROM any time an address from 16E0-17FF is accessed. So,
from >16E0 to >17FF is a copy of the 99/4A GROM0.

Unfortunately, this overwrites the Cassette GROM text table in the 99/4 GROM.

We can move just the text table. The pointers start at >15A0 in the 4A and >1616 in the 4. I need to end at >16DF. Currently ends at >1752. We can hack this table and make an 8k GROM hold the data, since all the lookup data is here in GROM.

Testing in Classic99 seems to work. However, it has a true 99/4A keyboard, so reset is FCTN-=, not Shift-Q, etc.

So, the 99/4 GROM0 needs to be modified as such:
The first message we need to move is "THEN PRESS ENTER". No need to move the other ones. That's:

OLD:
1628 : DATA >16D9 = '>10,>00,:THEN PRESS ENTER:'
162A : DATA >16EB = '>0F,>FE,:PRESS R TO READ:'
162C : DATA >16FC = '>0F,>00,:PRESS E TO EXIT:'
162E : DATA >170D = '>07,>00,:DATA OK:'
1630 : DATA >1716 = '>15,>00,:ERROR - NO DATA FOUND:'
1632 : DATA >172D = '>10,>00,:PRESS C TO CHECK:'
1634 : DATA >173F = '>11,>00,:PRESS R TO RECORD:'


NEW:
1628: 1800 1812 1823 1834 183d 1854 1866

1800 : '>10,>00,:THEN PRESS ENTER:'
1812 : '>0F,>FE,:PRESS R TO READ:'
1823 : '>0F,>00,:PRESS E TO EXIT:'
1834 : '>07,>00,:DATA OK:'
183D : '>15,>00,:ERROR - NO DATA FOUND:'
1854 : '>10,>00,:PRESS C TO CHECK:'
1866 : '>11,>00,:PRESS R TO RECORD:'
1879 :

Replace from >16E0 to >17FF with a copy of the 99/4A GROM0's same address range.

Because we can build 8k GROMs, we can install extra programs (might as well use the 99/4 GROM 0 since we are already growing it). The extra programs can be used to configure the system. :)

The A is the main GROM file that loads at >0000. It requires the B file which is the remaining 128 bytes and loads at >1800 as part of config.

This is the original GROM except that the selection menu starts with 0 instead of 1, so that the MPD Config option is always available without interfering with the default '1' for TI BASIC. Just a user convenience thing, that.

We also hack the boot menu counting so that config lives at #0 instead of #1:

Display initialization here:

0241 : ST @>8358,>30 Number
>30 changed to >2F so first value is 0.

Next the input detection. This is quite different from the 99/4A versions and requires a few extra patches. It appears to scan all keyboard modes, so the letters on the keyboard can select as well as the numbers:

       CEQ   >FF,@KEY  		Any key pressed?
       BS    G02D0			If not, try next kscan mode
       INC   @>8359			?
G02BF  CGE   >31,@KEY		Is the keypress '1' or higher?
       BR    G02C9			If no, branch
       SUB   >31,@KEY		Yes, subtract '1' to get an index
       B     G02CB			skip over
G02C9  DEC   @KEY			decrement the value to get an index
G02CB  CHE   @>836C,@KEY	Is the index greater than the highest value?
       BR    G02E1			branch if valid
G02D0  INC   @KBNO			try next key mode

Oddly, this means that the number keys, or the letters can select programs. This may have worked better on the 99/4 keyboard (but doesn't seem to in emulation!). To be safe, we will disable the split key reads and take only ASCII values. We need to change both '1' values to zero, and change the first branch to go to G02D0 (try next key) instead of decrementing and using the value.

G02BF  CGE   >30,@KEY		Is the keypress '0' or higher?
       BR    G02D0			If no, branch
       SUB   >31,@KEY		Yes, subtract '0' to get an index

Worth noting -- TI BASIC from the 4A is not compatible with the 4 because of the load lowercase letters jump, which causes a reboot as it's the entry point for the 4's GROM! Some other programs are also guilty of this (like the RAG Disassembler).

In order to boost compatibility, we can drop a RTN statement there (still no character set, but no crash either). It does mean we need to change the boot code, though, and so apps may work here but not on a real 99/4. I guess it would be obvious, a function that needs this probably uses lowercase and won't see any.

The only early statement that is relatively optional is the one that mutes voice 0 on the sound chip (since we are about to beep anyway). So we remove that to make room. Original code is:

G004A  DST	 >8C02,@>83FE
       DST   >0100,@>83FC
       DCLR  @>83CE
       ST    >70,@>9400
       ST    >9F,@>8400
       
We need to change it to:
G004A  RTN  					* load lowercase character set - not available
       DATA 00, 00, 00          * padding
G004E  DST	 >8C02,@>83FE		* entry point
       DST   >0100,@>83FC
       DCLR  @>83CE
       ST    >70,@>9400
*      ST    >9F,@>8400   		* deleted (BE 81 00 9F)

Also change the boot vector at >0020 from 40,4A to 40,4E

------------------------------------

This is a fix being made to all GROM0s to make sure the VDP registers come up correctly
on the 9938 and 9958.

The patch function is in unused space at >1FD0 in config space:

39 00 08 08 19 88	MOVE >0008 to REG>08 FROM GROM@>1FC8
39 00 08 00 19 80	MOVE >0008 to REG>00 FROM GROM@>1FC0
00					RTN

Next, we need to call this, overwriting the MOVE at >0070:
39 00 07 01 04 02  becomes:
06 1F D0		CALL G@>1FD0	do the move
05 00 76		B G@>0076		essentially a NOP
