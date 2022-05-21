This is a modified GROM containing Editor/Assembler in the first bank, and both EDIT1 and ASSM1/ASSM2 included as data. Thus this version of Editor/Assembler functions without the need for any diskette files.

NEWEAG.BIN is provided for emulators - it contains the GROMs padded to 8k boundaries.

NEWEA_6K.bin removes the padding, each 6k GROM immediately follows the previous. This is for writing to real GROMs.

Afraid I don't know how to put a GRAM KRACKER header on there... you can let me know?

This code was all written by TI, except my tiny patches to copy GROM data to RAM. Per my distribution license, I must provide the following text from TI:

-----------------------

TI makes no warranty with respect to the programs and is under
no obligation to provide any support or assistance with respect
to the programs. TI is under no obligation to provide upgrades
to the programs.

No liability is accepted on the part of Texas Instruments or
the author with respect to use, copying or distribution of the
programs.

-----------------------

... and I'm not going to either. ;)

-Tursi

* EDIT1 at GROM >8000 (6k)
* ASSM1 at GROM >A000 (6k) and >C000 (2k)
* ASSM2 at GROM >C800 (4k)

EDIT1 -- >01F1
31 17 00 8F	 MOVE >1700 TO @>2000 FROM GROM@>8000
9D 00 80 00

hook for ASSM -- jump at >067F
06 70 30	CALL GROM@>7030
05 66 87	B GROM@>6687  (skip over block)

Note that the tag for ASSM1 (first byte) is changed from >AA to >BB
so the TI doesn't think it's a GROM header. The test is fixed at offset >0654
in EACOMPLT.BIN.

This is modified for the distorter - the ASSM loader pages
the ASSM1 and 2 files through GROM2 during load. We will also
use an 8k page for ASSM1. Unfortunately, ST can't write to
GROM, only MOVE can. So it's:

ASSM1 and 2 -- >1030 (low ram line buffer overwrite)
Note that the addresses will be correctly relocated.

>1030:
		21 00 01  	MOVE >0001 TO GROM@>1F12 FROM GROM@>7056
		1F 12 70 56
>1037:				
		31 20 00 8F MOVE >2000 to @>2000 FROM GROM@>8000
		9D 00 80 00
>103F:		
		21 00 01	MOVE >0001 TO GROM@>1F12 FROM GROM@>7057
		1F 12 70 57
>1046:
		31 0F FC 8F MOVE >0FFC to @>A000 FROM GROM@>8000
		1D 00 80 00
>104E:		
		21 00 01	MOVE >0001 TO GROM@>1F12 FROM GROM@>7058
		1F 12 70 58
>1055:		
		00			RTN
>1056:		
		0C 0C 0A	DATA >0B,>0C,>0A


001 G R V C N
001 0 0 0 0 1 = 21

-----------------------

There is a ridiculously hacky block of code here:

      CZ    @>8304
      BS    G@G6272                 to load
*     BR    G@G6649                 to err (68BE) <-i
*                                                  |
G625B TEXT  'FILE NAME?'            ! FI is also br instr

The text "FILE NAME" includes part of the code. The problem is that the branch target, >6649, is WRONG. It must be >6648! The string 'FILE NAME' occurs elsewhere in FMT instructions, and this one in particular is only used by MOVEs, so we can fix this and just reuse one of the other strings:

				CZ    @>8304
				BS    G@G6272               to load
025b: 46 48     BR    G@G6648               to err
625D 			TEXT  'LE NAME?'			unused now

>06A5 has a good replacement. Fix up the lookups:

G623B:	31 00 0a b0	MOVE  >000A,G@G66a5,V*>8320   'file name?'
		20 66 a5
G657B:	31 00 0a b0 MOVE  >000A,G@G66a5,V*>8320   file name?
		20 66 a5

Several errors use explicit address tests to confirm behaviour, so we have to patch those tests:

G69EE DCEQ  G6A7A,@>834A            io error
      BR    G@G69F8
      ST    @>834E,V@>02D2          display err #
G69F8 DCEQ  G6AAB,@>834A            assembly error

Change the two DCEQ values to base at >2000 instead of >6000

The function that loads the E/A utilities needs a patch:

6B9C:BF 4C 70 00  DST   G7000,@>834C            --------------
to
6B9C:BF 4C 30 00  DST   G3000,@>834C            --------------      

Relocated to GROM1 with my 'GromRelocate' tool.