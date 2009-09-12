

#ifndef __PB__PRIMARY_H__
#define __PB__PRIMARY_H__

#include <fusion/call.h>

#include <core/layers.h>
#include <core/screens.h>

extern ScreenFuncs       pbPrimaryScreenFuncs;
extern DisplayLayerFuncs pbPrimaryLayerFuncs;

FusionCallHandlerResult
dfb_pb_call_handler( int           caller,
                      int           call_arg,
                      void         *call_ptr,
                      void         *ctx,
                      unsigned int  serial,
                      int          *ret_val );

#endif

