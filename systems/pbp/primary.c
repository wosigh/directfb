

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

/*#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>*/
#include <pthread.h>
#include <time.h>

#include "pbp.h"
#include "primary.h"
#include "primary_mem.h"

#include <sys/types.h>            
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#include "event.h"


static QUIET = 1;

void logTimeStamp(FILE *out, int line)
{
	struct tm *t;
	time_t now;

	if (!out)
		return;
	time(&now);
	t = localtime(&now);

	fprintf(out, "%ld %.4d-%.2d-%.2d %.2d:%.2d:%.2d	%d	plugin: ", (long)now,
		t->tm_year + 1900,
		t->tm_mon + 1,
		t->tm_mday,
		t->tm_hour,
		t->tm_min,
		t->tm_sec, line);
}


FILE *log = NULL;

#define myprintf(fmt...) 	    do {					\
									if (!QUIET) { 		\
										if (!log) \
											log = fopen("/tmp/dfbadapter-dfb.log", "w");\
										logTimeStamp(log, __LINE__); \
										fprintf(log, fmt); 	\
									}					\
								} while (0)


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
               DFB_SCREEN_DESC_NAME_LENGTH, "PBP Primary Screen" );

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
static long draw_serial = 0;


static void PluginEventListener(DirectThread *thread, void *arg)
{
	 
     struct sockaddr_un  addr;
	 socklen_t           addr_len = sizeof(addr);

	 dfb_pb->dfb_socket_fd = socket( PF_LOCAL, SOCK_RAW, 0 );
	 
	 if (dfb_pb->dfb_socket_fd < 0) {
		 perror ("Error creating local socket!\n" );
	 }

	 addr.sun_family = AF_UNIX;
	 snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", dfb_pb->dfb_socket_name);
	 
	 unlink(dfb_pb->dfb_socket_name);
	 int err = bind(dfb_pb->dfb_socket_fd, (struct sockaddr*)&addr, sizeof(addr));
	 D_ASSERT(err == 0);

	 fd_set set;
	 char buf[1024];

	 while (true) {
          int result;
		            
          FD_ZERO( &set );
          FD_SET( dfb_pb->dfb_socket_fd, &set );


		  myprintf("dfb:		 run select on dfb socket...\n");
          result = select( dfb_pb->dfb_socket_fd + 1, &set, NULL, NULL, NULL );
          if (result < 0) {
               switch (errno) {
                    case EINTR:
                         continue;

                    default:
                         D_PERROR( "PluginEventListener: select() failed!\n" );
					return;	 
               }
          }

          if (FD_ISSET( dfb_pb->dfb_socket_fd, &set ) && 
              recvfrom( dfb_pb->dfb_socket_fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len ) > 0) {
               
			   PluginEvent *e = (PluginEvent*)buf;               

               myprintf("dfb:		 -> event from '%s'...\n", addr.sun_path );

               switch (e->type) {
			   		case PE_DrawResponse:
						draw_serial = e->ePluginDrawResponse.serial;
						pthread_mutex_lock(&dfb_pb->draw_mutex);
						pthread_cond_signal(&dfb_pb->drawn_cond);
						pthread_mutex_unlock(&dfb_pb->draw_mutex);
					 myprintf("dfb:		 signal to wait update_screen\n");
						break;
					default:
                        D_BUG( "unexpected message type (%d)", e->type );
                        break;	

			   }
          }			   
	 }
	
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


static DFBResult
primaryInitLayer( CoreLayer                  *layer,
                  void                       *driver_data,
                  void                       *layer_data,
                  DFBDisplayLayerDescription *description,
                  DFBDisplayLayerConfig      *config,
                  DFBColorAdjustment         *adjustment )
{
     printf("dfb:		come to primaryInitLayer()\n");
	 /* set capabilities and type */
     description->caps = DLCAPS_SURFACE;
     description->type = DLTF_GRAPHICS;

     /* set name */
     snprintf( description->name,
               DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "PBP Primary Layer" );

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


	 DFBResult ret = pb_primarymemmanager_create(dfb_pb->dfb_primarymem_name, &dfb_pb->primary_mem_mgr);
	 D_ASSERT(ret == DFB_OK);
	 	
	 pthread_mutex_init(&dfb_pb->draw_mutex,NULL);
	 pthread_cond_init(&dfb_pb->drawn_cond,NULL);

	 direct_thread_create( DTT_OUTPUT, PluginEventListener, NULL, "Plugin Event Listener" );

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
	 static long update_serial = 0;

	 update_serial++;

	 myprintf("dfb:		come to %s\n", __FUNCTION__);

     DFBResult    ret;

     D_ASSERT( surface != NULL );
	 

	 CoreSurfaceBufferLock  lock; 
	 dfb_surface_lock_buffer(surface, CSBR_FRONT, CSAF_GPU_READ, &lock);

	 /* send draw event to plugin */
	 DFBDrawRequest		 draw_request;
	 DFBPBPEvent			 e;

	 draw_request.type   = DFE_DrawRequest;
	 //TODO:
	 draw_request.serial = update_serial;
	 draw_request.offset = lock.addr - dfb_pb->primary_mem_mgr->v_addr;
	 draw_request.w		 = surface->config.size.w;
	 draw_request.h		 = surface->config.size.h;
	 draw_request.pitch  = lock.pitch;

	 e.eDFBDrawRequest   = draw_request;

	/*//for test
	//memset(dfb_pb->primary_mem_mgr->v_addr+draw_request.offset, 0x80, surface->config.size.h*draw_request.pitch);
	unsigned long *src = (unsigned long*)dfb_pb->primary_mem_mgr->v_addr + draw_request.offset;
	int i;
	for (i = 0; i<surface->config.size.h*surface->config.size.w; i++)
		src[i] = 0xFFFF0000;*/


	 dfb_surface_unlock_buffer(surface, &lock);	


		
	 struct sockaddr_un  addr;
	 socklen_t           addr_len = sizeof(addr);

	 addr.sun_family = AF_UNIX;
	 snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", dfb_pb->plugin_socket_name);

send:
	 while (sendto( dfb_pb->dfb_socket_fd, &e, sizeof(DFBPBPEvent), 0, (struct sockaddr*)&addr, addr_len ) < 0) {
          switch (errno) {
               case EINTR:
			   	 myprintf("dfb: 		sendto errno:EINTR\n");
                    continue;
               case ECONNREFUSED:
			   	 myprintf("dfb:		sendto errno:ECONNREFUSEED\n");
			   		continue;
                    //return DR_FUSION;
               default:
                    break;
          }

          D_PERROR( "sendto()\n" );

          return DFB_IO;
     }
	 
	 myprintf("dfb:		draw event sent to plugin\n");

/*#define WAIT_DRAW_RES_TIMEOUT_SECS 1
#define WAIT_DRAW_RES_TIMEOUT_NSECS 3000
	 struct timeval now;
     struct timespec timeout;
     int retcode;*/

     pthread_mutex_lock(&dfb_pb->draw_mutex);
     
	 /*gettimeofday(&now, NULL);

	 if ( now.tv_usec * 1000 + WAIT_DRAW_RES_TIMEOUT_NSECS >= 1000000 ) {
     	 timeout.tv_sec = now.tv_sec + WAIT_DRAW_RES_TIMEOUT_SECS + 1;
     	 timeout.tv_nsec = now.tv_usec * 1000 + WAIT_DRAW_RES_TIMEOUT_NSECS - 1000000;	 	 
	 }
	 else {
     	 timeout.tv_sec = now.tv_sec + WAIT_DRAW_RES_TIMEOUT_SECS;
     	 timeout.tv_nsec = now.tv_usec * 1000 + WAIT_DRAW_RES_TIMEOUT_NSECS;
	 }	 
     retcode = 0;
	 
	 retcode = pthread_cond_timedwait(&dfb_pb->drawn_cond, &dfb_pb->draw_mutex, &timeout);
	 if (retcode == ETIMEDOUT) {
		 pthread_mutex_unlock(&dfb_pb->draw_mutex);
		 myprintf("dfb:		wait on drawn_cond timeout\n");
		 goto send;
	 }*/
	 while (update_serial != draw_serial)
	 	 pthread_cond_wait(&dfb_pb->drawn_cond, &dfb_pb->draw_mutex);

	 pthread_mutex_unlock(&dfb_pb->draw_mutex);
	 
	 myprintf("dfb:		leave %s\n", __FUNCTION__);
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

