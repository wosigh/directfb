#ifndef __PB__PB_H__
#define __PB__PB_H__

#include <fusion/call.h>
#include <fusion/lock.h>

#include <core/surface_pool.h>
#include <core/system.h>
#include "primary_mem.h"

typedef struct {
     FusionSkirmish   lock;
     FusionCall       call;

     CoreSurface     *primary;
     CoreSurfacePool *pb_pool;

     VideoMode       *modes;        /* linked list of valid video modes */

	 char			 dfb_primarymem_name[50];
	 PrimaryMemManager *primary_mem_mgr; 		

	 int			 dfb_socket_fd;
	 char			 dfb_socket_name[50];
	 char			 plugin_socket_name[50];

	 pthread_mutex_t draw_mutex;
	 pthread_cond_t drawn_cond;

	 char			 input_fifo_name[50];
     //GdkWindow            *container;    /* fake physical screen */
     //GdkPixbuf            *screen;
} DFBPB;

extern DFBPB  *dfb_pb;
extern CoreDFB *dfb_pb_core;



#endif

