This is just the 99/4A GROM0 patched further to RESEMBLE the 99/8. The 99/8 GROM accesses hardware in a different manner and doesn't run on the 99/4A (at least as deep as I looked into it).

1) The character set (95 characters at 7 bytes each) is replaced at >6B4. The 99/8 GROM 3.0 defines 96 characters, but the last character is blank just like in the 99/4A, so no practical difference.

2) The "(C)1981  TEXAS INSTRUMENTS" line at >48F is changed to "(C)1983  TEXAS INSTRUMENTS  3.0", which takes 5 more characters and overwrites "HOME " from the next string.

3) These two MOVEs are updated to reflect the new strings:

016F : MOVE >0018 TO VDP@>02C4 FROM GROM@>048F c Texas Instruments 
	--> 0018 to 001D, 02C4 to 02C2
	
0176 : MOVE >000D TO VDP@>016A FROM GROM@>04A7 Home Computer
    --> 000D to 0008, 016A to 016C, 04A7 to 04AC
    
A few moves on the selection screen need to be updated:
       
The 99/8 loads the small character set before the selection screen, so this needs to be patched in.
0263 : MOVE >0011 TO VDP@>0028 FROM GROM@>0496 Texas Instruments
    --> 0028 to 0029

026A : MOVE >000D TO VDP@>0068 FROM GROM@>04A7 Home Computer
	--> 000D to 0008, 0068 to 006D, 04A7 to 04AC

The format for 'PRESS' needs to be changed slightly, change the >9E at >25B to >9C.

We need some code space to both load the new character set, and to change the numbers
1,2,3,etc, to formatted letters [A],[B],[C] etc. To do this, we are going to replace
the INSERT CARTRIDGE handling, as that will never be needed with this system.

This is the original code for insert cartridge:
0278 : CZ @>836C 		No program?
027A : BR GROM@>0293 	Jump, if program
027C : FMT 				No program, then message: Insert cartridge
027D : ... XPT=>02
027F : ... ':INSERT CARTRIDGE:'
0290 : ... END FMT
0291 : BR GROM@>02EF
0293 : INC @>8358
0295 : ST VDP*>8352,@>8358 Write number
---- this is where we want to resume
0299 : DINCT @>8352

Patch the starting position:
0272 : DST @>8352,>00E4 Address screen
  -->00E4 to 00E2

Patch the starting character:
0276 : ST @>8358,>30 Number
  --> >30 (which is >2F patched in our 99/4A) to >3F - config will be '@'

Here is the replacement code:
0278 : BF 4A 09 00	DST @>834A,>0900 	Capital letters
027C : 06 00 18		CALL G@>0018		Load small lowercase
027F : 90 58		INC @>8358			next index
0281 : BE B0 52 5B	ST VDP*>8352,>5B	'['
0285 : 91 52		DINC @>8352			Next VDP char
0287 : BC B0 52 58 	ST VDP*>8352,@>8358 Write number
028B : 91 52		DINC @>8352			Next VDP char
028D : BE B0 52 5D	ST VDP*>8352,>5D	']'
0291 : 42 99		BR G@>0299
0293 : 42 99		BR G@>0299
0295 : 42 99		BR G@>0299
0297 : 42 99		BR G@>0299

The 99/8 single-spaces the program entries:
02E0 : DADD @>8352,>003A New VDP address
  --> change the >003A to >001A

And change the loop address:
02E7 : BS GROM@>0293 No, next program
  --> change to 027F

Next the input detection:
02FD : SUB @>8375,>31 Integer from ASCII
  --> >31 changed to >40 to allow @ as input
  
Even though 'SET SPEED' didn't do anything on the WIP 99/8, it was part of
the experience. Since it didn't do much, we can reproduce it rather easily.
It's difficult to embed it between the built-in BASIC and the GROM carts,
especially because we are using the 99/4A BASIC and so can't hack that header,
so we'll just hack it in as the first value (it will show up after MPD config).

GROM0 is pretty darn full, though, so we store this code in the configuration
bank. We have plenty of room there, even though it's only 2k. It will only show
when it's actually linked, and only the 99/8 GROM will link it.

This address is patched into the header at offset >0006 - enter >18E0

>1880:
31 02 00  		MOVE >0200 TO VDP@>0900 FROM GROM@>04B4		large letters again
a9 00 04 b4

08				FMT									Show screen
A1					ROW+ 2
83					COL+ 4
08 SET SPEED		'SET SPEED'
A1					ROW+ 2
96					COL+ >17
07 [A] SLOW			'[A] SLOW'
A0					ROW+ 1
97					COL+ >18
18 [B] 99/4A...		'[B] 99/4A SPEED FOR GAMES'
A0					ROW+ 1
86					COL+ 7
0D [C] FULL SPEED	'[C] FULL SPEED'
FB				ENDFMT

