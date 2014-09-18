/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 *
 * Copyright 2011-2014 Marc-Andre Moreau <marcandre.moreau@gmail.com>
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

#include <winpr/crt.h>
#include <winpr/synch.h>
#include <winpr/input.h>
#include <winpr/sysinfo.h>

#include <freerdp/codec/color.h>
#include <freerdp/codec/region.h>

#include "../shadow_screen.h"
#include "../shadow_surface.h"

#include "mac_shadow.h"

static macShadowSubsystem* g_Subsystem = NULL;

void mac_shadow_input_synchronize_event(macShadowSubsystem* subsystem, UINT32 flags)
{

}

void mac_shadow_input_keyboard_event(macShadowSubsystem* subsystem, UINT16 flags, UINT16 code)
{
	DWORD vkcode;
	DWORD keycode;
	BOOL extended;
	CGEventRef kbdEvent;
	CGEventSourceRef source;
	
	extended = (flags & KBD_FLAGS_EXTENDED) ? TRUE : FALSE;
	
	if (extended)
		code |= KBDEXT;
	
	vkcode = GetVirtualKeyCodeFromVirtualScanCode(code, 4);
	
	if (extended)
		vkcode |= KBDEXT;
	
	keycode = GetKeycodeFromVirtualKeyCode(vkcode, KEYCODE_TYPE_APPLE) - 8;
	
	if (keycode)
	{
		source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
	
		if (flags & KBD_FLAGS_DOWN)
		{
			kbdEvent = CGEventCreateKeyboardEvent(source, (CGKeyCode) keycode, TRUE);
			CGEventPost(kCGHIDEventTap, kbdEvent);
			CFRelease(kbdEvent);
		}
		else if (flags & KBD_FLAGS_RELEASE)
		{
			kbdEvent = CGEventCreateKeyboardEvent(source, (CGKeyCode) keycode, FALSE);
			CGEventPost(kCGHIDEventTap, kbdEvent);
			CFRelease(kbdEvent);
		}
	
		CFRelease(source);
	}
}

void mac_shadow_input_unicode_keyboard_event(macShadowSubsystem* subsystem, UINT16 flags, UINT16 code)
{

}

void mac_shadow_input_mouse_event(macShadowSubsystem* subsystem, UINT16 flags, UINT16 x, UINT16 y)
{
	UINT32 scrollX = 0;
	UINT32 scrollY = 0;
	CGWheelCount wheelCount = 2;
	
	if (flags & PTR_FLAGS_WHEEL)
	{
		scrollY = flags & WheelRotationMask;
		
		if (flags & PTR_FLAGS_WHEEL_NEGATIVE)
		{
			scrollY = -(flags & WheelRotationMask) / 392;
		}
		else
		{
			scrollY = (flags & WheelRotationMask) / 120;
		}
		
		CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
		CGEventRef scroll = CGEventCreateScrollWheelEvent(source, kCGScrollEventUnitLine,
								  wheelCount, scrollY, scrollX);
		CGEventPost(kCGHIDEventTap, scroll);
		
		CFRelease(scroll);
		CFRelease(source);
	}
	else
	{
		CGEventSourceRef source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
		CGEventType mouseType = kCGEventNull;
		CGMouseButton mouseButton = kCGMouseButtonLeft;
		
		if (flags & PTR_FLAGS_MOVE)
		{
			if (subsystem->mouseDownLeft)
				mouseType = kCGEventLeftMouseDragged;
			else if (subsystem->mouseDownRight)
				mouseType = kCGEventRightMouseDragged;
			else if (subsystem->mouseDownOther)
				mouseType = kCGEventOtherMouseDragged;
			else
				mouseType = kCGEventMouseMoved;
			
			CGEventRef move = CGEventCreateMouseEvent(source, mouseType, CGPointMake(x, y), mouseButton);
			CGEventPost(kCGHIDEventTap, move);
			CFRelease(move);
		}
		
		if (flags & PTR_FLAGS_BUTTON1)
		{
			mouseButton = kCGMouseButtonLeft;
			
			if (flags & PTR_FLAGS_DOWN)
			{
				mouseType = kCGEventLeftMouseDown;
				subsystem->mouseDownLeft = TRUE;
			}
			else
			{
				mouseType = kCGEventLeftMouseUp;
				subsystem->mouseDownLeft = FALSE;
			}
		}
		else if (flags & PTR_FLAGS_BUTTON2)
		{
			mouseButton = kCGMouseButtonRight;
			
			if (flags & PTR_FLAGS_DOWN)
			{
				mouseType = kCGEventRightMouseDown;
				subsystem->mouseDownRight = TRUE;
			}
			else
			{
				mouseType = kCGEventRightMouseUp;
				subsystem->mouseDownRight = FALSE;
			}
			
		}
		else if (flags & PTR_FLAGS_BUTTON3)
		{
			mouseButton = kCGMouseButtonCenter;
			
			if (flags & PTR_FLAGS_DOWN)
			{
				mouseType = kCGEventOtherMouseDown;
				subsystem->mouseDownOther = TRUE;
			}
			else
			{
				mouseType = kCGEventOtherMouseUp;
				subsystem->mouseDownOther = FALSE;
			}
		}
		
		CGEventRef mouseEvent = CGEventCreateMouseEvent(source, mouseType, CGPointMake(x, y), mouseButton);
		CGEventPost(kCGHIDEventTap, mouseEvent);
		
		CFRelease(mouseEvent);
		CFRelease(source);
	}
}

