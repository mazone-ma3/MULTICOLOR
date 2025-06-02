/* div.c by m@3 with Grok. */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

enum {
	CODE_BLUE  = 0,
	CODE_GREEN = 1,
	CODE_RED   = 2
};

// プレーンサイズ（320x200×4ビット÷8＝8000バイト）
#define PLANE_SIZE 8000
// フレーム数（96KB÷24KB＝4）
#define FRAME_COUNT 4
// 出力ファイル数（4フレーム×3プレーン＝12）
#define FILE_COUNT (FRAME_COUNT * 3)

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s input.raw\n", argv[0]);
		return 1;
	}

	// 入力ファイルオープン
	FILE *fp_in = fopen(argv[1], "rb");
	if (!fp_in) {
		fprintf(stderr, "Error: Cannot open %s\n", argv[1]);
		return 1;
	}

	// ファイルサイズ確認
	fseek(fp_in, 0, SEEK_END);
	long file_size = ftell(fp_in);
	fseek(fp_in, 0, SEEK_SET);
	if (file_size < PLANE_SIZE * FILE_COUNT) {
		fprintf(stderr, "Error: Input file %s is too small (%ld bytes, expected >= %d bytes)\n",
				argv[1], file_size, PLANE_SIZE * FILE_COUNT);
		fclose(fp_in);
		return 1;
	}
	printf("Input file size: %ld bytes (using %d bytes for 4 frames)\n", file_size, PLANE_SIZE * FILE_COUNT);

	// バッファ確保
	uint8_t *buffer = malloc(PLANE_SIZE * FILE_COUNT);
	if (!buffer) {
		fprintf(stderr, "Error: Memory allocation failed\n");
		fclose(fp_in);
		return 1;
	}

	// 入力読み込み（96KB）
	size_t bytes_read = fread(buffer, 1, PLANE_SIZE * FILE_COUNT, fp_in);
	if (bytes_read != PLANE_SIZE * FILE_COUNT) {
		fprintf(stderr, "Error: Failed to read %d bytes from %s\n", PLANE_SIZE * FILE_COUNT, argv[1]);
		free(buffer);
		fclose(fp_in);
		return 1;
	}
	fclose(fp_in);

	// 出力ファイル（R0.raw, G0.raw, B0.raw, ..., R3.raw, G3.raw, B3.raw）
	for (int frame = 0; frame < FRAME_COUNT; frame++) {
		char filename[16];
		FILE *fp_out;
		// Bプレーン
		sprintf(filename, "B%d.raw", frame);
		fp_out = fopen(filename, "wb");
		if (!fp_out) {
			fprintf(stderr, "Error: Cannot open %s\n", filename);
			free(buffer);
			return 1;
		}
		fwrite(buffer + frame * PLANE_SIZE * 1 + 4 *  CODE_BLUE * PLANE_SIZE, 1, PLANE_SIZE, fp_out);
		fclose(fp_out);
		printf("Generated: %s (%d bytes)\n", filename, PLANE_SIZE);
	}

	for (int frame = 0; frame < FRAME_COUNT; frame++) {
		char filename[16];
		FILE *fp_out;
		// Gプレーン
		sprintf(filename, "G%d.raw", frame);
		fp_out = fopen(filename, "wb");
		if (!fp_out) {
			fprintf(stderr, "Error: Cannot open %s\n", filename);
			free(buffer);
			return 1;
		}
		fwrite(buffer + frame * PLANE_SIZE * 1 + 4 * CODE_GREEN * PLANE_SIZE, 1, PLANE_SIZE, fp_out);
		fclose(fp_out);
		printf("Generated: %s (%d bytes)\n", filename, PLANE_SIZE);
	}

	for (int frame = 0; frame < FRAME_COUNT; frame++) {
		char filename[16];
		FILE *fp_out;
		// Rプレーン
		sprintf(filename, "R%d.raw", frame);
		fp_out = fopen(filename, "wb");
		if (!fp_out) {
			fprintf(stderr, "Error: Cannot open %s\n", filename);
			free(buffer);
			return 1;
		}
		fwrite(buffer + frame * PLANE_SIZE * 1 + 4 *  CODE_RED * PLANE_SIZE, 1, PLANE_SIZE, fp_out);
		fclose(fp_out);
		printf("Generated: %s (%d bytes)\n", filename, PLANE_SIZE);

	}

	free(buffer);
	printf("Generated %d files (R0.raw to B%d.raw) for FM77AV/X1turboZ\n", FILE_COUNT, FRAME_COUNT - 1);

	return 0;
}