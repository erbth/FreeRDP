/**
 * Copyright 2015 Thomas Erbesdobler <t.erbesdobler@team103.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef WITH_LIBVA

#include <libavcodec/avcodec.h>
#include <libavcodec/vaapi.h>
#include <libavutil/avutil.h>

#include <X11/Xlib.h>
#include <va/va.h>
#include <va/va_x11.h>

#include <freerdp/codec/vaapi_h264.h>

#define VAAPI_ERROR(x) if (vaStatus != VA_STATUS_SUCCESS) { fprintf (stderr, "VAAPI ERROR: %s\n", x); return -1; }


void vaapiReleaseBuffer (void * opaque, uint8_t *data)
{
	//printf ("\tvaapiReleaseBuffer\n");
}
 
int vaapiGetBuffer (struct AVCodecContext *s, AVFrame *frame, int flags)
{
	struct vaapiContext *vactx = s->opaque;

	assert (vactx);
	assert (frame->format == AV_PIX_FMT_VAAPI_VLD);

	//printf ("\tvaapiGetBuffer\n");

	if (!frame->buf[0])
	{
		if (!vactx->vaDisplay)
		{
			if (vaapiInit (vactx, NULL, frame->width, frame->height, 32) < 0)
				return AVERROR (-1);

			vaapiSetupLibav (vactx, s);
		}
		/* TODO: check wether resolution has changed */

		if (++ vactx->currentSurface >= vactx->numSurfaces)
			vactx->currentSurface = 0;

		/* create buffer from a surface ... */
		frame->buf[0] = av_buffer_create ((uint8_t*) &vactx->surfaces[vactx->currentSurface], sizeof (VASurfaceID),
							&vaapiReleaseBuffer, vactx, AV_BUFFER_FLAG_READONLY);
		if (!frame->buf[0])
			return AVERROR (ENOMEM);
		
		/* ... and pass its ID. */
		frame->data[3] = (uint8_t*) (0L | vactx->surfaces[vactx->currentSurface]);

		//printf ("\t\tbuffer allocated (surface %d).\n", vactx->currentSurface);
	}

	return 0;
}

enum AVPixelFormat vaapiGetFormat (AVCodecContext *avctx, const enum AVPixelFormat *formats)
{
	printf ("format: %d\n", *formats);

	return *formats;
}


void vaapiSetupLibav (struct vaapiContext *vactx, AVCodecContext *avctx)
{
	struct vaapi_context *vaapiContext = avctx->hwaccel_context;

	vaapiContext->display = vactx->vaDisplay;
	vaapiContext->config_id = vactx->vaConfig;
	vaapiContext->context_id = vactx->vaContext;
}


struct vaapiContext *vaapiCreateContext ()
{
	struct vaapiContext *vactx;

	vactx = malloc (sizeof (struct vaapiContext));
	if (!vactx)
	{
		return NULL;
	}
	
	memset (vactx, 0, sizeof (struct vaapiContext));
	
	return vactx;
}