void mac_shadow_input_extended_mouse_event(macShadowSubsystem* subsystem, UINT16 flags, UINT16 x, UINT16 y)
{

}

int mac_shadow_detect_monitors(macShadowSubsystem* subsystem)
{
	size_t wide, high;
	MONITOR_DEF* monitor;
	CGDirectDisplayID displayId;
	
	displayId = CGMainDisplayID();
	
	CGDisplayModeRef mode = CGDisplayCopyDisplayMode(displayId);
	
	subsystem->pixelWidth = CGDisplayModeGetPixelWidth(mode);
	subsystem->pixelHeight = CGDisplayModeGetPixelHeight(mode);
	
	wide = CGDisplayPixelsWide(displayId);
	high = CGDisplayPixelsHigh(displayId);
	
	CGDisplayModeRelease(mode);
	
	subsystem->retina = ((subsystem->pixelWidth / wide) == 2) ? TRUE : FALSE;
	
	if (subsystem->retina)
	{
		subsystem->width = wide;
		subsystem->height = high;
	}
	else
	{
		subsystem->width = subsystem->pixelWidth;
		subsystem->height = subsystem->pixelHeight;
	}
	
	subsystem->monitorCount = 1;
	
	monitor = &(subsystem->monitors[0]);
	
	monitor->left = 0;
	monitor->top = 0;
	monitor->right = subsystem->width;
	monitor->bottom = subsystem->height;
	monitor->flags = 1;
	
	return 1;
}

int mac_shadow_capture_start(macShadowSubsystem* subsystem)
{
	CGError err;
	
	err = CGDisplayStreamStart(subsystem->stream);
	
	if (err != kCGErrorSuccess)
		return -1;
	
	return 1;
}

int mac_shadow_capture_stop(macShadowSubsystem* subsystem)
{
	CGError err;
	
	err = CGDisplayStreamStop(subsystem->stream);
	
	if (err != kCGErrorSuccess)
		return -1;
	
	return 1;
}

