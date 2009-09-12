

#include <config.h>

#include <stdio.h>

#include <directfb.h>

#include <fusion/shmalloc.h>
#include <fusion/fusion.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/layers.h>
#include <core/palette.h>
#include <core/surface.h>
#include <core/system.h>

#include <gfx/convert.h>

#include <misc/conf.h>

#include <direct/memcpy.h>
#include <direct/messages.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "pb.h"
#include "primary.h"

#include <stdio.h>

//extern DFBPB  *dfb_pb;
//extern CoreDFB *dfb_pb_core;
/******************************************************************************/

static DFBResult dfb_pb_set_video_mode( CoreDFB *core, CoreLayerRegionConfig *config );
static DFBResult dfb_pb_update_screen( CoreDFB *core, DFBRegion *region );
static DFBResult dfb_pb_set_palette( CorePalette *palette );

static DFBResult update_screen( CoreSurface *surface,
                                int x, int y, int w, int h );

//static GdkPixbuf            *screen = NULL;
/******************************************************************************/

static DFBResult
primaryInitScreen( CoreScreen           *screen,
                   CoreGraphicsDevice       *device,
                   void                 *driver_data,
                   void                 *screen_data,
                   DFBScreenDescription *description )
{
     /* Set the screen capabilities. */
     description->caps = DSCCAPS_NONE;

     /* Set the screen name. */
     snprintf( description->name,
               DFB_SCREEN_DESC_NAME_LENGTH, "PB Primary Screen" );

     return DFB_OK;
}

static DFBResult
primaryGetScreenSize( CoreScreen *screen,
                      void       *driver_data,
                      void       *screen_data,
                      int        *ret_width,
                      int        *ret_height )
{
     D_ASSERT( dfb_pb != NULL );

     if (dfb_pb->primary) {
          //*ret_width  = dfb_pb->primary->width;
          *ret_width  = dfb_pb->primary->config.size.w;
          //*ret_height = dfb_pb->primary->height;
          *ret_height = dfb_pb->primary->config.size.h;
     }
     else {
          if (dfb_config->mode.width)
               *ret_width  = dfb_config->mode.width;
          else
               *ret_width  = 320;

          if (dfb_config->mode.height)
               *ret_height = dfb_config->mode.height;
          else
               *ret_height = 480;
     }

     return DFB_OK;
}

ScreenFuncs pbPrimaryScreenFuncs = {
     .InitScreen    = primaryInitScreen,
     .GetScreenSize = primaryGetScreenSize,
};

/******************************************************************************/

static void ScreenUpdateLoop( DirectThread *thread, void *arg )
{
	gdk_threads_enter();

//	gtk_init(NULL, NULL);

	

	gtk_main();
	gdk_threads_leave();
}

static int
primaryLayerDataSize( void )
{
     return 0;
}

static int
primaryRegionDataSize( void )
{
     return 0;
}

static gboolean
window_expose_event (GtkWidget      *widget,
                     GdkEventExpose *event)
{
	printf("expose event GdkWindow ptr =%p\n", widget->window);
	dfb_pb->container = widget->window;
}


static DFBResult
primaryInitLayer( CoreLayer                  *layer,
                  void                       *driver_data,
                  void                       *layer_data,
                  DFBDisplayLayerDescription *description,
                  DFBDisplayLayerConfig      *config,
                  DFBColorAdjustment         *adjustment )
{
     /* set capabilities and type */
     description->caps = DLCAPS_SURFACE;
     description->type = DLTF_GRAPHICS;

     /* set name */
     snprintf( description->name,
               DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "PB Primary Layer" );

     /* fill out the default configuration */
     config->flags       = DLCONF_WIDTH       | DLCONF_HEIGHT |
                           DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE;
     config->buffermode  = DLBM_FRONTONLY;

     if (dfb_config->mode.width)
          config->width  = dfb_config->mode.width;
     else
          config->width  = 320;

     if (dfb_config->mode.height)
          config->height = dfb_config->mode.height;
     else
          config->height = 480;

     if (dfb_config->mode.format != DSPF_UNKNOWN)
          config->pixelformat = dfb_config->mode.format;
     else if (dfb_config->mode.depth > 0)
          config->pixelformat = dfb_pixelformat_for_depth( dfb_config->mode.depth );
     else
          config->pixelformat = DSPF_ARGB;

	dfb_pb->container = NULL;
	g_thread_init(NULL);
	gdk_threads_init();
	gdk_threads_enter();

	gtk_init(NULL, NULL);
	
	GtkWidget *desktop = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size((GtkWindow*)desktop, 320, 480);
	//dfb_pb->container = desktop->window;
	g_signal_connect(desktop, "expose-event",
                          G_CALLBACK (window_expose_event), NULL);

	gtk_widget_show_all(desktop);
	
	gdk_threads_leave();


	 direct_thread_create( DTT_OUTPUT, ScreenUpdateLoop, NULL, "Screen Update" );
	 	

     return DFB_OK;
}

