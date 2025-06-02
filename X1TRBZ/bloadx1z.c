/* X1 4096 colors raw file to Screen CONV.(Analog 4096 colors) for ZSDCC By m@3 */
/* .COM版 */

#include <stdio.h>
#include <stdlib.h>

#define PARTS_SIZE 8000 //0x1E00 //0xf00

unsigned short y_table[200];

unsigned char mainram_data[PARTS_SIZE];

FILE *stream[2];

#define ON 1
#define OFF 0
#define ERROR 1
#define NOERROR 0

#define SIZE 40
#define LINE 200

void DI(void){
__asm
	DI
__endasm;
}

void EI(void){
__asm
	EI
__endasm;
}


short bload(char *loadfil, unsigned char *buffer, unsigned short size)
{
	if ((stream[0] = fopen( loadfil, "rb")) == NULL) {
		printf("Can\'t open file %s.", loadfil);
		return ERROR;
	}
//	printf("Load file %s", loadfil);// Address %x Size %x End %x\n", loadfil, buffer, size, buffer+size);
//	fread( buffer, 1, 4, stream[0]);
	fread( buffer, 1, size, stream[0]);
	fclose(stream[0]);
	return NOERROR;
}

//#define VRAM_MACRO(X,Y) (X + (Y / 8) * 80 + (Y & 7) * 0x800)
#define VRAM_MACRO(X,Y) (X + y_table[Y])

unsigned short adr_tmp;
unsigned char *mainram_tmp;

void trans_data(unsigned char *mainram_adr, unsigned short vram_adr)
{
	unsigned char ii,jj;
	mainram_tmp = mainram_adr;

//	vram_adr = 0x4000;

//	printf(" / trans Data %x\n", vram_adr);

	for(jj = 0 ; jj < LINE; ++jj){
		adr_tmp = VRAM_MACRO(0, jj) + vram_adr;
//		for(ii = 0 ; ii < SIZE; ++ii){
//			outp(adr_tmp++, *(mainram_adr++));

__asm
	exx
	ld	bc,(_adr_tmp)
	ld	hl,(_mainram_tmp)
	ld	d,SIZE
loop:
	ld	a,(hl)
	out	(c),a
	inc	bc
	inc	hl
	dec	d
	jr	nz,loop

	ld	(_mainram_tmp),hl
	exx
__endasm;
//		}
	}
}

/*パレット・セット*/
void pal_set(unsigned short color, unsigned char red, unsigned char green,
	unsigned char blue)
{
	unsigned short adr1;
	unsigned char adr2;
//	adr1 = (color & 0x04) / 4 * 16 * 8 + (color & 0x02) / 2 * 8;
	adr1 = (color & 0xff0) / 16;

//	adr2 = (color & 0x01) * 16 * 8;
	adr2 = (color & 0x0f) * 16;
	outp(0x1000 + adr1, adr2 + blue);
	outp(0x1100 + adr1, adr2 + red);
	outp(0x1200 + adr1, adr2 + green);

}

int	main(int argc,char **argv)
{
	char dst[100];
	unsigned char i, r, g, b, a;

	for(i = 0; i < 200; ++i){
		y_table[i] = ((i / 8) * 40 + (i & 7) * 0x800);
	}

	/* Pallet */
/*	outp(0x1000, 0xaa);
	outp(0x1100, 0xcc);
	outp(0x1200, 0xf0);*/

/*	if (argc < 2){ //argv[1] == NULL){
		printf("X1 BRG file Loader.\n");
		return ERROR;
	}*/

	/* Change Pallet */
	outp(0x1000, 0xa2);
	outp(0x1100, 0xc4);
	outp(0x1200, 0xf0);

	/* Priority */
//	outp(0x1300, 0xfe);

	outp(0x1fb0, 0x80);	/* 多色モード */
	outp(0x1fc5, 0x80);	/* グラフィックパレットアクセスON */

//	pal_allblack(0);

//	outp(0x1fc0, 0x01);	/* 多色モードプライオリティ */

	for(r = 0; r < 16; ++r){
		for(g = 0; g < 16; ++g){
			for(b = 0; b < 16; ++b){
				pal_set(b + r * 16 + g * 256, r, g, b);
			}
		}
	}


/*	snprintf(dst, sizeof dst, "%s.grb", argv[1]);
	bload(dst, mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0x4000);

	snprintf(dst, sizeof dst, "%s.grr", argv[1]);
	bload(dst, mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0x8000);

	snprintf(dst, sizeof dst, "%s.grg", argv[1]);
	bload(dst, mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0xc000);
*/

//	outp(0x1fd0, 0); /* BANK0 */

	bload("B3.raw", mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0x4000);

	bload("B2.raw", mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0x4400);

	bload("R3.raw", mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0x8000);

	bload("R2.raw", mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0x8400);

	bload("G3.raw", mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0xc000);

	bload("G2.raw", mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0xc400);

	a = inp(0x1fd0);
	outp(0x1fd0, a | 0x10); /* BANK1 */

	bload("B1.raw", mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0x4000);

	bload("B0.raw", mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0x4400);

	bload("R1.raw", mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0x8000);

	bload("R0.raw", mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0x8400);

	bload("G1.raw", mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0xc000);

	bload("G0.raw", mainram_data, PARTS_SIZE);
	trans_data(mainram_data, 0xc400);

	outp(0x1fd0, 0); /* 元に戻す */

	getchar();

	outp(0x1fc5, 0x0);
	outp(0x1fb0, 0x0);

	/* Pallet */
	outp(0x1000, 0xaa);
	outp(0x1100, 0xcc);
	outp(0x1200, 0xf0);

	/* Priority */
	outp(0x1300, 0x00);

	return NOERROR;
}