int mac_shadow_capture_peek_dirty_region(macShadowSubsystem* subsystem)
{
	size_t index;
	size_t numRects;
	const CGRect* rects;
	RECTANGLE_16 invalidRect;
	
	rects = CGDisplayStreamUpdateGetRects(subsystem->lastUpdate, kCGDisplayStreamUpdateDirtyRects, &numRects);
	
	if (!numRects)
		return -1;
	
	for (index = 0; index < numRects; index++)
	{
		invalidRect.left = (UINT16) rects[index].origin.x;
		invalidRect.top = (UINT16) rects[index].origin.y;
		invalidRect.right = invalidRect.left + (UINT16) rects[index].size.width;
		invalidRect.bottom = invalidRect.top + (UINT16) rects[index].size.height;
		
		if (subsystem->retina)
		{
			/* scale invalid rect */
			invalidRect.left /= 2;
			invalidRect.top /= 2;
			invalidRect.right /= 2;
			invalidRect.bottom /= 2;
		}
		
		region16_union_rect(&(subsystem->invalidRegion), &(subsystem->invalidRegion), &invalidRect);
	}
	
	return 0;
}

int mac_shadow_capture_get_dirty_region(macShadowSubsystem* subsystem)
{
	dispatch_semaphore_wait(subsystem->regionSemaphore, DISPATCH_TIME_FOREVER);
	
	if (subsystem->lastUpdate)
	{
		mac_shadow_capture_peek_dirty_region(subsystem);
	}
	
	dispatch_semaphore_signal(subsystem->regionSemaphore);
	
	return 1;
}

int mac_shadow_capture_clear_dirty_region(macShadowSubsystem* subsystem)
{
	dispatch_semaphore_wait(subsystem->regionSemaphore, DISPATCH_TIME_FOREVER);
	
	CFRelease(subsystem->lastUpdate);
	subsystem->lastUpdate = NULL;
	
	dispatch_semaphore_signal(subsystem->regionSemaphore);
	
	return 1;
}

int mac_shadow_capture_surface_copy(macShadowSubsystem* subsystem)
{
	dispatch_semaphore_wait(subsystem->regionSemaphore, DISPATCH_TIME_FOREVER);
	subsystem->updateReady = TRUE;
	
	dispatch_semaphore_wait(subsystem->dataSemaphore, DISPATCH_TIME_FOREVER);
	dispatch_semaphore_signal(subsystem->regionSemaphore);
	
	dispatch_semaphore_wait(subsystem->dataSemaphore, DISPATCH_TIME_FOREVER);
	dispatch_semaphore_signal(subsystem->dataSemaphore);
	
	return 1;
}

