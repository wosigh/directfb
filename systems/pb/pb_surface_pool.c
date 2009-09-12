

#include <config.h>

#include <direct/debug.h>

#include <fusion/conf.h>
#include <fusion/shmalloc.h>
#include <fusion/shm/pool.h>

#include <core/core.h>
#include <core/surface_pool.h>

#include <misc/conf.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "pb.h"

#include <stdio.h>

D_DEBUG_DOMAIN( PB_Pool, "PB/Pool", "PB Surface Pool" );

/**********************************************************************************************************************/

typedef struct {
} pbPoolData;



typedef struct {
     void *addr;
     int   pitch;
     int   size;
} pbAllocationData;

/**********************************************************************************************************************/

static int
pbPoolDataSize( void )
{
     return sizeof(pbPoolData);
}

static int
pbAllocationDataSize( void )
{
     return sizeof(pbAllocationData);
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
pbAllocateBuffer( CoreSurfacePool       *pool,
                      void                  *pool_data,
                      void                  *pool_local,
                      CoreSurfaceBuffer     *buffer,
                      CoreSurfaceAllocation *allocation,
                      void                  *alloc_data )
{
     CoreSurface          *surface;
     pbAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

	 
     surface = buffer->surface;

     D_MAGIC_ASSERT( surface, CoreSurface );

     dfb_surface_calc_buffer_size( surface, 8, 0, &alloc->pitch, &alloc->size );

     alloc->addr = D_MALLOC(alloc->size );
     if (!alloc->addr)
          return D_OOSHM();

     allocation->size  = alloc->size;
	 
	 if (surface->type & CSTF_LAYER) {
	 	  printf("Allocate primary surface buffer addr: %p\n", alloc->addr);
	 }

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
     pbAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     D_FREE( alloc->addr );
	 
	 CoreSurface *surface = buffer->surface;

	 if (surface->type & CSTF_LAYER) {
	 	  printf("Deallocate primary surface buffer addr: %p\n", alloc->addr);
	 }



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
     pbAllocationData *alloc = alloc_data;

     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     lock->addr  = alloc->addr;
     lock->pitch = alloc->pitch;
	 lock->handle = alloc->addr;

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
     D_MAGIC_ASSERT( pool, CoreSurfacePool );
     D_MAGIC_ASSERT( allocation, CoreSurfaceAllocation );
     D_MAGIC_ASSERT( lock, CoreSurfaceBufferLock );

     return DFB_OK;
}

const SurfacePoolFuncs pbSurfacePoolFuncs = {
     .PoolDataSize       = pbPoolDataSize,
     .AllocationDataSize = pbAllocationDataSize,
     .InitPool           = pbInitPool,
     .DestroyPool        = pbDestroyPool,

     .AllocateBuffer     = pbAllocateBuffer,
     .DeallocateBuffer   = pbDeallocateBuffer,

     .Lock               = pbLock,
     .Unlock             = pbUnlock,
};