int vaapiInit (struct vaapiContext *vactx, Display *display, int width, int height, int numSurfaces)
{
	int i;
	VAConfigAttrib attrib;

	VAEntrypoint entrypoints[20];
	int numEntrypoints, vldEntrypoint;

	VAImageFormat *imageFormats;

	VAStatus vaStatus;
	int majorVer, minorVer;

	if (vactx->vaDisplay)
		return 0;
	
	/* initialize libva ... */
	if (display)
		vactx->display = display;
	
	if (!vactx->display)
		return -1;

	vactx->vaDisplay = vaGetDisplay (vactx->display);

	vaStatus = vaInitialize (vactx->vaDisplay, &majorVer, &minorVer);
	VAAPI_ERROR ("couldn't init libva.");
	
	printf ("libva version is %d.%d (why the hack did I post this ?)\n", majorVer, minorVer);

	/* ... check for profile and entrypoint ... */
	vaQueryConfigEntrypoints (vactx->vaDisplay, VAProfileH264Main, entrypoints, &numEntrypoints);

	for (vldEntrypoint = 0; vldEntrypoint < numEntrypoints; vldEntrypoint ++)
	{
		if (entrypoints[vldEntrypoint] == VAEntrypointVLD)
			break;
	}

	if (vldEntrypoint == numEntrypoints)
	{
		printf ("seems like VLD for H.264 isn't supported by vaapi, but I don't know ...\n");
		return -1;
	}

	/* ... check for color format ... */
	attrib.type = VAConfigAttribRTFormat;
	vaGetConfigAttributes (vactx->vaDisplay, VAProfileH264Main, VAEntrypointVLD, &attrib, 1);

	if (! (attrib.value & VA_RT_FORMAT_YUV420))
	{
		printf ("well, YUV420 isn't supported by vaapi, admin: ?\n");
		return -1;
	}

	/* ... and here we go: create config ... */
	vaStatus = vaCreateConfig (vactx->vaDisplay, VAProfileH264Main, VAEntrypointVLD, &attrib, 1, &vactx->vaConfig);
	VAAPI_ERROR ("couldn't create vaapi config ...");

	/* ... some surfaces ... */
	vactx->surfaces = malloc (numSurfaces * sizeof (VASurfaceID));
	if (!vactx->surfaces)
	{
		return -1;
	}

	vactx->numSurfaces = numSurfaces;

	vaStatus = vaCreateSurfaces (vactx->vaDisplay, VA_RT_FORMAT_YUV420, width, height, vactx->surfaces, numSurfaces, NULL, 0);
	VAAPI_ERROR ("couldn't create vaapi surfaces");

	vactx->width = width;
	vactx->height = height;

	/* ... and finally a virtual hw decoding pipeline. */
	vaStatus = vaCreateContext (vactx->vaDisplay, vactx->vaConfig,
					width, height, VA_PROGRESSIVE, (VASurfaceID *) vactx->surfaces, numSurfaces,
					&vactx->vaContext);
	VAAPI_ERROR ("couldn't create vaapi context");

	/* we also need an va image */
	imageFormats = malloc (vaMaxNumImageFormats (vactx->vaDisplay) * sizeof (VAImageFormat));
	if (!imageFormats)
		return -1;
	
	vaStatus = vaQueryImageFormats (vactx->vaDisplay, imageFormats, &numEntrypoints);
	VAAPI_ERROR ("couldn't query image formats ...");

	for (i = 0; i < numEntrypoints; i ++)
	{
		if (imageFormats[i].fourcc == VA_FOURCC_NV12)
			break;
	}

	if (i == numEntrypoints)
	{
		printf ("seems like vaapi doesn't support NV12 as image format. admin: ? \n");
		return -1;
	}

	vaStatus = vaCreateImage (vactx->vaDisplay, &imageFormats[i], width, height, &vactx->vaImage);
	VAAPI_ERROR ("couldn't create VAImage ...\n")

	free (imageFormats);

	return 0;
}

void vaapiDestroyContext (struct vaapiContext **pVactx)
{
	struct vaapiContext *vactx;

	if (!pVactx)
		return;
	
	if (!*pVactx)
		return;
	
	vactx = *pVactx;

	if (vactx->vaDisplay)
	{
		if (vactx->vaImage.image_id)
			vaDestroyImage (vactx->vaDisplay, vactx->vaImage.image_id);

		if (vactx->surfaces)
		{
			vaDestroySurfaces (vactx->vaDisplay, vactx->surfaces, vactx->numSurfaces);
			free (vactx->surfaces);
		}
		
		if (vactx->vaConfig)
			vaDestroyConfig (vactx->vaDisplay, vactx->vaConfig);
		
		if (vactx->vaContext)
			vaDestroyContext (vactx->vaDisplay, vactx->vaContext);
		
		vaTerminate (vactx->vaDisplay);
	}
	
	free (vactx);

	*pVactx = NULL;
}

#endif
