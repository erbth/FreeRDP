#include <stdio.h>
#include <stdlib.h>

#ifndef _VAAPI_H_
#define _VAAPI_H_

#ifdef WITH_LIBVA

#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>

#include <X11/Xlib.h>
#include <va/va.h>
struct hwaccelSurface
{
	struct vaapiContext *vactx;

	VASurfaceID vaSurface;
	int refCount;

	VARectangle *clipRects;
	int numClipRects;
};

struct vaapiContext
{
	Display *display;

	VADisplay vaDisplay;
	
	VAConfigID vaConfig;
	VAContextID vaContext;
	VASurfaceID *baseSurfaceID;

	struct hwaccelSurface *surfaces;
	int width, height;

	int numSurfaces;
	int currentSurface;

	VAImage vaImage;
};


void vaapiReleaseBuffer (void * opaque, uint8_t *data);
int vaapiGetBuffer (struct AVCodecContext *s, AVFrame *frame, int flags);
enum AVPixelFormat vaapiGetFormat (AVCodecContext *avctx, const enum AVPixelFormat *formats);

void vaapiSetupLibav (struct vaapiContext *vactx, struct AVCodecContext *avctx);

struct hwaccelSurface *vaapiRefSurface (struct vaapiContext *vactx, VASurfaceID surface);
void vaapiUnrefSurface (struct hwaccelSurface *hwSurface);

struct vaapiContext *vaapiCreateContext ();
int vaapiInit (struct vaapiContext *vactx, Display *display, int width, int height, int numSurfaces);
void vaapiDestroyContext (struct vaapiContext **pVactx);

#endif
#endif
