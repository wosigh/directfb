/*
   TI Davinci driver - Video Layer

   (c) Copyright 2007  Telio AG

   Written by Denis Oliver Kropp <dok@directfb.org>

   Code is derived from VMWare driver.

   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

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

#ifndef __DAVINCI_VIDEO_H__
#define __DAVINCI_VIDEO_H__

#include <linux/fb.h>

#include <core/layers.h>

#define DAVINCI_VIDEO_SUPPORTED_OPTIONS  (DLOP_NONE)


typedef struct {
     struct fb_var_screeninfo var;

     bool                     enable;
     bool                     enabled;

     CoreLayerRegionConfig    config;

     vpfe_resizer_params_t    resizer;
     DFBDimension             resized;
     DFBPoint                 offset;
} DavinciVideoLayerData;


extern const DisplayLayerFuncs davinciVideoLayerFuncs;

#endif

