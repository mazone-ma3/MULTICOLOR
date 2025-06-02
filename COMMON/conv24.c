/* conv24.c by m@3 with Grok. */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

// BMPヘッダ
typedef struct {
	uint32_t file_size;
	uint32_t pixel_offset;
	uint32_t width;
	uint32_t height;
	uint16_t bits_per_pixel;
} BmpHeader;

void pset(int x, int y, int code, unsigned char *color)
{
	int k, l = 0, j;
	for(int i = 0; i < 12; ++i){
		j = (x / 8) + (y * 320 / 8) + (320 * 200 / 8) * i; //(11 - i);
		k = 7 - x % 8;
		if(BITTST(i, code)){
//	printf("%d ",code);
			BITSET(k, color[j]);
		}else{
			BITCLR(k, color[j]);
		}
//		color[j]  = i;
	}
}

unsigned char x1tzcolor[320*200 / 8 * 4 * 3];

// BMP読み込み（24bit対応）
void read_bmp(const char *filename, BmpHeader *header, uint8_t **pixels) {
	FILE *fp = fopen(filename, "rb");
	if (!fp) { fprintf(stderr, "Error: Cannot open %s\n", filename); exit(1); }
	fseek(fp, 2, SEEK_SET); fread(&header->file_size, 4, 1, fp);
	fseek(fp, 10, SEEK_SET); fread(&header->pixel_offset, 4, 1, fp);
	fseek(fp, 18, SEEK_SET); fread(&header->width, 4, 1, fp);
	fseek(fp, 22, SEEK_SET); fread(&header->height, 4, 1, fp);
	fseek(fp, 28, SEEK_SET); fread(&header->bits_per_pixel, 2, 1, fp);
	if (header->width != 320 || header->height != 200 || header->bits_per_pixel != 24) {
		fprintf(stderr, "Error: Only 320x200 24-bit BMP supported\n");
		fprintf(stderr, "(%d x %d x %dbit)\n", header->width, header->height, header->bits_per_pixel);
		fclose(fp); exit(1);
	}
	*pixels = malloc(header->width * header->height * 3); // 3バイト/ピクセル
	fseek(fp, header->pixel_offset, SEEK_SET);
	fread(*pixels, 3, header->width * header->height, fp);
	fclose(fp);
}

	unsigned char pattern[320 * 200 * 3 * 4 / 8]; //3 * 4 * 12];

// VRAM用.raw出力（プレーンドアクセス、R/G/B各8KB）
void write_raw(const char *filename, uint8_t *pixels, int width, int height) {
	FILE *fp = fopen(filename, "wb");
	if (!fp) { fprintf(stderr, "Error: Cannot open %s\n", filename); exit(1); }
	uint8_t *r_plane = malloc(width * height *2);/// 1); // 4ビット/ピクセル=0.5バイト
	uint8_t *g_plane = malloc(width * height *2);/// 1);
	uint8_t *b_plane = malloc(width * height *2);/// 1);

	int index = 0, j = 0, m = 0, n = 0;

	uint8_t r;// = pixels[i * 3];	 // R（0~255）
	uint8_t g;// = pixels[i * 3 + 1]; // G（0~255）
	uint8_t b;// = pixels[i * 3 + 2]; // B（0~255）
	uint8_t r4;// = r >> 4;		   // 4ビットR（0~15）
	uint8_t g4;// = g >> 4;		   // 4ビットG
	uint8_t b4;// = b >> 4;		   // 4ビットB
	int color, l;
	for(l = 0; l < 320 * 200*4*3/8; ++l){
		x1tzcolor[l] = 0x0; //ff;
	}
	int i;

	for (i = 0; i < width * height; ++i){
//	for (i = width * height - 1; i >= 0; --i){
			int j = width * height - i - 1;
			// RGB888（24bit）からRGB444に減色
			r = pixels[j * 3 + 2]; // R（0~255）
			g = pixels[j * 3 + 1]; // G（0~255）
			b = pixels[j * 3 + 0]; // B（0~255）
			r4 = r >> 4;		   // 4ビットR（0~15）
			g4 = g >> 4;		   // 4ビットG
			b4 = b >> 4;		   // 4ビットB

			color = (b4) + (g4) * 16 + (r4) * 256;

		// ツタンカーメンの金色（R:255, G:215, B:0）優先
//		if (r >= 192 && g >= 160 && b <= 32) { // 金色範囲（RGB888）
//			r4 = 15; g4 = 13; b4 = 0; // RGB444で金色（R:15, G:13, B:0）
//		}
		// 2ピクセルで1バイト（プレーンドアクセス）
//		if (i % 2 == 0) {
//			r_plane[offset[j] + idx[plane]] = r4 << 4; // 左4ビット
//			g_plane[offset[j] + idx[plane]] = g4 << 4;
//			b_plane[offset[j] + idx[plane]] = b4 << 4;
//		} else {
//			r_plane[offset[j] + idx[plane]] |= r4;	 // 右4ビット
//			g_plane[offset[j] + idx[plane]] |= g4;
//			b_plane[offset[j] + idx[plane]] |= b4;
//			idx[plane]++;
//		}
//		}else{
			// RGB888（24bit）からRGB444に減色
/*			r = pixels[i * 3];	 // R（0~255）
			g = pixels[i * 3 + 1]; // G（0~255）
			b = pixels[i * 3 + 2]; // B（0~255）
			r4 = r >> 4;		   // 4ビットR（0~15）
			g4 = g >> 4;		   // 4ビットG
			b4 = b >> 4;		   // 4ビットB

			color = r + g * 8 + b * 12;
		}
*/

		pset(320 - 1 - i % 320, i / 320, 0xfff * 0 + color * 1, x1tzcolor); //[i * 3]);
	}
//	printf("index = %d\n", index);
	printf("i = %d\n", i);

	printf("Done.\n");

//	fwrite(r_plane, 1, width * height / 2, fp); // 8KB
//	fwrite(g_plane, 1, width * height / 2, fp); // 8KB
//	fwrite(b_plane, 1, width * height / 2, fp); // 8KB

//	fwrite(r_plane, 1, idx[plane], fp); // 8KB
//	fwrite(g_plane, 1, idx[plane], fp); // 8KB
//	fwrite(b_plane, 1, idx[plane], fp); // 8KB

//	fwrite(pattern, 1, index, fp); // 8KB
	fwrite(x1tzcolor, 1, 320*200*4*3/8, fp); // 8KB

	free(r_plane); free(g_plane); free(b_plane);
	fclose(fp);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s input.bmp output.raw\n", argv[0]);
		return 1;
	}
	BmpHeader header;
	uint8_t *pixels = NULL;
	read_bmp(argv[1], &header, &pixels);
	write_raw(argv[2], pixels, header.width, header.height);
	free(pixels);
	printf("Converted %s to %s (RGB444 planes for FM77AV/X1turboZ)\n", argv[1], argv[2]);
	return 0;
}