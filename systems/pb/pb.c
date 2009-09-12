
#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <directfb.h>

#include <fusion/arena.h>
#include <fusion/shmalloc.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/layers.h>
#include <core/palette.h>
#include <core/surface.h>
#include <core/system.h>

#include <gfx/convert.h>

#include <misc/conf.h>

#include <direct/messages.h>

#include "pb.h"
#include "primary.h"

#include <core/core_system.h>

DFB_CORE_SYSTEM( pb )

DFBPB  *dfb_pb      = NULL;
CoreDFB *dfb_pb_core = NULL;

extern const SurfacePoolFuncs pbSurfacePoolFuncs;

static void
system_get_info( CoreSystemInfo *info )
{
     info->type = CORE_PB;
	 info->caps = CSCAPS_NONE;

     snprintf( info->name, DFB_CORE_SYSTEM_INFO_NAME_LENGTH, "PB" );
}

static DFBResult
system_initialize( CoreDFB *core, void **data )
{
     char       *driver;
     CoreScreen *screen;

     D_ASSERT( dfb_pb == NULL );

	 dfb_pb_core = core;	

     dfb_pb = (DFBPB*) SHCALLOC( dfb_core_shmpool(dfb_pb_core), 1, sizeof(DFBPB) );
     if (!dfb_pb) {
          D_ERROR( "DirectFB/PB: Couldn't allocate shared memory!\n" );
          return D_OOSHM();
     }

     
     /* Initialize PB */
//	 PB_Init(dfb_pb->container);
//	 gtk_init(NULL, NULL);

	 
     fusion_skirmish_init( &dfb_pb->lock, "PB System", dfb_core_world(core) );

     fusion_call_init( &dfb_pb->call, dfb_pb_call_handler, NULL, dfb_core_world(core) );

     screen = dfb_screens_register( NULL, NULL, &pbPrimaryScreenFuncs );

     dfb_layers_register( screen, NULL, &pbPrimaryLayerFuncs );

     dfb_surface_pool_initialize( core, &pbSurfacePoolFuncs, &dfb_pb->pb_pool );	 

     fusion_arena_add_shared_field( dfb_core_arena( core ), "PB", dfb_pb );

     *data = dfb_pb;

     return DFB_OK;
}

static DFBResult
system_join( CoreDFB *core, void **data )
{
     void       *ret;
     CoreScreen *screen;

     D_ASSERT( dfb_pb == NULL );

     fusion_arena_get_shared_field( dfb_core_arena( core ), "PB", &ret );

     dfb_pb = ret;
     dfb_pb_core = core;

     screen = dfb_screens_register( NULL, NULL, &pbPrimaryScreenFuncs );

     dfb_layers_register( screen, NULL, &pbPrimaryLayerFuncs );

     *data = dfb_pb;

     return DFB_OK;
}

static DFBResult
system_shutdown( bool emergency )
{
     D_ASSERT( dfb_pb != NULL );

     fusion_call_destroy( &dfb_pb->call );

     fusion_skirmish_prevail( &dfb_pb->lock );

     fusion_skirmish_destroy( &dfb_pb->lock );

     SHFREE( dfb_core_shmpool(dfb_pb_core), dfb_pb );
     dfb_pb = NULL;
     dfb_pb_core = NULL;

     return DFB_OK;
}

static DFBResult
system_leave( bool emergency )
{
     D_ASSERT( dfb_pb != NULL );

     dfb_pb = NULL;
     dfb_pb_core = NULL;

     return DFB_OK;
}

static DFBResult
system_suspend( void )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
system_resume( void )
{
     return DFB_UNIMPLEMENTED;
}

static volatile void *
system_map_mmio( unsigned int    offset,
                 int             length )
{
    return NULL;
}

static void
system_unmap_mmio( volatile void  *addr,
                   int             length )
{
}

static int
system_get_accelerator( void )
{
     return -1;
}

static VideoMode *
system_get_modes( void )
{
     return NULL;
}

static VideoMode *
system_get_current_mode( void )
{
     return NULL;
}

static DFBResult
system_thread_init( void )
{
     return DFB_OK;
}

static bool
system_input_filter( CoreInputDevice   *device,
                     DFBInputEvent *event )
{
     return false;
}

static unsigned long
system_video_memory_physical( unsigned int offset )
{
     return 0;
}

static void *
system_video_memory_virtual( unsigned int offset )
{
     return NULL;
}

static unsigned int
system_videoram_length( void )
{
     return 0;
}

static unsigned long
system_aux_memory_physical( unsigned int offset )
{
     return 0;
}

static void *
system_aux_memory_virtual( unsigned int offset )
{
     return NULL;
}

static unsigned int
system_auxram_length( void )
{
     return 0;
}

static void
system_get_busid( int *ret_bus, int *ret_dev, int *ret_func )
{
     return;
}

static void
system_get_deviceid( unsigned int *ret_vendor_id,
                     unsigned int *ret_device_id )
{
     return;
}