void (^mac_capture_stream_handler)(CGDisplayStreamFrameStatus, uint64_t, IOSurfaceRef, CGDisplayStreamUpdateRef) =
	^(CGDisplayStreamFrameStatus status, uint64_t displayTime, IOSurfaceRef frameSurface, CGDisplayStreamUpdateRef updateRef)
{
	int x, y;
	int count;
	int width;
	int height;
	int nSrcStep;
	BYTE* pSrcData;
	RECTANGLE_16 surfaceRect;
	const RECTANGLE_16* extents;
	macShadowSubsystem* subsystem = g_Subsystem;
	rdpShadowServer* server = subsystem->server;
	rdpShadowSurface* surface = server->surface;
	
	if (ArrayList_Count(server->clients) < 1)
	{
		region16_clear(&(subsystem->invalidRegion));
		return;
	}
	
	dispatch_semaphore_wait(subsystem->regionSemaphore, DISPATCH_TIME_FOREVER);
	
	if (!subsystem->updateReady)
	{
		dispatch_semaphore_signal(subsystem->regionSemaphore);
		return;
	}
	
	mac_shadow_capture_peek_dirty_region(subsystem);
		
	surfaceRect.left = surface->x;
	surfaceRect.top = surface->y;
	surfaceRect.right = surface->x + surface->width;
	surfaceRect.bottom = surface->y + surface->height;
		
	region16_intersect_rect(&(subsystem->invalidRegion), &(subsystem->invalidRegion), &surfaceRect);
		
	if (region16_is_empty(&(subsystem->invalidRegion)))
	{

	}
			
	extents = region16_extents(&(subsystem->invalidRegion));

	x = extents->left;
	y = extents->top;
	width = extents->right - extents->left;
	height = extents->bottom - extents->top;
		
	IOSurfaceLock(frameSurface, kIOSurfaceLockReadOnly, NULL);
	
	pSrcData = (BYTE*) IOSurfaceGetBaseAddress(frameSurface);
	nSrcStep = (int) IOSurfaceGetBytesPerRow(frameSurface);

	if (subsystem->retina)
	{
		freerdp_image_copy_from_retina(surface->data, PIXEL_FORMAT_XRGB32, surface->scanline,
				       x, y, width, height, pSrcData, nSrcStep, x, y);
	}
	else
	{
		freerdp_image_copy(surface->data, PIXEL_FORMAT_XRGB32, surface->scanline,
					x, y, width, height, pSrcData, PIXEL_FORMAT_XRGB32, nSrcStep, x, y, NULL);
	}
	
	IOSurfaceUnlock(frameSurface, kIOSurfaceLockReadOnly, NULL);
		
	subsystem->updateReady = FALSE;
	dispatch_semaphore_signal(subsystem->dataSemaphore);
		
	ArrayList_Lock(server->clients);
		
	count = ArrayList_Count(server->clients);
		
	InitializeSynchronizationBarrier(&(subsystem->barrier), count + 1, -1);
		
	SetEvent(subsystem->updateEvent);
		
	EnterSynchronizationBarrier(&(subsystem->barrier), 0);
	ResetEvent(subsystem->updateEvent);
		
	DeleteSynchronizationBarrier(&(subsystem->barrier));
		
	ArrayList_Unlock(server->clients);
		
	region16_clear(&(subsystem->invalidRegion));
	
	if (status != kCGDisplayStreamFrameStatusFrameComplete)
	{
		switch (status)
		{
			case kCGDisplayStreamFrameStatusFrameIdle:
				break;
				
			case kCGDisplayStreamFrameStatusStopped:
				break;
				
			case kCGDisplayStreamFrameStatusFrameBlank:
				break;
				
			default:
				break;
		}
	}
	else if (!subsystem->lastUpdate)
	{
		CFRetain(updateRef);
		subsystem->lastUpdate = updateRef;
	}
	else
	{
		CGDisplayStreamUpdateRef tmpRef = subsystem->lastUpdate;
		subsystem->lastUpdate = CGDisplayStreamUpdateCreateMergedUpdate(tmpRef, updateRef);
		CFRelease(tmpRef);
	}
	
	dispatch_semaphore_signal(subsystem->regionSemaphore);
};

int mac_shadow_capture_init(macShadowSubsystem* subsystem)
{
	void* keys[2];
	void* values[2];
	CFDictionaryRef opts;
	CGDirectDisplayID displayId;
	
	displayId = CGMainDisplayID();
	
	subsystem->regionSemaphore = dispatch_semaphore_create(1);
	subsystem->dataSemaphore = dispatch_semaphore_create(1);
	
	subsystem->updateBuffer = (BYTE*) malloc(subsystem->pixelWidth * subsystem->pixelHeight * 4);
	
	subsystem->captureQueue = dispatch_queue_create("mac.shadow.capture", NULL);
	
	keys[0] = (void*) kCGDisplayStreamShowCursor;
	values[0] = (void*) kCFBooleanFalse;
	
	opts = CFDictionaryCreate(kCFAllocatorDefault, (const void**) keys, (const void**) values, 1, NULL, NULL);
	
	subsystem->stream = CGDisplayStreamCreateWithDispatchQueue(displayId, subsystem->pixelWidth, subsystem->pixelHeight,
							'BGRA', opts, subsystem->captureQueue, mac_capture_stream_handler);
	
	CFRelease(opts);
	
	return 1;
}

int mac_shadow_screen_grab(macShadowSubsystem* subsystem)
{
	mac_shadow_capture_get_dirty_region(subsystem);
	mac_shadow_capture_surface_copy(subsystem);
	
	return 1;
}

