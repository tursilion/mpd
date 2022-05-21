This contains the 2k of configuration (and extra data for the 99/4 ROM). It loads at >1800
and does not bank, it's always there.

>1800 - 128 bytes for the rest of the 99/4 GROM 0
>1880 - Set Speed program for 99/8 
>18F0 - (16 bytes)
>1900 - Configuration program (reboots on exit)
>1D00 - CALL DIR subprogram
>1F00 - 99/8 patch for GROM base called from >02D9
>1F08 - (8 bytes)
>1F10 - 99/8 patch for GROM base called from >0325
>1F20 - 99/8 patch to sort the cartridge list, called from >1FAB (also a patch)
>1F50 - PAB for CALL DIR pre-padded with periods (long to support hard drives, in theory)
>1F60 - Patch for CALL DIR retry
>1F70 - (16 bytes)
>1F80 - 99/8 patch to process the cartridge list entries, called from >022E
>1FA0 - 99/8 patch to process the end of the cartridge list, called from >0230
>1FB2 - (14 bytes)
>1FC0 - VDP register settings (16 registers for 9938 support, used by all)
>1FD0 - Function to load VDP register settings (used by all)
>1FDD - (5 bytes)
>1FE2 - Patch to program list header for config
>1FF2 - Patch to subprogram header for CALL DIR
>1FFA - (4 bytes)
>1FFE - two bytes for the 1000Hz timer (LSB first)

Top of Config block:
MENU PROGRAM Entry:
>1FE2:	>0000	Next program
		>1900	Entry point
		>0A		Size
		TEXT 'MPD CONFIG'
(gap of 1 byte)
SUBPROGRAM HEADER FOR CALL DIR
>1FF2:	>0000	Next subprogram (automatic override to real one)
		>1D00	Entry point
		>03		Size
		TEXT 	'DIR'
		
------------------------------------------------------
We also incorporate a hand-made version of directory to simplify life a bit in BASIC and XB. A subprogram "CALL DIR" is added. To work in both BASIC and XB, we pass as a quoted string parameter:
Thus, something like "CALL DIR("DSK1")" - variables are not okay!
The output is similar to the TI BASIC catalog program in the disk controller manual, and does a simple open and print. The one difference is that it attempts my long directory support first, falling back on IF38 only if needed.
Note that this program uses some of the top of the SRAM bank as a VDP backup buffer. This should not conflict with a custom GROM0, since it can only use 6k, but any module over 6k may be affected! Don't use custom 8k GROMs with CALL DIR.

Originally done by hand, but when I had to rework part of it for Extended BASIC, I finally switched to a compiler! See separate source file.

I used my own "gpltranslate" to convert the code into RAG Assembler GPL format. I used my DF802BIN tool
to convert the file to binary (passing the 'c' compressed option). Both these tools are a bit hacky and
not on my webpage, but you can have them if you like.
