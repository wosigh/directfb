/*
   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

//#include <PB.h>

#include <direct/debug.h>
#include <direct/mem.h>

#include <core/surface_pool.h>

#include <gfx/convert.h>

#include <directfb_util.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "pb.h"
#include "primary.h"

D_DEBUG_DOMAIN( PB_Pool, "PB/Pool", "PB Surface Pool" );

/**********************************************************************************************************************/

typedef struct {
} PBPoolData;

typedef struct {
     int          magic;

     //PB_Surface *pb_surf;
	 GdkPixbuf  *pb_surf;
} PBAllocationData;

/**********************************************************************************************************************/

static int
pbPoolDataSize( void )
{
     return sizeof(PBPoolData);
}

static int
pbAllocationDataSize( void )
{
     return sizeof(PBAllocationData);
}

static DFBResult
pbInitPool( CoreDFB                    *core,
             CoreSurfacePool            *pool,
             void                       *pool_data,
             void                       *pool_local,
             void                       *system_data,
             CoreSurfacePoolDescription *ret_desc )
{
     D_DEBUG_AT( PB_Pool, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_ASSERT( ret_desc != NULL );

     ret_desc->caps     = CSPCAPS_NONE;
     ret_desc->access   = CSAF_CPU_READ | CSAF_CPU_WRITE | CSAF_GPU_READ | CSAF_GPU_WRITE;
     ret_desc->types    = CSTF_LAYER | CSTF_WINDOW | CSTF_CURSOR | CSTF_FONT | CSTF_SHARED | CSTF_EXTERNAL;
     ret_desc->priority = CSPP_PREFERED;

     snprintf( ret_desc->name, DFB_SURFACE_POOL_DESC_NAME_LENGTH, "PB" );

     return DFB_OK;
}

static DFBResult
pbDestroyPool( CoreSurfacePool *pool,
                void            *pool_data,
                void            *pool_local )
{
     D_DEBUG_AT( PB_Pool, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );

     return DFB_OK;
}

static DFBResult
pbTestConfig( CoreSurfacePool         *pool,
               void                    *pool_data,
               void                    *pool_local,
               CoreSurfaceBuffer       *buffer,
               const CoreSurfaceConfig *config )
{
     D_DEBUG_AT( PB_Pool, "%s()\n", __FUNCTION__ );

     switch (config->format) {
          /*case DSPF_A8:
          case DSPF_RGB16:
          case DSPF_RGB32:*/
          case DSPF_ARGB:
               break;

          default:
               return DFB_UNSUPPORTED;
     }

     return DFB_OK;
}

static DFBResult
pbAllocateBuffer( CoreSurfacePool       *pool,
                   void                  *pool_data,
                   void                  *pool_local,
                   CoreSurfaceBuffer     *buffer,
                   CoreSurfaceAllocation *allocation,
                   void                  *alloc_data )
{
     DFBResult          ret;
     CoreSurface       *surface;
     PBAllocationData *alloc = alloc_data;

     D_DEBUG_AT( PB_Pool, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     if (surface->type & CSTF_LAYER) {
          dfb_pb->screen = NULL; /* clear? */

          /*ret = dfb_pb_set_video_mode_handler( DFB_BITS_PER_PIXEL( surface->config.format),  
		  								surface->config.size.w,  surface->config.size.h);*/
		  
		  fusion_skirmish_prevail( &dfb_pb->lock );

	      //FIXME: Free screen while not NULL
	 	  dfb_pb->screen = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 
	 								 8, surface->config.size.w,  surface->config.size.h);
	      //if (config->buffermode == DLBM_FRONTONLY) {	
	       //update primary surface information
          //dfb_pb->primary->front_buffer->system.addr = (void *)gdk_pixbuf_get_pixels( screen );
          //  dfb_pb->primary->config.preallocated[0].addr = (void *)gdk_pixbuf_get_pixels( dfb_pb->screen );
          //dfb_pb->primary->front_buffer->system.pitch = gdk_pixbuf_get_rowstride( screen );	
          //	dfb_pb->primary->config.preallocated[0].pitch = gdk_pixbuf_get_rowstride( dfb_pb->screen );	
          //}

         fusion_skirmish_dismiss( &dfb_pb->lock );		

          /*if (ret) {
               D_DERROR( ret, "PB/Surface: dfb_pb_set_video_mode() failed!\n" );
               return ret;
          }*/

          D_ASSERT( dfb_pb->screen != NULL );

          if (!dfb_pb->screen) {
               D_ERROR( "PB/Surface: No screen surface!?\n" );
               return DFB_BUG;
          }

          alloc->pb_surf = dfb_pb->screen;

          //D_DEBUG_AT( PB_Pool, "  -> screen surface  %dx%d, %d, 0x%08x, pitch %d\n",
          //            dfb_pb->screen->w, dfb_pb->screen->h, dfb_pb->screen->format->BitsPerPixel,
          //            dfb_pb->screen->flags, dfb_pb->screen->pitch );

          allocation->flags |= CSALF_ONEFORALL;
     }
     else {
          DFBSurfacePixelFormat  format = surface->config.format;
          /*Uint32                 flags  = PB_HWSURFACE;// | PB_ASYNCBLIT | PB_FULLSCREEN;
          Uint32                 rmask;
          Uint32                 gmask;
          Uint32                 bmask;
          Uint32                 amask;

          if (surface->config.caps & DSCAPS_FLIPPING)
               flags |= PB_DOUBLEBUF;

          switch (format) {
               case DSPF_A8:
                    rmask = 0x00;
                    gmask = 0x00;
                    bmask = 0x00;
                    amask = 0xff;
                    break;

               case DSPF_RGB16:
                    rmask = 0xf800;
                    gmask = 0x07e0;
                    bmask = 0x001f;
                    amask = 0x0000;
                    break;

               case DSPF_RGB32:
                    rmask = 0x00ff0000;
                    gmask = 0x0000ff00;
                    bmask = 0x000000ff;
                    amask = 0x00000000;
                    break;

               case DSPF_ARGB:
                    rmask = 0x00ff0000;
                    gmask = 0x0000ff00;
                    bmask = 0x000000ff;
                    amask = 0xff000000;
                    break;

               default:
                    D_ERROR( "PB/Surface: %s() has no support for %s!\n",
                             __FUNCTION__, dfb_pixelformat_name(format) );
                    return DFB_UNSUPPORTED;
          }

          D_DEBUG_AT( PB_Pool, "  -> PB_CreateRGBSurface( 0x%08x, "
                      "%dx%d, %d, 0x%08x, 0x%08x, 0x%08x, 0x%08x )\n",
                      flags, surface->config.size.w, surface->config.size.h,
                      DFB_BITS_PER_PIXEL(format), rmask, gmask, bmask, amask );

          alloc->pb_surf = PB_CreateRGBSurface( flags,
                                                  surface->config.size.w,
                                                  surface->config.size.h,
                                                  DFB_BITS_PER_PIXEL(format),
                                                  rmask, gmask, bmask, amask );*/
		  switch (format) {
               case DSPF_ARGB:
                    break;

               default:
                    D_ERROR( "PB/Surface: %s() has no support for %s!\n",
                             __FUNCTION__, dfb_pixelformat_name(format) );
                    return DFB_UNSUPPORTED;
          }										  
		  alloc->pb_surf =  gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8/*DFB_BITS_PER_PIXEL(format)*/,
		  									surface->config.size.w, surface->config.size.h);										  
          if (!alloc->pb_surf) {
               D_ERROR( "PB/Surface: PB_CreateRGBSurface("
                        "%dx%d, %d) failed!\n",
                        surface->config.size.w, surface->config.size.h,
                        DFB_BITS_PER_PIXEL(format));

               return DFB_FAILURE;
          }
     }

     D_MAGIC_SET( alloc, PBAllocationData );

     return DFB_OK;
}

