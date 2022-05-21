20111104

Very much WIP and incomplete. There is no complete binary to download.

Multiple Personality Distorter is a project I started many years ago now to allow a single stock TI-99/4A console to act like a 99/4, a 99/4A, or a v2.2 99/4A - just by using a software menu and an upgraded version of my uberGROM code.

I got those working, in addition to a completely faked 99/8 menu... and really all that I had left in mind was some sort of GUI interface. Then I started thinking about all kinds of personalities - like Apple 2, and sort of overwhelmed myself.

I would like to finish this project someday, but it's not at the top of the list at the moment. I don't know what 'finish' will mean, but I really should release something, as it mostly works already. ;)

Rough notes follow:

Have a configuration program accessible via menu option (linked from GROM 0). This menu option can be disabled, in which case the BASIC "CALL MPDCONFIG".

ATMega AVRs with 116k or more of flash and 12 or more I/O pins:

128	- 128k flash, 4k EEPROM, 4k SRAM, 53 I/O pins, MLF64
The 1284 is the only one offered in DIP form, all others are in the MLF form factor (the 2560 and 2561 are 256k!)
Might be worth designing for those ones, though, we can do the in-circuit programming?
Or not, at Digikey, the 1284 is half the cost of the 128 (about $8/ea). Big though!!

Or.. some other microcontroller? the STMicro maybe? The code is very simple and easy to port, we just need flash space.
None of these are in DIP form, though.
The STM8S207SB is an 8-bit controller with 128k in a 44 pin package, 5v tolerant, 16MHz clock (about 12MIPs at 16MHz). ($4.09ea)
The STM32F100CB is the 32-bit ARM (Pinky controller) with 128k in a 48 pin package, 3.6v power. ($4.42 ea) Note the biggest ones go up to 1MB flash!
What about the Coldfire based one?

EEPROM can store configuration data, which is basically which bank points to which GROM file. To avoid REVIEW PROGRAM LIBRARY, multiple GROM bases are not used. The following GROM rules are in effect:

NOTE: THE PROGRAMS ALL TEND TO ASSUME THEIR BANKS, SO DON'T REORDER THEM!

GROM 0s are assumed to be 6k in size. The following GROMs are available (Classic99 needs 8k padding):
0		99/4 GROM 0 (patched)
1		99/4A GROM 0
2		v2.2 GROM 0
3		99/8 GROM 0 (faked)
4		Tursi GUI GROM 0 (must stop before >16E0 due to keyboard mapping)

