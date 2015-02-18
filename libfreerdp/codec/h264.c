/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * H.264 Bitmap Compression
 *
 * Copyright 2014 Mike McDonald <Mike.McDonald@software.dell.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <winpr/crt.h>
#include <winpr/print.h>
#include <winpr/bitstream.h>

#include <freerdp/primitives.h>
#include <freerdp/codec/h264.h>
#include <freerdp/log.h>

#include <sys/time.h>

#define TAG FREERDP_TAG("codec")

/**
 * Dummy subsystem
 */

static int dummy_decompress(H264_CONTEXT* h264, BYTE* pSrcData, UINT32 SrcSize)
{
	return -1;
}

static void dummy_uninit(H264_CONTEXT* h264)
{

}

static BOOL dummy_init(H264_CONTEXT* h264)
{
	return TRUE;
}

static H264_CONTEXT_SUBSYSTEM g_Subsystem_dummy =
{
	"dummy",
	dummy_init,
	dummy_uninit,
	dummy_decompress
};

/**
 * OpenH264 subsystem
 */

#ifdef WITH_OPENH264

#include "wels/codec_def.h"
#include "wels/codec_api.h"

struct _H264_CONTEXT_OPENH264
{
	ISVCDecoder* pDecoder;
};
typedef struct _H264_CONTEXT_OPENH264 H264_CONTEXT_OPENH264;

static BOOL g_openh264_trace_enabled = FALSE;

static void openh264_trace_callback(H264_CONTEXT* h264, int level, const char* message)
{
	WLog_INFO(TAG, "%d - %s", level, message);
}

static int openh264_decompress(H264_CONTEXT* h264, BYTE* pSrcData, UINT32 SrcSize)
{
	DECODING_STATE state;
	SBufferInfo sBufferInfo;
	SSysMEMBuffer* pSystemBuffer;
	H264_CONTEXT_OPENH264* sys = (H264_CONTEXT_OPENH264*) h264->pSystemData;

	if (!sys->pDecoder)
		return -1;

	/*
	 * Decompress the image.  The RDP host only seems to send I420 format.
	 */

	h264->pYUVData[0] = NULL;
	h264->pYUVData[1] = NULL;
	h264->pYUVData[2] = NULL;

	ZeroMemory(&sBufferInfo, sizeof(sBufferInfo));

	state = (*sys->pDecoder)->DecodeFrame2(
		sys->pDecoder,
		pSrcData,
		SrcSize,
		h264->pYUVData,
		&sBufferInfo);

	/**
	 * Calling DecodeFrame2 twice apparently works around Openh264 issue #1136:
	 * https://github.com/cisco/openh264/issues/1136
	 *
	 * This is a hack, but it works and it is only necessary for the first frame.
	 */

	if (sBufferInfo.iBufferStatus != 1)
		state = (*sys->pDecoder)->DecodeFrame2(sys->pDecoder, NULL, 0, h264->pYUVData, &sBufferInfo);

	pSystemBuffer = &sBufferInfo.UsrData.sSystemBuffer;

#if 0
	WLog_INFO(TAG, "h264_decompress: state=%u, pYUVData=[%p,%p,%p], bufferStatus=%d, width=%d, height=%d, format=%d, stride=[%d,%d]",
		state, h264->pYUVData[0], h264->pYUVData[1], h264->pYUVData[2], sBufferInfo.iBufferStatus,
		pSystemBuffer->iWidth, pSystemBuffer->iHeight, pSystemBuffer->iFormat,
		pSystemBuffer->iStride[0], pSystemBuffer->iStride[1]);
#endif

	if (state != 0)
		return -1;

	if (sBufferInfo.iBufferStatus != 1)
		return -2;

	if (pSystemBuffer->iFormat != videoFormatI420)
		return -1;

	if (!h264->pYUVData[0] || !h264->pYUVData[1] || !h264->pYUVData[2])
		return -1;

	h264->iStride[0] = pSystemBuffer->iStride[0];
	h264->iStride[1] = pSystemBuffer->iStride[1];
	h264->iStride[2] = pSystemBuffer->iStride[1];

	h264->width = pSystemBuffer->iWidth;
	h264->height = pSystemBuffer->iHeight;

	return 1;
}

