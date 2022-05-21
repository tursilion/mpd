// Apple 2+ DOS 3.3 simulation
// No graphics modes, limited BASIC. Mostly for launching things.
// We'll see how far it goes.
// (GR and HGR would be nice... but.. eh.)
// As it is, we don't even support lowercase. ;)
//
// Screen image table: 	0x0000
// Pattern table:		0x0800
// Second pattern:		0x1000 (for FLASH)

#include <vdp.h>
#include <sound.h>

void handlecmd();
extern unsigned char appleChars[];

int nFlash = 0;
int nMode = 0;		// 0 = normal, 1=inverse, 2=flash

#define NORMAL nMode=0
#define INVERSE nMode=1
#define FLASH nMode=2

// scrn_scroll that knows about the apple character modes
void apple_scroll() {
	// hacky, slow, but functional scroll that takes minimal memory
	unsigned char x[4];		// 4 byte buffer to speed it up
	int nLine = nTextEnd-nTextRow+1;
	for (int adr=nLine+gImage; adr<nTextEnd; adr+=4) {
		vdpmemread(adr, x, 4);
		vdpmemcpy(adr-nLine, x, 4);
	}
	vdpmemset(nTextRow, '\0', nLine);	// clear the last line
}

// putstring that handles inverse,flash,normal
void putapple(char *s) {
	int add = -32;
	if (nMode == 1) add=64;
	else if (nMode == 2) add = 128;
	
	while (*s) {
		unsigned char tmp = *s;
		switch (tmp) {
		case '\r':
			nTextPos = nTextRow;
			break;

		case '\n':
			apple_scroll();
			nTextPos = nTextRow;
			break;

		default:
			// make sure it's printable first, A2 doesn't print unprintables
			if ((tmp < 32)||(tmp>127)) return;
			// filter for non-normal
			if ((nMode) && (tmp>95)) tmp-=32;
			tmp += add;
			vdpchar(nTextPos, tmp);
			nTextPos++;
			if (nTextPos > nTextEnd) {
				apple_scroll();
				nTextPos = nTextRow;
			}
			break;
		}
		s++;
	}
}

// interrupt handler for flash
void intfctn() {
	--nFlash;
	if (nFlash <= -30) nFlash=30;
	// alternating gPattern is probably a bad idea...
	if (nFlash < 0) {
		VDP_SET_REGISTER(VDP_REG_PDT, 0x02);	gPattern = 0x1000;
	} else {
		VDP_SET_REGISTER(VDP_REG_PDT, 0x01);	gPattern = 0x800;
	}
}

// copy inverted patterns to VDP
void vdpinvcpy(int pAddr, const unsigned char *pSrc, int cnt) {
	VDP_SET_ADDRESS_WRITE(pAddr);
	while (cnt--) {
		VDPWD=~(*(pSrc++));
	}
}

int main() {
	// start the powerup beep - 1khz, 0.5s
	// actual beep is complex, due to crappy hardware. Goldwave suggests
	// 1k and 16hz at full volume, and 4k at half volume. Okay. ;)
	MUTE_SOUND();
	SOUND = 0x8c;	// 1khz
	SOUND = 0x00;
	SOUND = 0xa3;	// 4khz
	SOUND = 0x00;
	SOUND = 0xc1;	// 16*15=240hz for noise channel
	SOUND = 0x03;
	SOUND = 0xe3;	// periodic noise, user rate

	SOUND = 0x90;	// full volume
	SOUND = 0xb9;	// quarter volume
//	SOUND = 0xdf;	// no volume
	SOUND = 0xf7;	// half volume

	int x = set_text();
	VDP_SET_REGISTER(7, 0x31);		// lt. green on black
	vdpmemset(gImage, 32, 960);		// fill screen with '@'

	// to get the init effect up, we activate the screen here. The rest of the init
	// provides delay for the beep to be heard and @'s to be seen
	VDP_REG1_KSCAN_MIRROR=x;
	VDP_SET_REGISTER(1, x);

	// See font.c for layout
	// hard coded addresses instead of gPattern since we set up two tables for flash
	vdpmemcpy(0x0800, appleChars, 96*8);				// normal set
	vdpinvcpy(0x0800+(96*8), appleChars, 64*8);
	vdpmemcpy(0x0800+(160*8), appleChars, 64*8);
	vdpmemcpy(0x0800+(224*8), &appleChars[96*8], 25*8);	// lowres
	
	// display the title
	vdpmemset(gImage, 0, 960);		// clear screen
	vdpmemcpy(15, "\x34\x29\xD\x19\x19\xF\x14\x21", 8);	// TI-99/4A
	
	// silence it
	MUTE_SOUND();	// sound off

	vdpmemcpy(0x1000, appleChars, 96*8);				// inverse set (for flash)
	vdpinvcpy(0x1000+(96*8), appleChars, 64*8);
	vdpinvcpy(0x1000+(160*8), appleChars, 64*8);
	vdpmemcpy(0x1000+(224*8), &appleChars[96*8], 25*8);	// lowres
	
	// delay before the rest of boot
	for (int idx=0; idx<60; idx++) vdpwaitvint();
	
	// now we can just use the print functions to get the brackets out
	putapple("\n]\n");
	
	// do any last initialization here
	VDP_INT_HOOK = intfctn;
	
	// brief delay
	for (int idx=0; idx<10; idx++) vdpwaitvint();
	
	// main loop
	// TODO: make sure you disable interrupt hook before exitting!
	NORMAL;
	for (;;) {
		putapple("\n]");
		handlecmd();
	}
	
	return 0;
}

void handlecmd() {
	// read a single command line, then process it
	
	FLASH;
	putapple(" ");	// flashing character as cursor (temp)
	NORMAL;
	
	for (;;) {
		vdpwaitvint();
		VDP_SCREEN_TIMEOUT=0xffff;
	}
}


	
	