static DFBResult
primaryTestRegion( CoreLayer                  *layer,
                   void                       *driver_data,
                   void                       *layer_data,
                   CoreLayerRegionConfig      *config,
                   CoreLayerRegionConfigFlags *failed )
{
     CoreLayerRegionConfigFlags fail = 0;

     switch (config->buffermode) {
          case DLBM_FRONTONLY:
          case DLBM_BACKSYSTEM:
          case DLBM_BACKVIDEO:
               break;

          default:
               fail |= CLRCF_BUFFERMODE;
               break;
     }

     if (config->options)
          fail |= CLRCF_OPTIONS;

     if (failed)
          *failed = fail;

     if (fail)
          return DFB_UNSUPPORTED;

     return DFB_OK;
}

static DFBResult
primaryAddRegion( CoreLayer             *layer,
                  void                  *driver_data,
                  void                  *layer_data,
                  void                  *region_data,
                  CoreLayerRegionConfig *config )
{
     return DFB_OK;
}

static DFBResult
primarySetRegion( CoreLayer                  *layer,
                  void                       *driver_data,
                  void                       *layer_data,
                  void                       *region_data,
                  CoreLayerRegionConfig      *config,
                  CoreLayerRegionConfigFlags  updated,
                  CoreSurface                *surface,
                  CorePalette                *palette,
				  CoreSurfaceBufferLock      *lock)
{
     DFBResult ret;

     if (surface)
        	dfb_pb->primary = surface;

     ret = dfb_pb_set_video_mode( dfb_pb_core, config );
     if (ret)
          return ret;

     if (palette)
          dfb_pb_set_palette( palette );

     return DFB_OK;
}

static DFBResult
primaryRemoveRegion( CoreLayer             *layer,
                     void                  *driver_data,
                     void                  *layer_data,
                     void                  *region_data )
{
     dfb_pb->primary = NULL;

     return DFB_OK;
}

static DFBResult
primaryFlipRegion( CoreLayer           *layer,
                   void                *driver_data,
                   void                *layer_data,
                   void                *region_data,
                   CoreSurface         *surface,
                   DFBSurfaceFlipFlags  flags,
				   CoreSurfaceBufferLock      *lock)
{
     //dfb_surface_flip_buffers( surface, false );
     dfb_surface_flip( surface, false );

     return dfb_pb_update_screen( dfb_pb_core, NULL );
}

static DFBResult
primaryUpdateRegion( CoreLayer           *layer,
                     void                *driver_data,
                     void                *layer_data,
                     void                *region_data,
                     CoreSurface         *surface,
                     const DFBRegion     *update,
					 CoreSurfaceBufferLock      *lock)
{

     if (surface && (surface->config.caps & DSCAPS_FLIPPING)) {
          if (update) {
               DFBRegion region = *update;

               return dfb_pb_update_screen( dfb_pb_core, &region );
          }
          else
               return dfb_pb_update_screen( dfb_pb_core, NULL );
     }

     return DFB_OK;
}


DisplayLayerFuncs pbPrimaryLayerFuncs = {
     .LayerDataSize     = primaryLayerDataSize,
     .RegionDataSize    = primaryRegionDataSize,
     .InitLayer         = primaryInitLayer,

     .TestRegion        = primaryTestRegion,
     .AddRegion         = primaryAddRegion,
     .SetRegion         = primarySetRegion,
     .RemoveRegion      = primaryRemoveRegion,
     .FlipRegion        = primaryFlipRegion,
     .UpdateRegion      = primaryUpdateRegion,

     //.AllocateSurface   = primaryAllocateSurface,
     //.ReallocateSurface = primaryReallocateSurface,
};

/******************************************************************************/

static DFBResult
update_screen( CoreSurface *surface, int x, int y, int w, int h )
{

     DFBResult    ret;

     D_ASSERT( surface != NULL );
	 
	 //TODO: Draw the dfb surface in GdkWindow(our screen)
	 //dfb_surface_calc_buffer_size( surface, 8, 0, &pitch, &alloc->size );
	 //
	 if (dfb_pb->container) {
	 CoreSurfaceBufferLock  lock; 
	 dfb_surface_lock_buffer(surface, CSBR_FRONT, CSAF_GPU_READ, &lock);
	 
	 GdkPixbuf*     pixbuf = gdk_pixbuf_new_from_data((guchar *)(lock.addr), GDK_COLORSPACE_RGB,
                                                         TRUE, 8,
                                                         surface->config.size.w, surface->config.size.h,
                                                         lock.pitch, NULL, NULL);
	 dfb_surface_unlock_buffer(surface, &lock);														

	 //wain for the gdkwindow to be created
     
	 gdk_threads_enter();

	 gdk_draw_pixbuf(dfb_pb->container, NULL, pixbuf,
					0, 0,
					0, 0,
					surface->config.size.w, surface->config.size.h,
					GDK_RGB_DITHER_NONE, 0, 0);	

	 gdk_threads_leave();											 
	 }
	 
	 return DFB_OK;
}

/******************************************************************************/