static void openh264_uninit(H264_CONTEXT* h264)
{
	H264_CONTEXT_OPENH264* sys = (H264_CONTEXT_OPENH264*) h264->pSystemData;

	if (sys)
	{
		if (sys->pDecoder)
		{
			(*sys->pDecoder)->Uninitialize(sys->pDecoder);
			WelsDestroyDecoder(sys->pDecoder);
			sys->pDecoder = NULL;
		}

		free(sys);
		h264->pSystemData = NULL;
	}
}

static BOOL openh264_init(H264_CONTEXT* h264)
{
	long status;
	SDecodingParam sDecParam;
	H264_CONTEXT_OPENH264* sys;
	static int traceLevel = WELS_LOG_DEBUG;
	static EVideoFormatType videoFormat = videoFormatI420;
	static WelsTraceCallback traceCallback = (WelsTraceCallback) openh264_trace_callback;

	sys = (H264_CONTEXT_OPENH264*) calloc(1, sizeof(H264_CONTEXT_OPENH264));

	if (!sys)
	{
		goto EXCEPTION;
	}

	h264->pSystemData = (void*) sys;

	WelsCreateDecoder(&sys->pDecoder);

	if (!sys->pDecoder)
	{
		WLog_ERR(TAG, "Failed to create OpenH264 decoder");
		goto EXCEPTION;
	}

	ZeroMemory(&sDecParam, sizeof(sDecParam));
	sDecParam.eOutputColorFormat  = videoFormatI420;
	sDecParam.eEcActiveIdc = ERROR_CON_FRAME_COPY;
	sDecParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;

	status = (*sys->pDecoder)->Initialize(sys->pDecoder, &sDecParam);

	if (status != 0)
	{
		WLog_ERR(TAG, "Failed to initialize OpenH264 decoder (status=%ld)", status);
		goto EXCEPTION;
	}

	status = (*sys->pDecoder)->SetOption(sys->pDecoder, DECODER_OPTION_DATAFORMAT, &videoFormat);

	if (status != 0)
	{
		WLog_ERR(TAG, "Failed to set data format option on OpenH264 decoder (status=%ld)", status);
	}

	if (g_openh264_trace_enabled)
	{
		status = (*sys->pDecoder)->SetOption(sys->pDecoder, DECODER_OPTION_TRACE_LEVEL, &traceLevel);

		if (status != 0)
		{
			WLog_ERR(TAG, "Failed to set trace level option on OpenH264 decoder (status=%ld)", status);
		}

		status = (*sys->pDecoder)->SetOption(sys->pDecoder, DECODER_OPTION_TRACE_CALLBACK, &traceCallback);

		if (status != 0)
		{
			WLog_ERR(TAG, "Failed to set trace callback option on OpenH264 decoder (status=%ld)", status);
		}

		status = (*sys->pDecoder)->SetOption(sys->pDecoder, DECODER_OPTION_TRACE_CALLBACK_CONTEXT, &h264);

		if (status != 0)
		{
			WLog_ERR(TAG, "Failed to set trace callback context option on OpenH264 decoder (status=%ld)", status);
		}
	}

	return TRUE;

EXCEPTION:
	openh264_uninit(h264);

	return FALSE;
}

static H264_CONTEXT_SUBSYSTEM g_Subsystem_OpenH264 =
{
	"OpenH264",
	openh264_init,
	openh264_uninit,
	openh264_decompress
};

#endif

/**
 * libavcodec subsystem
 */

#ifdef WITH_LIBAVCODEC

#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>

#ifdef WITH_LIBVA
#include <libavcodec/vaapi.h>
#include <va/va.h>
#include <freerdp/codec/vaapi_h264.h>
#endif

struct _H264_CONTEXT_LIBAVCODEC
{
	AVCodec* codec;
	AVCodecContext* codecContext;
	AVFrame* videoFrame;

	void *hwaccel;
};
typedef struct _H264_CONTEXT_LIBAVCODEC H264_CONTEXT_LIBAVCODEC;