>18CD: 
03				SCAN				read keyboard
D6 75 41		CEQ @>8375,>41		pressed A?
78 DD 			BS G@>18DD			jump ahead
D6 75 42 		CEQ @>8375,>42		pressed B?
78 DD 			BS G@>18DD			jump ahead
D6 75 43 		CEQ @>8375,>43		pressed C?
58 CD 			BR G@>18CD			no, so loop

>18DD:
8F 80 D0		DCLR @>83D0		make the console search titles again
41 B4 			BR G@>01B4		JUMP BACK TO ACCEPT/CLEAR/DISPLAY
  
>18E2: (header)
21 4D			point back to TI BASIC GROM (not 4)
18 80
09 SET SPEED

====================================================================

This section deals with scanning multiple GROM bases. 

Original code:

12A1 : DATA >045B
12A3 : DATA >01
12A4 : DATA >B1
12A5 : DATA >15
12A6 : TEXT ':REVIEW MODULE LIBRARY:'

01B6 : ST @>8372,>FE 				Data stack >FE
01B9 : ST @>836D,>06 				Program LINK
01BC : CLR @>836C

01BE : CLR @>83FB					clear GROM base pointer 
01C1 : MOVE >001E TO VDP@>0400 FROM GROM@>6000	copy from base 0
01C8 : ST @>83FB,>04 GRMRD +4
01CC : MOVE >001E TO VDP@>0420 FROM GROM@>6000	copy from base 1

* this loop compares all bytes that were copied to see if they are
* the same. It doesn't check valid, that just makes it put
* review module library up if they are not
01D3 : CLR @>8358
01D5 : CLR @>8359
01D7 : B GROM@>01DC
01DA : INC @>8359
01DC : CGT @>8359,>1D
01DF : BS GROM@>01ED
01E1 : CEQ VDP@>0400(@>8358),VDP@>0420(@>8358) Are GROM's parallel?
01E8 : BR GROM@>01EF
01EA : B GROM@>01DA
01ED : BR GROM@>01FD

* we come here if they are different
01EF : INCT @>8372
01F1 : DCLR *>8372
01F4 : INCT @>8372
01F6 : DST *>8372,>12A1 				This is the link address to Review Module library (see above)
01FB : INC @>836C 						1st program

* this checks for ROM headers (before GROM headers!)
01FD : CEQ @>6000,>AA 					ROM Header? OH!!! / Not at GROM V2.2
0202 : BR GROM@>0224 					/
0204 : DST @>8358,@>6006 				Next program start / The other shifted
0209 : DCZ @>8358 						/ accordingly
020B : BS GROM@>0224 					/
020D : INCT @>8372 						/
020F : DST *>8372,>FFFF 				Flag on stack / This change eliminates
0214 : INCT @>8372 						/ the ROM moduls
0216 : DST *>8372,@>8358 				Address on stack /
021A : INC @>836C 2nd program? 			/
021C : DST @>8358,@>0000(@>8358) 		/
0222 : BR GROM@>0209 					/

* this uses XLM >1A to search the GROMs in the current base
0224 : INCT @>8372 						Increase stack
0226 : DCLR *>8372 						Search program in GROM
0229 : XML >1A 							GSRLNK
022B : BS GROM@>0224 					Loop till all

022D : DECT @>8372							fix up stack
022F : DCEQ @>8302,>12A1					is the first entry >12A1 (ie: review module library?)
0233 : BR GROM@>0240						If no, skip ahead, we're done
0235 : MOVE >0001 TO @>8359 FROM GROM@>6000 If yes, check that current GROM is valid
023B : CEQ @>8359,>AA						If not, 
023E : BR GROM@>01B6 						Reset back to first GROM base
0240 : MOVE >0001 TO REG>01 FROM GROM@>044E Screen enable

----------------------------------------------

The trick in in the XML >1A - it increments the GROM base address each time it
is called, including wrapping around after 16 bases. The base is stored at >83D0,
zero this value to restart the search.

We just need to do three things. First, in the main search we should flag whether we
detected multiple bases, and if we did, we should cycle all bases when building the list.

We need to store the base somewhere we can get at it, too, luckily, the list stores
the link address, which is never used.

To do the first, we will patch this code:

01E2: BS GROM@>01F0   <-- change to >01FB   (this was a jump to a jump)

and patch this one:

01EB : BR GROM@>01F2  <-- change to >01F0

* 1F0 was an unnecessary jump from 1E2! We skip it now.
01F0 : BR GROM@>0200 
* we come here if they are different
01F2 : INCT @>8372
01F4 : DCLR *>8372
01F7 : INCT @>8372
01F9 : DST *>8372,>12A1 				This is the link address to Review Module library (see above)
01FE : INC @>836C 						1st program