static DFBResult
pbDeallocateBuffer( CoreSurfacePool       *pool,
                     void                  *pool_data,
                     void                  *pool_local,
                     CoreSurfaceBuffer     *buffer,
                     CoreSurfaceAllocation *allocation,
                     void                  *alloc_data )
{
     PBAllocationData *alloc = alloc_data;

     D_DEBUG_AT( PB_Pool, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );
     D_MAGIC_ASSERT( alloc, PBAllocationData );

     g_object_unref( alloc->pb_surf );

     D_MAGIC_CLEAR( alloc );

     return DFB_OK;
}

static DFBResult
pbLock( CoreSurfacePool       *pool,
         void                  *pool_data,
         void                  *pool_local,
         CoreSurfaceAllocation *allocation,
         void                  *alloc_data,
         CoreSurfaceBufferLock *lock )
{
     PBAllocationData *alloc = alloc_data;
     GdkPixbuf        *pb_surf;

//     D_DEBUG_AT( PB_Pool, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( alloc, PBAllocationData );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     pb_surf = alloc->pb_surf;
     D_ASSERT( pb_surf != NULL );

/*     if (PB_MUSTLOCK( pb_surf ) && PB_LockSurface( pb_surf )) {
          D_ERROR( "PB/Surface: PB_LockSurface() on a %dx%dx surface failed!\n", pb_surf->w, pb_surf->h );
          return DFB_FAILURE;
     }

     D_ASSUME( pb_surf->pixels != NULL );
     if (!pb_surf->pixels)
          return DFB_UNSUPPORTED;

     D_ASSERT( pb_surf->pitch > 0 );*/

     lock->addr   = (void *)gdk_pixbuf_get_pixels(pb_surf);
     lock->pitch  = gdk_pixbuf_get_rowstride(pb_surf);
     lock->offset = 0;
     lock->handle = pb_surf;

     return DFB_OK;
}

static DFBResult
pbUnlock( CoreSurfacePool       *pool,
           void                  *pool_data,
           void                  *pool_local,
           CoreSurfaceAllocation *allocation,
           void                  *alloc_data,
           CoreSurfaceBufferLock *lock )
{
     PBAllocationData *alloc = alloc_data;
     GdkPixbuf        *pb_surf;

//     D_DEBUG_AT( PB_Pool, "%s()\n", __FUNCTION__ );

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( alloc, PBAllocationData );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     pb_surf = alloc->pb_surf;
     D_ASSERT( pb_surf != NULL );

     /*if (PB_MUSTLOCK( pb_surf ))
          PB_UnlockSurface( pb_surf );*/

     return DFB_OK;
}

const SurfacePoolFuncs pbSurfacePoolFuncs = {
     .PoolDataSize       = pbPoolDataSize,
     .AllocationDataSize = pbAllocationDataSize,
     .InitPool           = pbInitPool,
     .DestroyPool        = pbDestroyPool,

     .TestConfig         = pbTestConfig,

     .AllocateBuffer     = pbAllocateBuffer,
     .DeallocateBuffer   = pbDeallocateBuffer,

     .Lock               = pbLock,
     .Unlock             = pbUnlock,
};

