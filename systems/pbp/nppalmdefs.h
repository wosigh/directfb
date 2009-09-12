/* ============================================================
 * Date  : 2008-01-17
 * Copyright 2008 Palm, Inc. All rights reserved.
 * ============================================================ */

#ifndef NPPPALMDEFS_H
#define NPPPALMDEFS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NpPalmValuesEnum
{
    npPalmEventLoopValue = 10000,

	/**
	 * Used for the plugin to inform WebKit that it wants to be cached which means
	 * it won't be deleted as it would normally be during significant style changes
	 * (like hiding the parent div element). The return value is ignored if the
	 * object element specifies the 'x-palm-cache-plugin' true/false attribute.
	 */
	npPalmCachePluginValue = 10001,

	/**
	 * Used to determine the application identifer in the context of which this
	 * plugin is created
	 */
	npPalmApplicationIdentifier = 10002
} NpPalmValuesEnum;

typedef enum NpPalmEventsEnum
{
    npPalmNullEvent,
    npPalmPenDownEvent     = 1 << 0,          
    npPalmPenUpEvent       = 1 << 1,
    npPalmPenMoveEvent     = 1 << 2,          
    npPalmKeyDownEvent     = 1 << 3,
    npPalmKeyUpEvent       = 1 << 4,
    npPalmKeyRepeatEvent   = 1 << 5,
    npPalmKeyPressEvent    = 1 << 6,
    npPalmDrawEvent        = 1 << 7
} NpPalmEventsEnum;

typedef struct NpPalmKeyEvent
{
    int32_t chr;        // 32-bits to allow any Unicode character
    int32_t modifiers;  // flag to be defined somewhere! FIXME
	int32_t rawkeyCode; // Palm : to send raw events to browserserver
	int32_t rawModifier; // Palm : to send raw events to browserserver
} NpPalmKeyEvent;


typedef struct NpPalmPenEvent
{
    int32_t xCoord, yCoord; // position of pen
    int32_t modifiers;      // flag to be defined somewhere! FIXME
} NpPalmPenEvent;

typedef struct NpPalmDrawEvent
{
    void*    dstBuffer;     // destination paint buffer (points to top-left of area to paint)
    uint32_t dstRowBytes;   // Stride in bytes for paint buffer
    int32_t  srcLeft;       // left  edge of box to draw from in unscaled coords
    int32_t  srcRight;      // right edge of box to draw from in unscaled coords
    int32_t  srcTop;        // top edge of box to draw from in unscaled coords
    int32_t  srcBottom;     // bottom edge of box to draw from in unscaled coords
    void*    graphicsContext; // pointer to the PGContext which plugin can use for drawing
    int32_t  dstLeft;
    int32_t  dstRight;
    int32_t  dstTop;
    int32_t  dstBottom;
} NpPalmDrawEvent;

typedef struct NpPalmEventType 
{
    NpPalmEventsEnum eventType;

    union {
        NpPalmKeyEvent     keyEvent;
        NpPalmPenEvent     penEvent;
        NpPalmDrawEvent    drawEvent;
    } data;

} NpPalmEventType;

typedef struct NpPalmWindow
{
    bool      visible;     // Whether window is visible on screen or not
    uint32_t  bpp;         // Bits-Per-Pixel for paint surfaces
    double    scaleFactor; // scale factor to use for painting
} NpPalmWindow;

#ifdef __cplusplus
}
#endif

#endif /* NPPPALMDEFS_H */