This whole subroutine is unnecessary now, so we'll do this:
01F0: BE 75 FF			ST @>8375,>FF			use keyboard return as a flag byte (multiple bases)
01F3: 41 FD				BR G@>01FD				skip ahead
01F5: 00 00 00 00 00 00

01FB: 86 75				CLR @>8375				flag - single base
01FD: 87 80 D0			DCLR @>83D0				zero out search space (every time)

Now there is no REVIEW MODULE LIBRARY, so we need to take care of the rest.

After the GROM search, we loop as needed through all the bases. Luckily RML adds some code here we don't need:

* this uses XLM >1A to search the GROMs in the current base
0227 : INCT @>8372 						Increase stack
0229 : DCLR *>8372 						Search program in GROM
022C : XML >1A 							GSRLNK
022E : BS GROM@>0227 					Loop till all

0230 : DECT @>8372							fix up stack
0232 : DCEQ @>8302,>12A1					is the first entry >12A1 (ie: review module library?)
0236 : BR GROM@>0243						If no, skip ahead, we're done
0238 : MOVE >0001 TO @>8359 FROM GROM@>6000 If yes, check that current GROM is valid
023E : CEQ @>8359,>AA						If not, 
0241 : BR GROM@>01B6 						Reset back to first GROM base
0243 :

This saves the GROM base in the old flag word. We also need to change the ROM flag to >0000
(which we patch below). It must also remove any entries from the console GROMs (<6000) when 
the base is not >9800, to avoid duplicates.

-At 22E: Change like so:

022E: 7F 80		BS GROM@>1F80		there's more to come!
0230: 5F A0		BR G@>1FA0			call end of list handling
0232: 00 00 00 00 00 00 00 00
023A: 00 00 00 00 00 00 00 00 
0241: 00 00

* check whether to keep the received row
1F80: 8E 80 FB		CZ @>83FB			skip the valid test on GROM 0
1F83: 7F 92 		BS GROM@>1F92		it's good
1F85: CA 90 72 60	CHE *>8372,>60		test if cart GROM or higher
1F89: 7F 92			BS GROM@>1F92		it's good
1F8B: 96 72			DECT @>8372			drop this one, get the next one
1F8D: 92 6C			DEC @>836C			decrement count, too
1F8F: 05 02 29		B GROM@>0229		get next without increment			

* handles accepting one row then going back for more
1F92: 96 72				DECT @>8372			point to flag word
1F94: BD 90 72 80 FA	DST *>8372,@>83FA	save the GROM base
1F99: 94 72				INCT @>8372			fix the stack pointer
1F9B: CA 6C 10			CHE @>836C,>10		is the screen/stack full?
1F9E: 42 27				BR GROM@>0227		Nope, get the next one with increment

* handles the end of the list
1FA0: 8E 75		CZ @>8375			test for only one GROM base
1FA2: 7F A9		BS G@>1FA9			yes, just one base
1FA4: 8E 80 D1	CZ @>83D1			did the GROM base loop?
1FA7: 42 29		BR G@>0229			nope, go get some more without increment
1FA9: 96 72		DECT @>8372			fix up stack pointer
1FAB: 06 1F 20	CALL G@>1F20		call the stack sort function
1FAE: 05 02 43	B G@>0243			skip ahead
1FB1:

* when multiple GROM bases are involved, the display order gets a bit messed up -
* base 9800 gets pushed below all later bases. Changing the search order broke
* things pretty badly, so we do a little sort function here instead, post-search.
* Sorting by base. The rules are: >0000 first (ROM), then sort in descending order.
* do not change the order of entries with the same base. this will preserve the
* order that we know and love. :) >836A and >835C, 8352, 8358 (byte) are available.
* we also know that the ROMs (0000) will be first, then the GROM bases in reverse
* order, so that helps the sort a little. We can do a little bubble-ish sort.
* It's not super efficient, but it's easy to code even in GPL, and our lists
* are generally very short.

SEARCH
1F20:	BF 5C 00 04		DST @>835C,>0004	* ptr1 - 0, ptr2 - 4

LP1		
1F24:	C8 5D 72		CHE @>835D,@>8372	* are we done?
1F27:	7F 4D			BS G@>DONE			* yes!
		
1F29:	8F 90 5C		DCZ *>835C			* is the lower pointer ROM?
1F2C:	7F 46			BS G@>SKIP			* yes, skip test
		
1F2E:	C5 90 5D 90 5C	DCH *>835D,*>835C	* not high or equal!! that would change ordering
1F33:	5F 46 			BR G@>SKIP			* equal or lower, correct order

