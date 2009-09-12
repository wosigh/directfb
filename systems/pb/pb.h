#ifndef __PB__PB_H__
#define __PB__PB_H__

#include <fusion/call.h>
#include <fusion/lock.h>

#include <core/surface_pool.h>
#include <core/system.h>

typedef struct {
     FusionSkirmish   lock;
     FusionCall       call;

     CoreSurface     *primary;
     CoreSurfacePool *pb_pool;

     VideoMode            *modes;        /* linked list of valid video modes */

     GdkWindow            *container;    /* fake physical screen */
     GdkPixbuf            *screen;
} DFBPB;

extern DFBPB  *dfb_pb;
extern CoreDFB *dfb_pb_core;



#endif

