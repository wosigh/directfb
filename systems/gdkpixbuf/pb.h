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

     struct {
          pthread_mutex_t  lock;
          pthread_cond_t   cond;

          DirectThread    *thread;

          bool             pending;
          DFBRegion        region;

          bool             quit;
     } update;

     VideoMode            *modes;        /* linked list of valid video modes */

     GdkWindow            *container;    /* fake physical screen */
     GdkPixbuf            *screen;
} DFBPB;

extern DFBPB  *dfb_pb;
extern CoreDFB *dfb_pb_core;

DFBResult dfb_pb_set_video_mode( CoreDFB *core, CoreSurfaceConfig *config );
DFBResult dfb_pb_update_screen( CoreDFB *core, DFBRegion *region );
DFBResult dfb_pb_set_palette( CorePalette *palette );

#endif

