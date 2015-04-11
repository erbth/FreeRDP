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

#ifndef _VAAPI_H_
#define _VAAPI_H_

#ifdef WITH_LIBVA

#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>

#include <X11/Xlib.h>
#include <va/va.h>
struct vaapiContext
{
	Display *display;

	VADisplay vaDisplay;
	
	VAConfigID vaConfig;
	VAContextID vaContext;
	VASurfaceID *surfaces;

	int width, height;

	int numSurfaces;
	int currentSurface;

	VAImage vaImage;
};


void vaapiReleaseBuffer (void * opaque, uint8_t *data);
int vaapiGetBuffer (struct AVCodecContext *s, AVFrame *frame, int flags);
enum AVPixelFormat vaapiGetFormat (AVCodecContext *avctx, const enum AVPixelFormat *formats);

void vaapiSetupLibav (struct vaapiContext *vactx, struct AVCodecContext *avctx);

struct vaapiContext *vaapiCreateContext ();
int vaapiInit (struct vaapiContext *vactx, Display *display, int width, int height, int numSurfaces);
void vaapiDestroyContext (struct vaapiContext **pVactx);

#endif
#endif