static int libavcodec_decompress(H264_CONTEXT* h264, BYTE* pSrcData, UINT32 SrcSize)
{
	int status;
	int gotFrame = 0;
	AVPacket packet;
	H264_CONTEXT_LIBAVCODEC* sys = (H264_CONTEXT_LIBAVCODEC*) h264->pSystemData;

	av_init_packet(&packet);

	packet.data = pSrcData;
	packet.size = SrcSize;

	if (sys->hwaccel)
	{
#ifdef WITH_LIBVA
		struct vaapiContext *vactx = sys->hwaccel;

		VASurfaceID vaSurface;
		VAImage *vaImage;

		VAStatus vaStatus;
		
		BYTE *buf;

		if (!vactx->display)
			return -1;

		status = avcodec_decode_video2 (sys->codecContext, sys->videoFrame, &gotFrame, &packet);

		if (status < 0)
		{
			WLog_ERR (TAG, "couldn't decode video frame with vaapi (status=%s)", status);
			return -1;
		}

		if (gotFrame)
		{
			vaImage = &vactx->vaImage;

			/* unmap foreign image, well, if it is there ... */
			if (vactx->vaImage.image_id)
			{
				vaStatus = vaUnmapBuffer (vactx->vaDisplay, vaImage->buf);
				if (vaStatus != VA_STATUS_SUCCESS)
					return -1;

				vaStatus = vaDestroyImage (vactx->vaDisplay, vaImage->image_id);
				if (vaStatus != VA_STATUS_SUCCESS)
					return -1;
			}


			/* ... just wanted to say I've got it from here: http://stackoverflow.com/questions/3378560/how-to-disable-gcc-warnings-for-a-few-lines-of-code ... */
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#endif
			vaSurface = (VASurfaceID) sys->videoFrame->data[3];
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

			/* ... derive image from our surface ... */
			vaStatus = vaSyncSurface (vactx->vaDisplay, vaSurface);
			if (vaStatus != VA_STATUS_SUCCESS)
			{
				WLog_ERR (TAG, "couldn't sync surface: %s", vaErrorStr (vaStatus));
				return -1;
			}

			vaStatus = vaDeriveImage (vactx->vaDisplay, vaSurface, vaImage);
			if (vaStatus != VA_STATUS_SUCCESS)
			{
				WLog_ERR (TAG, "couldn't derive an VAImage from our surface ...\n");
				return -1;
			}

			/* ... and finally map the buffer */ 
			vaStatus = vaMapBuffer (vactx->vaDisplay, vaImage->buf, (void **) &buf);
			if (vaStatus != VA_STATUS_SUCCESS)
				return -1;

			if (vaImage->format.fourcc != VA_FOURCC_NV12)
				return -1;

			h264->pYUVData[0] = buf + vaImage->offsets[0];
			h264->pYUVData[1] = buf + vaImage->offsets[1];

			h264->iStride[0] = vaImage->pitches[0];
			h264->iStride[1] = vaImage->pitches[1];

			h264->width = vaImage->width;
			h264->height = vaImage->height;
		}
		else
			return -2;
#else
		return -1;
#endif
	}
	else
	{
		status = avcodec_decode_video2(sys->codecContext, sys->videoFrame, &gotFrame, &packet);

		if (status < 0)
		{
			WLog_ERR(TAG, "Failed to decode video frame (status=%d)", status);
			return -1;
		}

#if 0
		WLog_INFO(TAG, "libavcodec_decompress: frame decoded (status=%d, gotFrame=%d, width=%d, height=%d, Y=[%p,%d], U=[%p,%d], V=[%p,%d])",
			status, gotFrame, sys->videoFrame->width, sys->videoFrame->height,
			sys->videoFrame->data[0], sys->videoFrame->linesize[0],
			sys->videoFrame->data[1], sys->videoFrame->linesize[1],
			sys->videoFrame->data[2], sys->videoFrame->linesize[2]);
#endif

		if (gotFrame)
		{
			h264->pYUVData[0] = sys->videoFrame->data[0];
			h264->pYUVData[1] = sys->videoFrame->data[1];
			h264->pYUVData[2] = sys->videoFrame->data[2];

			h264->iStride[0] = sys->videoFrame->linesize[0];
			h264->iStride[1] = sys->videoFrame->linesize[1];
			h264->iStride[2] = sys->videoFrame->linesize[2];

			h264->width = sys->videoFrame->width;
			h264->height = sys->videoFrame->height;
		}
		else
			return -2;
	}

	return 1;
}

