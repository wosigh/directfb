#ifndef __PB__EVENT_H__
#define __PB__EVENT_H__

/* plugin event */
typedef enum {
	PE_DrawResponse
} PluginEventType;

typedef struct {
	PluginEventType 	type;
	long				serial;
	bool 				done;
} PluginDrawResponse;

typedef union {
	PluginEventType		type;

	PluginDrawResponse	ePluginDrawResponse;
} PluginEvent;	


/* dfb event */
typedef enum {
	DFE_DrawRequest
} DFBEventType;

typedef struct {
	DFBEventType		type;
	long 				serial;
	int					offset;
	int					w;
	int					h;
	int					pitch;
} DFBDrawRequest;

typedef union {
	DFBEventType		type;

	DFBDrawRequest 		eDFBDrawRequest;
} DFBPBPEvent;

#endif
