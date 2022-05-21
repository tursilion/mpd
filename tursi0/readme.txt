* Note this must include the 99/4A keyboard tables from >16E0-17FF for
* KSCAN to work!

* This one is all from scratch, although some routines
* are adapted from the existing console GROMs.
* It will not work correctly without the MPD, as it
* counts on the bank switching behaviour in order to
* save some data space!
* (C) Tursi/Mike Brent 2011
* harmlesslion.com
*
* Header >0000
	DATA	>AA03		* version
	DATA	>0000		* unused
	DATA	>0000		* power up (none, we are the powerup anyway)
	DATA	>0000		* program list (none)
	DATA	>0000		* DSR (none - regular GROM0 has CS1/CS2)
	DATA	>0000		* Subroutine (none - regular GROM0 has some display code for CS1)
	DATA	>0000		* Interrupt (none)
	DATA	>0000		*

* VDP registers at >0010 (original start of jump table)
* 16 registers in order to support the 9938 as well -- you
* should read the last 8 first, then the first 8 so that
* it also works on the 9918.
	DATA 	>0000, >0000, >0000, >0000, >0000, >0000, >0000, >0000

* jump table -- as this GROM0 is custom, the only one that we need
* is at >0020, which is the powerup vector. We'll just run our actual
* code from there.
* >0020
	