static void libavcodec_uninit(H264_CONTEXT* h264)
{
	H264_CONTEXT_LIBAVCODEC* sys = (H264_CONTEXT_LIBAVCODEC*) h264->pSystemData;

	if (!sys)
		return;

#ifdef WITH_LIBVA
	if (sys->hwaccel)
	{
		vaapiDestroyContext ((struct vaapiContext **) &sys->hwaccel);
	}
#endif

	if (sys->videoFrame)
	{
		av_free(sys->videoFrame);
	}

	if (sys->codecContext)
	{
		avcodec_close(sys->codecContext);
		av_free(sys->codecContext);
	}

	free(sys);
	h264->pSystemData = NULL;
}

static BOOL libavcodec_init(H264_CONTEXT* h264)
{
	H264_CONTEXT_LIBAVCODEC* sys;

	sys = (H264_CONTEXT_LIBAVCODEC*) calloc(1, sizeof(H264_CONTEXT_LIBAVCODEC));

	if (!sys)
	{
		goto EXCEPTION;
	}

	h264->pSystemData = (void*) sys;

	avcodec_register_all();

	sys->codec = avcodec_find_decoder(AV_CODEC_ID_H264);

	if (!sys->codec)
	{
		WLog_ERR(TAG, "Failed to find libav H.264 codec");
		goto EXCEPTION;
	}

	sys->codecContext = avcodec_alloc_context3(sys->codec);

	if (!sys->codecContext)
	{
		WLog_ERR(TAG, "Failed to allocate libav codec context");
		goto EXCEPTION;
	}

	if (sys->codec->capabilities & CODEC_CAP_TRUNCATED)
	{
		sys->codecContext->flags |= CODEC_FLAG_TRUNCATED;
	}

	if (avcodec_open2(sys->codecContext, sys->codec, NULL) < 0)
	{
		WLog_ERR(TAG, "Failed to open libav codec");
		goto EXCEPTION;
	}

#ifdef WITH_LIBVA
	{
		struct vaapi_context *hwaccelContext;
		struct vaapiContext *vactx;

		/* well, init vaapi like stuff ... */
		vactx = vaapiCreateContext ();
		if (!vactx)
		{
			WLog_ERR (TAG, "Could not create a vaapi context.");
			goto EXCEPTION;
		}

		sys->hwaccel = vactx;
		h264->hwaccel = vactx;

		sys->codecContext->opaque = vactx;

		sys->codecContext->get_format = &vaapiGetFormat;
		sys->codecContext->get_buffer2 = &vaapiGetBuffer;

		hwaccelContext = av_malloc (sizeof (struct vaapi_context));
		if (!hwaccelContext)
		{
			WLog_ERR (TAG, "seems like there's no memory left ...");
			goto EXCEPTION;
		}

		sys->codecContext->hwaccel_context = hwaccelContext;

		memset (hwaccelContext, 0, sizeof (struct vaapi_context));
	}
#endif

	sys->videoFrame = avcodec_alloc_frame();

	if (!sys->videoFrame)
	{
		WLog_ERR(TAG, "Failed to allocate libav frame");
		goto EXCEPTION;
	}

	return TRUE;

EXCEPTION:
#ifdef WITH_LIBVA
	vaapiDestroyContext ((struct vaapiContext **) &sys->hwaccel);
#endif
	libavcodec_uninit(h264);

	return FALSE;
}

static H264_CONTEXT_SUBSYSTEM g_Subsystem_libavcodec =
{
	"libavcodec",
	libavcodec_init,
	libavcodec_uninit,
	libavcodec_decompress
};

#endif

