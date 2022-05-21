The DM1000 header was originally created using Classic99, with full 8k GROMs and character set loading.

ARC303G was just stripped of its header and stored raw. It loads at >A5CE and is >1FFA bytes long.

The DM1000 loader was then manually patched to include Arc303g, with Arc303g being paged into bank 2 and copied from there.

Original GROM code created by Classic99:

2036:

31 00 01 59 	MOVE >0001 TO @>8359 FROM GROM@>0001  * 99/4 is version 01, 99/4A is 02
00 01 
D6 59 01 		CEQ @>8359,>01
60 4A 			BS @>204A			* to label 1
BF 4A 0B 00 	DST >0B00,@>834A
06 00 4A 		CALL @>004A      	* 99/4A - load lowercase letters
40 51			BR @>2051        	* to label 2
BF 4A 0A 00 	DST >0A00,@>834A	* LABEL 1 - 99/4 makes a copy for upper/lower
06 00 18 		CALL @>0018
86 74 			CLR @>8374			* LABEL 2 - common - load main set
BF 4A 09 00 	DST >0900,@>834A
06 00 18 		CALL @>0018
31 00 07 A8		MOVE >0007 to VDP@>08F0 FROM GROM@>202E
F0 20 2E

BE 74 05		ST >05,@>8374
03				SCAN

31 1F 2E 8F		MOVE >1F2E TO @>A000 FROM GROM@>20D2
1D 00 20 D2 
31 1E CA 8F 	MOVE >1ECA TO @>BF2E FROM GROM@>4000
3C 2E 40 00 

BF 00 A0 00 	DST >A000, @>8300	* Load address for XML
0F F0 			XML >F0				* EXecute cart
0B				EXIT				* if it returns

// Code for ARC303 - it's hacky, but we have enough space to just
// duplicate the above code completely, with minor changes for
// location and such. So patch in the following code at the
// first free byte of 207C (offset 007c!)
207C:
31 00 01 59 	MOVE >0001 TO @>8359 FROM GROM@>0001  * 99/4 is version 01, 99/4A is 02
00 01 
D6 59 01 		CEQ @>8359,>01
60 90 			BS @>2090			* to label 1
BF 4A 0B 00 	DST >0B00,@>834A
06 00 4A 		CALL @>004A      	* 99/4A - load lowercase letters
40 97			BR @>2097        	* to label 2
BF 4A 0A 00 	DST >0A00,@>834A	* LABEL 1 - 99/4 makes a copy for upper/lower
06 00 18 		CALL @>0018
86 74 			CLR @>8374			* LABEL 2 - common - load main set
BF 4A 09 00 	DST >0900,@>834A
06 00 18 		CALL @>0018
31 00 07 A8		MOVE >0007 to VDP@>08F0 FROM GROM@>202E
F0 20 2E

BE 74 05		ST >05,@>8374
03				SCAN

21 00 01 		MOVE >0001 TO GROM@>1F12 FROM GROM@>20C1	* map in ARC303G at GROM2
1F 12 20 C1 
31 1F FA 8F 	MOVE >1FFA TO @>A5CE FROM GROM@>4000		* copy to RAM
22 CE 40 00 

BF 00 A5 CE 	DST >A5CE, @>8300	* Load address for XML
0F F0 			XML >F0				* EXecute cart
0B				EXIT				* if it returns
0F				DATA >0F			* page number for ARC303 banking

There isn't quite enough space in the header to ARC303 in the menu, too, so we throw the
extra entry at the end of the GROM. Change the >0000 at >000C to >5ED0

>5ED0: (offset 3ED0!)
0000			No next program
207C			Start of this program
0D				Length of name
41 52 43 48		ARCHIVER 3.03
49 56 45 52
20 33 2e 30 33

