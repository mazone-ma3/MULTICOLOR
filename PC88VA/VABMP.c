/* VABMP.c OpenWatcom By m@3*/

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <i86.h>
#include <fcntl.h>
#include <dos.h>
#include <unistd.h>

#define TVRAM_ON() 	outp(0x153, 0x51)		/* T-VRAM�I�� */
#define GVRAM_ON() 	outp(0x153, 0x54)		/* G-VRAM�I�� */

/************************************************************************/
/*		BIT����}�N����`												*/
/************************************************************************/

/* BIT�f�[�^�Z�o */
#define BITDATA(n) (1 << (n))

/* BIT�Z�b�g */
#define BITSET(BITNUM, NUMERIC) {	\
	NUMERIC |= BITDATA(BITNUM);		\
}

/* BIT�N���A */
#define BITCLR(BITNUM, NUMERIC) {	\
	NUMERIC &= ~BITDATA(BITNUM);	\
}

/* BIT�`�F�b�N */
#define BITTST(BITNUM, NUMERIC) (NUMERIC & BITDATA(BITNUM))

/* BIT���] */
#define BITNOT(BITNUM, NUMERIC) {	\
	NUMERIC ^= BITDATA(BITNUM);		\
}


void g_init(void)
{
	union REGS reg;
	union REGS reg_out;

//	reg.h.ah = 0x2a;
//	int86(0x83, &reg, &reg_out);	/* �e�L�X�g������ */

	reg.h.ah = 0x00;
	reg.x.bx = 0xa006; // 640
	reg.h.cl=16;
	reg.h.ch=1;
	int86(0x8f, &reg, &reg_out);	/* �O���t�B�b�NBIOS������ */

	outpw(0x100, 0xb462);	/* none-interless Graphic-on 400dot(200line) */
	return;

}

unsigned short __far *vram2;

// BMP�w�b�_
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

// BMP�ǂݍ��݁i24bit�Ή��j
void read_bmp(const char *filename, BmpHeader *header, unsigned char **pixels)
{
	int i, j;
	unsigned short color;
	unsigned short r;		// R�i0~255�j
	unsigned short g;		// G�i0~255�j
	unsigned short b;		// B�i0~255�j
	unsigned short r4;		// 4�r�b�gR�i0~15�j
	unsigned short g4;		// 4�r�b�gG
	unsigned short b4;		// 4�r�b�gB
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
	*pixels = malloc(header->width * 3);	// 3�o�C�g/�s�N�Z��
	fseek(fp, header->pixel_offset, SEEK_SET);
	for(i = 0; i <  header->height; ++i){
		for(j = 0; j < header->width; j++){
			fread(pixels1, 3, 1, fp);

			r = pixels1[2];			// R�i0~255�j8bit
			g = pixels1[1];			// G�i0~255�j8bit
			b = pixels1[0];			// B�i0~255�j8bit

			r4 = (r / 8) & 0x1f;	// 5�r�b�gR�i0~15�j
			g4 = (g / 4) & 0x3f;	// 6�r�b�gG
			b4 = (b / 8) & 0x1f;	// 5�r�b�gB

			color = (unsigned short)((b4) | ((r4) * (1 << 5)) | ((g4) * (1 << 10)));

			pset(j, header->height - 1 - i, 0xffff*0 + 1 * color);
		}
	}
	fclose(fp);
	free(pixels);
}

// ���C�����[�`�� �����ݒ�
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
	outp(0x146, 0x02);	/* �J�[�\��OFF */

	GVRAM_ON();
	outp(0x580, 0x10);

	_disable();
	outp(0x44, 0x07);
	outp(0x45, 0x00);
	_enable();

	read_bmp(argv[1], &header, &pixels);
	return 0;
}