* okay, these two are out of sequence.
* swap them, then restart the search
* It's unclear if EX supports a word mode??
1F35:	C1 90 5C 90 5D	DEX *>835C,*>835D	* exchange flags
1F3A:	A3 5C 02 02		DADD @>835C,>0202	* update both pointers at once
1F3E:	C1 90 5C 90 5D	DEX *>835C,*>835D	* exchange addresses
1F43:	05 1F 20		B G@>SEARCH			* restart search

* test the next two elements		
SKIP
1F46:	A3 5C 04 04		DADD @>835C,>0404	* update both pointers in one operation
1F4A:	05 1F 24		B G@>LP1
		
* all sorted!		
DONE	
1F4D:	00				RTN
		

To change the flag for ROMS, we need to make the following patches:

0212 : DST *>8372,>FFFF  <- change the FFFF to 0000

02B8 : CZ @>835C
02BA : BS GROM@>02D0	<- Change the BS (62) to a BR (42)

0325 : CZ @>8378 Check flag
0327 : BS GROM@>0332	<- Change the BS (63) to a BR (43)

036F : CZ @>8378 GROM?
0371 : BS GROM@>0378 	<- Change the BS (63) to a BR (43)

To make the program list look right, even though we've already patched it a lot, we need to update the GROM base
before we can read the data out, so:

Original code:
02D0 : MOVE >0001 TO @>835F FROM GROM@>0000(@>836A) Length byte and text
02D7 : DINC @>836A
02D9 : MOVE @>835E TO VDP*>8352 FROM GROM@>0000(@>836A) from GROM

We can replace that nice long MOVE with the GROM base and a subroutine call for smaller patches!
02D0: BD 80 FA 5C			DST @>83FA,@>835C				Set the correct GROM base
02D4: 06 1F 00				CALL >1F00						replaces the MOVE!

1F00: 33 00 01 5F 00 00 6A	MOVE >0001 TO @>835F FROM GROM@>0000(@>836A) 
1F07: 00					RTN

In the launch code, we also need to set the correct GROM base. To be clean, we should set it
in both cases, so that ROM programs that expect >9800 are not confused (although this should
rarely be a problem). Because of this, we'll intercept the startup a little early:

0325 : CZ @>8378 			Check flag
0327 : BR GROM@>0332		(note we changed this to a BR above! Now we're replacing it.)

We'll replace the above again, with a jump to a patch routine:

0325: 05 1F 10		B @>1F10
0328: 00

and the patch routine will be like this:

1F10: 86 80 FB		CLR @>83FB			Reset base to 9800
1F13: 8E 6E			CZ @>836E
1F15: 63 29			BS G@>0329			If zero, then launch as ROM
1F17: BD 80 FA 6E	DST @>83FA,@>836E	Update the right GROM base
1F1B: 05 03 32		B G@>0332			go parse as GROM
1F1E:

Finally, we need both bytes of the flag for that to work, but the current location
only has room for one byte. Since we don't need the stack anymore, we'll move it
to >836E for testing instead of >8378 (this repatches some of the above again):

0315: ST @>8378,*>836C Flag in >8378
changes to:
0315: BD 6E 90 6C 	DST @>836E,*>836C

036F : CZ @>8378  <-- change 8378 to 836E

and prevent 6E from being cleared:

0358 : MOVE >006F TO @>8301 FROM @>8300  <--- change 006F to 006D

The new longer possible listing runs afoul of a little function that checks for GROM
startups. We move that function's temporary space from 8328 to 8340 (it already also
uses 8342):

037C : ST @>8342,>60
037F : MOVE >0002 TO @>8328 FROM GROM@>6000   	<--- change 8328 to 8340
0385 : CEQ @>8328,>AA GROM here?				<--- change 8328 to 8340
0388 : BR GROM@>0392
038A : CGE @>8329,>00 Number <>0?				<--- change 8329 to 8341

-----------------------------------------------------

After all that hard work, there's just one more patch in here, to fix
the register setup so that it will work on 9938 and 9958 VDPs. We write
all 16 registers, but to keep it compatible with the 9918, we do it in
two steps, writing registers 8-15 first, then 0-7 (so that the 9918
ends up with the correct values!)

The data fits nicely in unused space at >1FD0 in config space:

39 00 08 08 1F C8	MOVE >0008 to REG>08 FROM GROM@>1FC8
39 00 08 00 1F C0	MOVE >0008 to REG>00 FROM GROM@>1FC0
00					RTN

Next, we need to call this, overwriting the MOVE at >006E:
39 00 08 00 04 51  becomes:
06 1F D0		CALL G@>1FD0	do the move
05 00 74		B G@>0074		essentially a NOP
