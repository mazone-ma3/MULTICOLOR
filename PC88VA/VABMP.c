/* VABMP.c OpenWatcom By m@3*/

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <i86.h>
#include <fcntl.h>
#include <dos.h>
#include <unistd.h>

#define TVRAM_ON() 	outp(0x153, 0x51)		/* T-VRAM選択 */
#define GVRAM_ON() 	outp(0x153, 0x54)		/* G-VRAM選択 */

/************************************************************************/
/*		BIT操作マクロ定義												*/
/************************************************************************/

/* BITデータ算出 */
#define BITDATA(n) (1 << (n))

/* BITセット */
#define BITSET(BITNUM, NUMERIC) {	\
	NUMERIC |= BITDATA(BITNUM);		\
}

/* BITクリア */
#define BITCLR(BITNUM, NUMERIC) {	\
	NUMERIC &= ~BITDATA(BITNUM);	\
}

/* BITチェック */
#define BITTST(BITNUM, NUMERIC) (NUMERIC & BITDATA(BITNUM))

/* BIT反転 */
#define BITNOT(BITNUM, NUMERIC) {	\
	NUMERIC ^= BITDATA(BITNUM);		\
}


void g_init(void)
{
	union REGS reg;
	union REGS reg_out;

//	reg.h.ah = 0x2a;
//	int86(0x83, &reg, &reg_out);	/* テキスト初期化 */

	reg.h.ah = 0x00;
	reg.x.bx = 0xa006; // 640
	reg.h.cl=16;
	reg.h.ch=1;
	int86(0x8f, &reg, &reg_out);	/* グラフィックBIOS初期化 */

	outpw(0x100, 0xb462);	/* none-interless Graphic-on 400dot(200line) */
	return;

}

unsigned short __far *vram2;

// BMPヘッダ
typedef struct {
	unsigned long file_size;
	unsigned long pixel_offset;
	unsigned long width;
	unsigned long height;
	unsigned short bits_per_pixel;
} BmpHeader;

#define DIV 1

void pset(unsigned long x, unsigned long y, unsigned short code)
{
	int i;

	int k, l = 0, j;
	unsigned long m;
	unsigned short n;
	unsigned short code2;

	m = (x + y * 640) * 2;

	if(m < 0x10000L){
		n = m;
		vram2 = (unsigned short __far *)MK_FP(0x0a000, 0);
	}else if (m < 0x20000L){
		n = (m - 0x10000L);
		vram2 = (unsigned short __far *)MK_FP(0x0b000, 0);
	}else if (m < 0x30000L){
		n = (m - 0x20000L);
		vram2 = (unsigned short __far *)MK_FP(0x0c000, 0);
	}else{
		n = (m - 0x30000L);
		vram2 = (unsigned short __far *)MK_FP(0x0d000, 0);
	}

	vram2[n/2] = code;
}

// BMP読み込み（24bit対応）
void read_bmp(const char *filename, BmpHeader *header, unsigned char **pixels)
{
	int i, j;
	unsigned short color;
	unsigned short r;		// R（0~255）
	unsigned short g;		// G（0~255）
	unsigned short b;		// B（0~255）
	unsigned short r4;		// 4ビットR（0~15）
	unsigned short g4;		// 4ビットG
	unsigned short b4;		// 4ビットB
	unsigned char pixels1[4];

	FILE *fp = fopen(filename, "rb");
	if (!fp) { fprintf(stderr, "Error: Cannot open %s\n", filename); exit(1); }
	fseek(fp, 2, SEEK_SET); fread(&header->file_size, 4, 1, fp);
	fseek(fp, 10, SEEK_SET); fread(&header->pixel_offset, 4, 1, fp);
	fseek(fp, 18, SEEK_SET); fread(&header->width, 4, 1, fp);
	fseek(fp, 22, SEEK_SET); fread(&header->height, 4, 1, fp);
	fseek(fp, 28, SEEK_SET); fread(&header->bits_per_pixel, 2, 1, fp);
	if (header->width != 640 || header->height != 200 || header->bits_per_pixel != 24) {
		fprintf(stderr, "Error: Only 640x200 24-bit BMP supported\n");
		fprintf(stderr, "(%d x %d x %dbit)\n", header->width, header->height, header->bits_per_pixel);
		fclose(fp); exit(1);
	}
	*pixels = malloc(header->width * 3);	// 3バイト/ピクセル
	fseek(fp, header->pixel_offset, SEEK_SET);
	for(i = 0; i <  header->height; ++i){
		for(j = 0; j < header->width; j++){
			fread(pixels1, 3, 1, fp);

			r = pixels1[2];			// R（0~255）8bit
			g = pixels1[1];			// G（0~255）8bit
			b = pixels1[0];			// B（0~255）8bit

			r4 = (r / 8) & 0x1f;	// 5ビットR（0~15）
			g4 = (g / 4) & 0x3f;	// 6ビットG
			b4 = (b / 8) & 0x1f;	// 5ビットB

			color = (unsigned short)((b4) | ((r4) * (1 << 5)) | ((g4) * (1 << 10)));

			pset(j, header->height - 1 - i, 0xffff*0 + 1 * color);
		}
	}
	fclose(fp);
	free(pixels);
}

// メインルーチン 初期設定
int main(int argc, char *argv[])
{
	short i, j, k = 0;
	short errlv;

	BmpHeader header;

	unsigned char *pixels = NULL;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s input.bmp\n", argv[0]);
		return 1;
	}


	g_init();

	while(inp(0x142) & 0x05);
	outp(0x142, 0x15);	/* CURDEF */
	while(inp(0x142) & 0x05);
	outp(0x146, 0x02);	/* カーソルOFF */

	GVRAM_ON();
	outp(0x580, 0x10);

	_disable();
	outp(0x44, 0x07);
	outp(0x45, 0x00);
	_enable();

	read_bmp(argv[1], &header, &pixels);
	return 0;
}