int h264_decompress(H264_CONTEXT* h264, BYTE* pSrcData, UINT32 SrcSize,
		BYTE** ppDstData, DWORD DstFormat, int nDstStep, int nDstWidth,
		int nDstHeight, RDPGFX_RECT16* regionRects, int numRegionRects)
{
	int index;
	int status;
	int* iStride;
	BYTE* pDstData;
	BYTE* pDstPoint;
	prim_size_t roi;
	BYTE** pYUVData;
	int width, height;
	BYTE* pYUVPoint[3];
	RDPGFX_RECT16* rect;
	primitives_t *prims = primitives_get();

	struct timeval T1, T2, T3;

	if (!h264)
		return -1;

#if 0
	WLog_INFO(TAG, "h264_decompress: pSrcData=%p, SrcSize=%u, pDstData=%p, nDstStep=%d, nDstHeight=%d, numRegionRects=%d",
		pSrcData, SrcSize, *ppDstData, nDstStep, nDstHeight, numRegionRects);
#endif

	if (!(pDstData = *ppDstData))
		return -1;

	gettimeofday (&T1, NULL);

	if ((status = h264->subsystem->Decompress(h264, pSrcData, SrcSize)) < 0)
		return status;

	gettimeofday (&T2, NULL);

	pYUVData = h264->pYUVData;
	iStride = h264->iStride;

	for (index = 0; index < numRegionRects; index++)
	{
		rect = &(regionRects[index]);

		/* Check, if the ouput rectangle is valid in decoded h264 frame. */
		if ((rect->right > h264->width) || (rect->left > h264->width))
			return -1;
		if ((rect->top > h264->height) || (rect->bottom > h264->height))
			return -1;

		/* Check, if the output rectangle is valid in destination buffer. */
		if ((rect->right > nDstWidth) || (rect->left > nDstWidth))
			return -1;
		if ((rect->bottom > nDstHeight) || (rect->top > nDstHeight))
			return -1;

		width = rect->right - rect->left;
		height = rect->bottom - rect->top;

#if 0
		WLog_INFO(TAG, "regionRect: x: %d y: %d width: %d height: %d",
		       rect->left, rect->top, width, height);
#endif
		printf ("numregionRects: %d\n", numRegionRects);

		if (h264->hwaccel)
		{
			pDstPoint = pDstData + rect->top * nDstStep + rect->left * 4;
			pYUVPoint[0] = pYUVData[0] + rect->top * iStride[0] + rect->left;

			/* NV12: there are always packets of words in the UV plane ... */
			pYUVPoint[1] = pYUVData[1] + rect->top/2 * iStride[1] + (rect->left & ~0x0001);

			roi.width = width;
			roi.height = height;

			prims->NV12ToRGB_8u_P2AC4R((const BYTE**) pYUVPoint, iStride, pDstPoint, nDstStep, &roi);
		}
		else
		{
			pDstPoint = pDstData + rect->top * nDstStep + rect->left * 4;
			pYUVPoint[0] = pYUVData[0] + rect->top * iStride[0] + rect->left;

			pYUVPoint[1] = pYUVData[1] + rect->top/2 * iStride[1] + rect->left/2;
			pYUVPoint[2] = pYUVData[2] + rect->top/2 * iStride[2] + rect->left/2;

			roi.width = width;
			roi.height = height;

			prims->YUV420ToRGB_8u_P3AC4R((const BYTE**) pYUVPoint, iStride, pDstPoint, nDstStep, &roi);
		}
	}

	gettimeofday (&T3, NULL);

	printf ("decoding took %u usec, and converting %u usec\n",
		(unsigned int) (T2.tv_usec - T1.tv_usec), (unsigned int) (T3.tv_usec - T2.tv_usec));

	return 1;
}

int h264_compress(H264_CONTEXT* h264, BYTE* pSrcData, UINT32 SrcSize, BYTE** ppDstData, UINT32* pDstSize)
{
	return 1;
}

BOOL h264_context_init(H264_CONTEXT* h264)
{
#ifdef WITH_LIBAVCODEC
	if (g_Subsystem_libavcodec.Init(h264))
	{
		h264->subsystem = &g_Subsystem_libavcodec;
		return TRUE;
	}
#endif

#ifdef WITH_OPENH264
	if (g_Subsystem_OpenH264.Init(h264))
	{
		h264->subsystem = &g_Subsystem_OpenH264;
		return TRUE;
	}
#endif

	return FALSE;
}

int h264_context_reset(H264_CONTEXT* h264)
{
	return 1;
}

H264_CONTEXT* h264_context_new(BOOL Compressor)
{
	H264_CONTEXT* h264;

	h264 = (H264_CONTEXT*) calloc(1, sizeof(H264_CONTEXT));

	if (h264)
	{
		h264->Compressor = Compressor;

		h264->subsystem = &g_Subsystem_dummy;

		if (!h264_context_init(h264))
		{
			free(h264);
			return NULL;
		}
	}

	return h264;
}

void h264_context_free(H264_CONTEXT* h264)
{
	if (h264)
	{
		h264->subsystem->Uninit(h264);

		free(h264);
	}
}