typedef enum {
     PB_SET_VIDEO_MODE,
     PB_UPDATE_SCREEN,
     PB_SET_PALETTE
} DFBPBCall;

DFBResult
dfb_pb_set_video_mode_handler( CoreLayerRegionConfig *config )
//dfb_pb_set_video_mode_handler( int depth, int w, int h )
{
     /*fusion_skirmish_prevail( &dfb_pb->lock );

	 //FIXME: Free screen while not NULL
	 dfb_pb->primary = 
	 dfb_pb->screen = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 
	 								 8, config->width, config->height);
	 //dfb_pb->screen = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, w, h); 
	 if (config->buffermode == DLBM_FRONTONLY) {	
	       //update primary surface information
          //dfb_pb->primary->front_buffer->system.addr = (void *)gdk_pixbuf_get_pixels( screen );
          dfb_pb->primary->config.preallocated[0].addr = (void *)gdk_pixbuf_get_pixels( dfb_pb->screen );
          //dfb_pb->primary->front_buffer->system.pitch = gdk_pixbuf_get_rowstride( screen );	
          dfb_pb->primary->config.preallocated[0].pitch = gdk_pixbuf_get_rowstride( dfb_pb->screen );	
     }

     fusion_skirmish_dismiss( &dfb_pb->lock );*/

     return DFB_OK;
}

static DFBResult
dfb_pb_update_screen_handler( DFBRegion *region )
{
     DFBResult    ret;
     CoreSurface *surface = dfb_pb->primary;

     fusion_skirmish_prevail( &dfb_pb->lock );

     if (!region)
          ret = update_screen( surface, 0, 0, surface->config.size.w, surface->config.size.h );
     else
          ret = update_screen( surface,
                               region->x1,  region->y1,
                               region->x2 - region->x1 + 1,
                               region->y2 - region->y1 + 1 );

     fusion_skirmish_dismiss( &dfb_pb->lock );

     return DFB_OK;
}

static DFBResult
dfb_pb_set_palette_handler( CorePalette *palette )
{
     fusion_skirmish_prevail( &dfb_pb->lock );

// do something usefull
     fusion_skirmish_dismiss( &dfb_pb->lock );

     return DFB_OK;
}

FusionCallHandlerResult
dfb_pb_call_handler( int           caller,
                      int           call_arg,
                      void         *call_ptr,
                      void         *ctx,
                      unsigned int  serial,
                      int          *ret_val )
{
     switch (call_arg) {
          case PB_SET_VIDEO_MODE:
               *ret_val = dfb_pb_set_video_mode_handler( call_ptr );
               break;

          case PB_UPDATE_SCREEN:
               *ret_val = dfb_pb_update_screen_handler( call_ptr );
               break;

          case PB_SET_PALETTE:
               *ret_val = dfb_pb_set_palette_handler( call_ptr );
               break;

          default:
               D_BUG( "unknown call" );
               *ret_val = DFB_BUG;
               break;
     }

     return FCHR_RETURN;
}

static DFBResult
dfb_pb_set_video_mode( CoreDFB *core, CoreLayerRegionConfig *config )
{
     int                    ret;
     CoreLayerRegionConfig *tmp = NULL;

     D_ASSERT( config != NULL );

     if (dfb_core_is_master( core ))
          return dfb_pb_set_video_mode_handler( config );

     if (!fusion_is_shared( dfb_core_world(core), config )) {
          tmp = SHMALLOC( dfb_core_shmpool(dfb_pb_core), sizeof(CoreLayerRegionConfig) );
          if (!tmp)
               return D_OOSHM();

          direct_memcpy( tmp, config, sizeof(CoreLayerRegionConfig) );
     }

     fusion_call_execute( &dfb_pb->call, FCEF_NONE, PB_SET_VIDEO_MODE,
                          tmp ? tmp : config, &ret );

     if (tmp)
          SHFREE( dfb_core_shmpool(dfb_pb_core), tmp );

     return ret;
}

static DFBResult
dfb_pb_update_screen( CoreDFB *core, DFBRegion *region )
{
     int        ret;
     DFBRegion *tmp = NULL;

     if (dfb_core_is_master( core ))
          return dfb_pb_update_screen_handler( region );

     if (region) {
          if (!fusion_is_shared( dfb_core_world(core), region )) {
               tmp = SHMALLOC( dfb_core_shmpool(dfb_pb_core), sizeof(DFBRegion) );
               if (!tmp)
                    return D_OOSHM();

               direct_memcpy( tmp, region, sizeof(DFBRegion) );
          }
     }

     fusion_call_execute( &dfb_pb->call, FCEF_ONEWAY, PB_UPDATE_SCREEN,
                          tmp ? tmp : region, &ret );

     if (tmp)
          SHFREE( dfb_core_shmpool(dfb_pb_core), tmp );

     return ret;
}

static DFBResult
dfb_pb_set_palette( CorePalette *palette )
{
     int ret;

     fusion_call_execute( &dfb_pb->call, FCEF_NONE, PB_SET_PALETTE,
                          palette, &ret );

     return ret;
}