void* mac_shadow_subsystem_thread(macShadowSubsystem* subsystem)
{
	int fps;
	DWORD status;
	DWORD nCount;
	UINT64 cTime;
	DWORD dwTimeout;
	DWORD dwInterval;
	UINT64 frameTime;
	HANDLE events[32];
	HANDLE StopEvent;
	
	StopEvent = subsystem->server->StopEvent;
	
	nCount = 0;
	events[nCount++] = StopEvent;
	
	fps = 16;
	dwInterval = 1000 / fps;
	frameTime = GetTickCount64() + dwInterval;
	
	while (1)
	{
		cTime = GetTickCount64();
		dwTimeout = (cTime > frameTime) ? 0 : frameTime - cTime;
		
		status = WaitForMultipleObjects(nCount, events, FALSE, dwTimeout);
		
		if (WaitForSingleObject(StopEvent, 0) == WAIT_OBJECT_0)
		{
			break;
		}
		
		if ((status == WAIT_TIMEOUT) || (GetTickCount64() > frameTime))
		{
			mac_shadow_screen_grab(subsystem);
			
			dwInterval = 1000 / fps;
			frameTime += dwInterval;
		}
	}
	
	ExitThread(0);
	return NULL;
}

int mac_shadow_subsystem_init(macShadowSubsystem* subsystem)
{
	g_Subsystem = subsystem;
	
	mac_shadow_detect_monitors(subsystem);
	
	mac_shadow_capture_init(subsystem);
	
	return 1;
}

int mac_shadow_subsystem_uninit(macShadowSubsystem* subsystem)
{
	if (!subsystem)
		return -1;

	return 1;
}

int mac_shadow_subsystem_start(macShadowSubsystem* subsystem)
{
	HANDLE thread;

	if (!subsystem)
		return -1;

	mac_shadow_capture_start(subsystem);
	
	thread = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE) mac_shadow_subsystem_thread,
			(void*) subsystem, 0, NULL);

	return 1;
}

int mac_shadow_subsystem_stop(macShadowSubsystem* subsystem)
{
	if (!subsystem)
		return -1;

	return 1;
}

void mac_shadow_subsystem_free(macShadowSubsystem* subsystem)
{
	if (!subsystem)
		return;

	mac_shadow_subsystem_uninit(subsystem);

	free(subsystem);
}

macShadowSubsystem* mac_shadow_subsystem_new(rdpShadowServer* server)
{
	macShadowSubsystem* subsystem;

	subsystem = (macShadowSubsystem*) calloc(1, sizeof(macShadowSubsystem));

	if (!subsystem)
		return NULL;

	subsystem->server = server;
	
	subsystem->updateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	region16_init(&(subsystem->invalidRegion));

	subsystem->Init = (pfnShadowSubsystemInit) mac_shadow_subsystem_init;
	subsystem->Uninit = (pfnShadowSubsystemInit) mac_shadow_subsystem_uninit;
	subsystem->Start = (pfnShadowSubsystemStart) mac_shadow_subsystem_start;
	subsystem->Stop = (pfnShadowSubsystemStop) mac_shadow_subsystem_stop;
	subsystem->Free = (pfnShadowSubsystemFree) mac_shadow_subsystem_free;

	subsystem->SynchronizeEvent = (pfnShadowSynchronizeEvent) mac_shadow_input_synchronize_event;
	subsystem->KeyboardEvent = (pfnShadowKeyboardEvent) mac_shadow_input_keyboard_event;
	subsystem->UnicodeKeyboardEvent = (pfnShadowUnicodeKeyboardEvent) mac_shadow_input_unicode_keyboard_event;
	subsystem->MouseEvent = (pfnShadowMouseEvent) mac_shadow_input_mouse_event;
	subsystem->ExtendedMouseEvent = (pfnShadowExtendedMouseEvent) mac_shadow_input_extended_mouse_event;

	return subsystem;
}

rdpShadowSubsystem* Mac_ShadowCreateSubsystem(rdpShadowServer* server)
{
	return (rdpShadowSubsystem*) mac_shadow_subsystem_new(server);
}