GROMs 1 and 2 are assumed to be 8k in size. Again, they don't need to be padded except for Classic99:
5		99/4 TI BASIC 1
6		99/4 TI BASIC 2
7		99/4A TI BASIC 1
8		99/4A TI BASIC 2
9		E/A Complete 1 (program)
10		E/A Complete 2 (EDIT1)
11		E/A Complete 3 (ASSM1)
12		E/A Complete 4 (ASSM2) (and part of ASSM1, since it's 6k)
13		Disk Manager 1000 1
14		Disk Manager 1000 2
15		ARC303G 1

Configuration is a special GROM that sits on top of GROM0 at >1800. It's only 2k in size and is always mapped there. >1FFE->1FFF is reserved for a 1KHZ 16-bit timer.
16		Configuration

EEPROM stores configuration data and user data. The first few bytes are reserved for settings. When mapped, you can change the data in there. You can map it only to GROM bank 2 and it's only 2k.
17		EEPROM

Finally, it's helpful to expose some RAM to make some operations simpler. We expose a full 8k of SRAM in the GROM space.
Note if this bank is mapped to bank 0 (temporary map only, never to flash!), that only the first 6k is available.
18		SRAM (8k)

The EEPROM stores this permanent data:
Byte 0 - index of GROM 0
Byte 1 - inverse of byte 0 (to ensure valid)
Byte 2 - index of GROM 1
Byte 3 - inverse of byte 2 (to ensure valid)
Byte 4 - index of GROM 2
Byte 5 - inverse of byte 4 (to ensure valid)

The rest of the EEPROM is available for saving data in my custom BIOS.

The above configuration is only read at startup and reset. However, you can temporarily change pages, too, by writing to GROM0 space (these changes are not saved!):
Write to >1F10 - Temporarily change GROM 0 index
Write to >1F11 - Temporarily change GROM 1 index
Write to >1F12 - Temporarily change GROM 2 index

There is a little more magic needed for reads:
-When the GROM address loaded is >0020, reload the saved GROM settings - for reset
-When the GROM address is >16E0 through >17FF, always return data from 99/4A GROM 0 (don't change page) - for keyboard
-When the GROM address is >1800 through >1FFF, return from the configuration block (except for the one exception below)
-When the GROM address is >0006/0007, return the address of the config tool
-When the GROM address is >000A/000B, always return the config subroutine header instead
-When the GROM address is the subroutine header 1st/2nd bytes, return the actual data at >000A/>000B (links to real sub)
-When the GROM address is the program header 1st/2nd bytes, return the actual data at >0006/>0007
-When the GROM address is >2006/>2007, and the 99/8 GROM0 is loaded, return the address of the Set Speed program header.

The following hacks need to be accomplished:
-My little GUI shell written in GPL - this should map in the 99/4A GROMs (all three) when running a program
-99/4 GROMs hacked to work with 99/4A Keyboard startup - done!
-99/4 GROMs hacked to not reset at >004A (load lowercase vector) - done!
-All GROM0s hacked to start counting at 0, so the config can always be up - done!
-Disk Manager 1000 converted to GROM1 loadable program - done!
-E/A Complete modified to load EDIT/ASSM files by paging them through GROM2 -DONE!
-ARC303G converted to GROM1 loadable program - DONE!
-Configuration program written in GPL - done!

Classic99 INI WIP:

[usercart16]
name="MPD"
; We always load the real 99/4A ROM
rom0=C|0000|2000|C:\WORK\classic99\MODS\994AROM.BIN

; but all the GROMs go into the M block

; load 6k of the patched 99/4 GROM 0
rom1=M|0000|1800|C:\WORK\TI\distorter\994\con4g0_mod_A.grm

; Load just the 99/4A GROM 0
rom2=M|2000|1800|C:\WORK\TI\distorter\994a\994AGROM0.BIN

; load the v2.2 GROM 0
rom3=M|4000|1800|C:\WORK\TI\distorter\v2.2\CON22G0.GRM

; Load the 99/8 faked GROM0
rom4=M|6000|1800|C:\WORK\TI\distorter\998\998FakeGROM0.BIN

; Tursi GROM0 goes at >8000

; 99/4 BASIC
rom5=M|A000|1800|C:\WORK\TI\distorter\994\Con4g1.bin
rom6=M|C000|1800|C:\WORK\TI\distorter\994\Con4g2.bin

; 99/4A BASIC (2.2 TI BASIC is identical to 99/4A TI BASIC)
rom7=M|E000|1800|C:\WORK\TI\distorter\v2.2\con22g1.grm
rom8=M|10000|1800|C:\WORK\TI\distorter\v2.2\con22g2.grm

; E/A complete 
rom9=M|12000|1800|C:\WORK\TI\distorter\EAComplete\EACOMPLT-MPG_G1.bin
rom10=M|14000|1800|C:\WORK\TI\distorter\EAComplete\EDIT1
rom11=M|16000|1FFA|C:\WORK\TI\distorter\EAComplete\ASSM1Patched
rom12=M|18000|1000|C:\WORK\TI\distorter\EAComplete\ASSM2

; Disk Manager 1000 (note: two full 8k GROMs!)
rom13=M|1A000|4000|C:\WORK\TI\distorter\Dm1000\DM1000_1G.BIN
;       1C000 used too

; Archiver 3.03 (joined with DM1000)
rom14=M|1E000|1FFA|C:\WORK\TI\distorter\Dm1000\ARC303G

; Config loaded at >20000 - this area is only 2k in size! (0x0800)
rom15=M|20000|0080|C:\WORK\TI\distorter\Config\con4g0_mod_B.grm
rom16=M|20080|0080|C:\WORK\TI\distorter\Config\con8setspeed.grm
rom17=M|207e2|001B|C:\WORK\TI\distorter\Config\cfgtables.bin
rom18=M|20100|0400|C:\WORK\TI\distorter\Config\config_grom_1900.bin

; EEPROM goes at >22000 (not loaded here due to lack of NVRAM in Classic99)

; a real cartridge to test
;rom19=C|6000|2000|C:\WORK\classic99\MODS\POLEPOSC.BIN
;rom20=X|6000|2000|C:\WORK\classic99\MODS\POLEPOSD.BIN

----------------------------------------

Compatiblity Notes:

The 99/4A GROM is the basis for the system and it is assumed that this is installed
into a 99/4A (non-v2.2) console. No other configuration has been tested or supported.

The 99/4A mode is thus expected to be 100% compatible. It is only slightly modified
in order to support loading the configuration menu as option #0.

The 99/4A v2.2 mode is likely minimally modified. It has an identical version of
TI BASIC and in the original machine, the ROM is also identical. Thus the v2.2 GROM
should work 100%.

The 99/4 GROM0 was not designed to operate with the 99/4A ROM, but only a handful of
patches were necessary to make it load. However, the top of GROM memory is replaced
with the 99/4A keyboard table -- this is necessary because the ROM directly reads
into GROM to find the keyboard lookup data. This means, in particular, that the
99/4A keyboard will behave in 99/4A mode, even though you are running a 99/4 O/S.
In addition, the console interrupt routine is the 99/4A version, meaning that you
still get features like user-interrupt hooks that are not available in the true
99/4. Finally, applications like Extended BASIC that use the keyboard scan to
detect the machine type may detect it as a 99/4A because of the keyboard behaviour.
The primary difference is the availability of the lowercase character set load
function in GROM. In order to prevent crashes due to false detect, the GROM has
been modified to return to caller if the load lowercase function at >004A is called
from GROM. No character set is loaded, but the program will continue to execute.
This is not the same as a real 99/4, which will appear to reboot.

The 99/8 mode is completely false. This is really the 99/4A GROM0 modified to look
like the 99/8, and with the 99/8 character set loaded. This means you will get
the 99/8 menus, and the true lowercase characters of the 99/8 font, but otherwise
it is still running as a 99/4A. This also means that 99/8 mode will support ROM-
based cartridges, which probably would not have been true on the real 99/8 (GROM
only, like the version 2.2).

The Tursi Advanced Boot (or whatever I call it) is completely new, self-contained
code, however, when launching external software, it will activate the 99/4A GROMs,
including TI BASIC, and so should maintain compatibility with existing software.

Replacing TI BASIC with the other options will generally work well, but be aware
that some programs which call into GPL may expect the TI BASIC GROMs to be present.
If they are not, those programs may malfunction or crash. In that case, just
enable the TI BASIC GROMs before launching.
