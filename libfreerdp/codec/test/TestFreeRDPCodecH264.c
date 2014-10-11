#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freerdp/codec/h264.h>

#define FRAMECOUNT 3

int TestFreeRDPCodecH264(int argc, char** argv)
{
	H264_CONTEXT *h264;
	BYTE *pSrcData = NULL, *pDstData = NULL;
	FILE *src, *dst;
	UINT32 nSrcSize;
	RDPGFX_RECT16 rect;
	int ret;
	char buf[20];
	int i, j;
	int count;
	
	
	h264 = h264_context_new(FALSE);
	
	for(count = 0; count < FRAMECOUNT; count ++) {
		sprintf(buf, "sample_%d.h264", count);
		
		src = fopen(buf, "rb");
		if (!src) {
			printf("couldn't open %s\n", buf);
			return 1;
		}
		
		pSrcData = malloc(100000 * sizeof(BYTE));
		nSrcSize = fread(pSrcData, 1, 100000, src);
		
		if (nSrcSize < 1) {
			free(pSrcData);
			printf("couldn't read buf\n", buf);
			return 1;
		}
		
		fclose(src);
		
		
		rect.left = 0;
		rect.top = 0;
		rect.right = 1024;
		rect.bottom = 768;
		
		
		pDstData = malloc(1024 * 768 * 4 * sizeof(BYTE));
		
		
		ret = h264_decompress(h264, pSrcData, nSrcSize, &pDstData, 0, 1024 * 4, 768, &rect, 1);
		printf("return value: %d\n", ret);
		
		printf("%dx%d, %d\n", h264->width, h264->height, nSrcSize);
		
		free(pSrcData);
		
		
		sprintf(buf, "h264_%d.ppm", count);
		
		dst = fopen(buf, "wb");
		if (!dst) {
			free(pDstData);
			printf("couldn't create %s\n", buf);
			return 1;
		}
		
		sprintf(buf, "P6\n%d %d\n255\n", h264->width, h264->height);
		fwrite(buf, 1, strlen(buf), dst);
		fflush(dst);
		
		for (i = 0; i < h264->height; i++) {
			for (j = 0; j < h264->width; j++) {
				buf[0] = *(pDstData + 2);
				buf[1] = *(pDstData + 1);
				buf[2] = *(pDstData);
				
				fwrite(buf, 1, 3, dst);
				//fflush(dst);
			}
		}
		
		fflush(dst);
		fclose(dst);
		
		
		free(pDstData);
	}
	
	h264_context_free(h264);
	
	return 0;
}