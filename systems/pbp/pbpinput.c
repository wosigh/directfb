/*stole from sdlinput.c*/

#include <config.h>

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <directfb.h>

#include "nppalmdefs.h"

#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/input.h>
#include <core/system.h>

#include <direct/mem.h>
#include <direct/thread.h>

#include "pbp.h"

#include <core/input_driver.h>

DFB_INPUT_DRIVER(pbpinput)

/*
 * declaration of private data
 */
typedef struct {
	CoreInputDevice *device;
	DirectThread *thread;
	DFBPB *dfb_pb;
	int input_fifo_fd;
	int stop;
} PBPInputData;

static DFBInputEvent motionX = {
	.type = DIET_UNKNOWN,
	.axisabs = 0,
};

static DFBInputEvent motionY = {
	.type = DIET_UNKNOWN,
	.axisabs = 0,
};

static void motion_compress(int x, int y)
{
	if (motionX.axisabs != x) {
		motionX.type = DIET_AXISMOTION;
		motionX.flags = DIEF_AXISABS;
		motionX.axis = DIAI_X;
		motionX.axisabs = x;
	}

	if (motionY.axisabs != y) {
		motionY.type = DIET_AXISMOTION;
		motionY.flags = DIEF_AXISABS;
		motionY.axis = DIAI_Y;
		motionY.axisabs = y;
	}
}

static void motion_realize(PBPInputData * data)
{
	if (motionX.type != DIET_UNKNOWN) {
		if (motionY.type != DIET_UNKNOWN) {
			/* let DirectFB know two events are coming */
			motionX.flags |= DIEF_FOLLOW;
		}

		dfb_input_dispatch(data->device, &motionX);

		motionX.type = DIET_UNKNOWN;
	}

	if (motionY.type != DIET_UNKNOWN) {
		dfb_input_dispatch(data->device, &motionY);

		motionY.type = DIET_UNKNOWN;
	}
}


static bool translate_key(NpPalmKeyEvent *key, DFBInputEvent * evt)
{
	evt->flags = DIEF_KEYID;
	// Numeric keypad
	/*if (key >= PBPK_KP0 && key <= PBPK_KP9) {
		evt->key_id = DIKI_KP_0 + key - PBPK_KP0;
		return true;
	}

	// Function keys
	if (key >= PBPK_F1 && key <= PBPK_F12) {
		evt->key_id = DIKI_F1 + key - PBPK_F1;
		return true;
	}*/

	// letter keys 
	if (key->rawkeyCode >= 'a' && key->rawkeyCode <= 'z') {
		evt->key_id = DIKI_A + key->rawkeyCode - 'a';
		return true;
	}

	if (key->rawkeyCode >= '0' && key->rawkeyCode <= '9') {
		evt->key_id = DIKI_0 + key->rawkeyCode - '0';
		return true;
	}

	switch (key->rawkeyCode) {
	/*case PBPK_QUOTE:
		evt->key_id = DIKI_QUOTE_RIGHT;
		return true;
	case PBPK_BACKQUOTE:
		evt->key_id = DIKI_QUOTE_LEFT;
		return true;
	case PBPK_COMMA:
		evt->key_id = DIKI_COMMA;
		return true;
	case PBPK_MINUS:
		evt->key_id = DIKI_MINUS_SIGN;
		return true;
	case PBPK_PERIOD:
		evt->key_id = DIKI_PERIOD;
		return true;
	case PBPK_SLASH:
		evt->key_id = DIKI_SLASH;
		return true;
	case PBPK_SEMICOLON:
		evt->key_id = DIKI_SEMICOLON;
		return true;
	case PBPK_LESS:
		evt->key_id = DIKI_LESS_SIGN;
		return true;
	case PBPK_EQUALS:
		evt->key_id = DIKI_EQUALS_SIGN;
		return true;
	case PBPK_LEFTBRACKET:
		evt->key_id = DIKI_BRACKET_LEFT;
		return true;
	case PBPK_RIGHTBRACKET:
		evt->key_id = DIKI_BRACKET_RIGHT;
		return true;
	case PBPK_BACKSLASH:
		evt->key_id = DIKI_BACKSLASH;
		return true;
		// Numeric keypad 
	case PBPK_KP_PERIOD:
		evt->key_id = DIKI_KP_DECIMAL;
		return true;

	case PBPK_KP_DIVIDE:
		evt->key_id = DIKI_KP_DIV;
		return true;

	case PBPK_KP_MULTIPLY:
		evt->key_id = DIKI_KP_MULT;
		return true;

	case PBPK_KP_MINUS:
		evt->key_id = DIKI_KP_MINUS;
		return true;
	case PBPK_KP_PLUS:
		evt->key_id = DIKI_KP_PLUS;
		return true;
	case PBPK_KP_ENTER:
		evt->key_id = DIKI_KP_ENTER;
		return true;

	case PBPK_KP_EQUALS:
		evt->key_id = DIKI_KP_EQUAL;
		return true;
	case PBPK_ESCAPE:
		evt->key_id = DIKI_ESCAPE;
		return true;
	case PBPK_TAB:
		evt->key_id = DIKI_TAB;
		return true;
	case PBPK_RETURN:
		evt->key_id = DIKI_ENTER;
		return true;*/
	case 32: // shift + space == enter on emulater
		if (key->rawModifier == 128) {
			evt->key_id = DIKI_ENTER;
			return true;
		}
		evt->key_id = DIKI_SPACE;
		return true;
	/*case PBPK_BACKSPACE:
		evt->key_id = DIKI_BACKSPACE;
		return true;
	case PBPK_INSERT:
		evt->key_id = DIKI_INSERT;
		return true;
	case PBPK_DELETE:
		evt->key_id = DIKI_DELETE;
		return true;
	case PBPK_PRINT:
		evt->key_id = DIKI_PRINT;
		return true;
	case PBPK_PAUSE:
		evt->key_id = DIKI_PAUSE;
		return true;
		// Arrows + Home/End pad 
	case PBPK_UP:
		evt->key_id = DIKI_UP;
		return true;

	case PBPK_DOWN:
		evt->key_id = DIKI_DOWN;
		return true;

	case PBPK_RIGHT:
		evt->key_id = DIKI_RIGHT;
		return true;
	case PBPK_LEFT:
		evt->key_id = DIKI_LEFT;
		return true;
	case PBPK_HOME:
		evt->key_id = DIKI_HOME;
		return true;
	case PBPK_END:
		evt->key_id = DIKI_END;
		return true;

	case PBPK_PAGEUP:
		evt->key_id = DIKI_PAGE_UP;
		return true;

	case PBPK_PAGEDOWN:
		evt->key_id = DIKI_PAGE_DOWN;
		return true;

		// Key state modifier keys 
	case PBPK_NUMLOCK:
		evt->key_id = DIKI_NUM_LOCK;
		return true;

	case PBPK_CAPSLOCK:
		evt->key_id = DIKI_CAPS_LOCK;
		return true;
	case PBPK_SCROLLOCK:
		evt->key_id = DIKI_SCROLL_LOCK;
		return true;
	
	case PBPK_RSHIFT:
		evt->key_id = DIKI_SHIFT_R;
		return true;*/

	/*case 131: //  used to emulate mouse down
		//evt->key_id = DIKI_SHIFT_L;
		if (key->rawModifier == 64) { 
		if (evt->type == DIET_KEYPRESS)
			shift_on = true;
		else
			shift_on = false;
		}	
		return true;*/
	/*case PBPK_RCTRL:
		evt->key_id = DIKI_CONTROL_R;
		return true;

	case PBPK_LCTRL:
		evt->key_id = DIKI_CONTROL_L;
		return true;

	case PBPK_RALT:
		evt->key_id = DIKI_ALT_R;
		return true;

	case PBPK_LALT:
		evt->key_id = DIKI_ALT_L;
		return true;

	case PBPK_RMETA:
		evt->key_id = DIKI_META_R;
		return true;

	case PBPK_LMETA:
		evt->key_id = DIKI_META_L;
		return true;

	case PBPK_LSUPER:
		evt->key_id = DIKI_SUPER_L;
		return true;

	case PBPK_RSUPER:
		evt->key_id = DIKI_SUPER_R;
		return true;

	case PBPK_MODE:
		evt->key_id = DIKI_ALT_R;
		evt->flags |= DIEF_KEYSYMBOL;
		evt->key_symbol = DIKS_ALTGR;
		return true;*/
	default:
		break;
	}

	evt->flags = DIEF_NONE;
	return false;
}

static bool PBP_PollInputEvent(NpPalmEventType *event, PBPInputData *data)
{
	
	fd_set set;
	//char buf[1024];

	int result;
               
	FD_ZERO( &set );
	FD_SET( data->input_fifo_fd, &set );

	//myprintf("dfb:		 run select on dfb socket...\n");
select:
	result = select( data->input_fifo_fd + 1, &set, NULL, NULL, NULL );
	if (result < 0) {
		switch (errno) {
			case EINTR:
				//continue;
				goto select;
			default:
				D_PERROR( "PBP_PollInputEvent: select() failed!\n" );
   			return false;	 
		}
	}

	if (FD_ISSET( data->input_fifo_fd, &set )) { 
		if (read( data->input_fifo_fd, event, sizeof(NpPalmEventType)) == sizeof(NpPalmEventType) /*> 0*/) { 
			printf("dfb:	recevied input event, type:%d", event->eventType);
			return true;
		}
	}	 
	return false;	 
}


/*
 * Input thread reading from device.
 * Generates events on incoming data.
 */
static void *pbpEventThread(DirectThread * thread, void *driver_data)
{
	printf("dfb:		come to pbpEventThread\n");
	PBPInputData *data = (PBPInputData *) driver_data;
	DFBPB *dfb_pb = data->dfb_pb;

	while (!data->stop) {
		printf("dfb:		pbpEventThread %d\n", __LINE__);
		DFBInputEvent evt;
		NpPalmEventType event;

		//fusion_skirmish_prevail(&dfb_pb->lock);

		/* Check for events */

		while (PBP_PollInputEvent(&event, data)) {
			printf("dfb:		pbpEventThread %d\n", __LINE__);

			//fusion_skirmish_dismiss(&dfb_pb->lock);

			switch (event.eventType) {
			case npPalmPenMoveEvent:
				motion_compress(event.data.penEvent.xCoord, event.data.penEvent.yCoord);
				break;
				
			case npPalmPenUpEvent:
			case npPalmPenDownEvent:
#if 0				
				motion_realize(data);

				if (event.eventType == npPalmPenDownEvent)
					evt.type = DIET_BUTTONPRESS;
				else
					evt.type = DIET_BUTTONRELEASE;

				evt.flags = DIEF_NONE;
				evt.button = DIBI_LEFT;

				/*switch (event.button.button) {
				case PBP_BUTTON_LEFT:
					evt.button = DIBI_LEFT;
					break;
				case PBP_BUTTON_MIDDLE:
					evt.button = DIBI_MIDDLE;
					break;
				case PBP_BUTTON_RIGHT:
					evt.button = DIBI_RIGHT;
					break;
				case PBP_BUTTON_WHEELUP:
				case PBP_BUTTON_WHEELDOWN:
					if (event.type != PBP_MOUSEBUTTONDOWN) {
						fusion_skirmish_prevail
						    (&dfb_pbp->lock);
						continue;
					}
					evt.type = DIET_AXISMOTION;
					evt.flags = DIEF_AXISREL;
					evt.axis = DIAI_Z;
					if (event.button.button ==
					    PBP_BUTTON_WHEELUP)
						evt.axisrel = -1;
					else
						evt.axisrel = 1;
					break;
				default:
					fusion_skirmish_prevail(&dfb_pbp->lock);
					continue;
				}*/

				dfb_input_dispatch(data->device, &evt);
#endif
				break;

			case npPalmKeyUpEvent:
			case npPalmKeyDownEvent:
				// emulator mouse left button using ctrl key
				if ( event.data.keyEvent.rawkeyCode == 131 || event.data.keyEvent.rawModifier == 64)
				{
					motion_realize(data);

					if (event.eventType == npPalmKeyDownEvent)
						evt.type = DIET_BUTTONPRESS;
					else
						evt.type = DIET_BUTTONRELEASE;

					evt.flags = DIEF_NONE;
					evt.button = DIBI_LEFT;
				} else {
					if (event.eventType == npPalmKeyDownEvent)
						evt.type = DIET_KEYPRESS;
					else
						evt.type = DIET_KEYRELEASE;
					/* Get a key id first */
					translate_key(&event.data.keyEvent, &evt);
				}	
				
				/*if (event.data.keyEvent.rawkeyCode ==97) {
					evt.flags |= DIEF_KEYSYMBOL;
					evt.key_symbol = DIKS_SMALL_A;
					printf("dfb   		we get a pressed\n");
				}*/

				/* If PBP provided a symbol, use it */
				/*if (event.key.keysym.unicode) {
					evt.flags |= DIEF_KEYSYMBOL;
					evt.key_symbol =
					    event.key.keysym.unicode;*/

			      	/**
                               * Hack to translate the Control+[letter]
                               * combination to
                               * Modifier: CONTROL, Key Symbol: [letter]
                               * A side effect here is that Control+Backspace
                               * produces Control+h
                               */
					/*if (evt.modifiers == DIMM_CONTROL &&
					    evt.key_symbol >= 1
					    && evt.key_symbol <=
					    ('z' - 'a' + 1)) {
						evt.key_symbol += 'a' - 1;
					}
				}*/

				dfb_input_dispatch(data->device, &evt);
				break;
			/*case PBP_QUIT:
				evt.type = DIET_KEYPRESS;
				evt.flags = DIEF_KEYSYMBOL;
				evt.key_symbol = DIKS_ESCAPE;

				dfb_input_dispatch(data->device, &evt);

				evt.type = DIET_KEYRELEASE;
				evt.flags = DIEF_KEYSYMBOL;
				evt.key_symbol = DIKS_ESCAPE;

				dfb_input_dispatch(data->device, &evt);
				break;*/

			default:
				break;
			}

			//fusion_skirmish_prevail(&dfb_pb->lock);
		}

		//fusion_skirmish_dismiss(&dfb_pb->lock);

		motion_realize(data);

		usleep(10000);

		direct_thread_testcancel(thread);
	}

	return NULL;
}

/* exported symbols */

/*
 * Return the number of available devices.
 * Called once during initialization of DirectFB.
 */
static int driver_get_available(void)
{
	if (dfb_system_type() == CORE_PBP)
		return 1;

	return 0;
}

/*
 * Fill out general information about this driver.
 * Called once during initialization of DirectFB.
 */
static void driver_get_info(InputDriverInfo * info)
{
	/* fill driver info structure */
	snprintf(info->name,
		 DFB_INPUT_DRIVER_INFO_NAME_LENGTH, "PBP Input Driver");
	snprintf(info->vendor,
		 DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH, "ameng.linux@gmail.com");

	info->version.major = 0;
	info->version.minor = 1;
}

/*
 * Open the device, fill out information about it,
 * allocate and fill private data, start input thread.
 * Called during initialization, resuming or taking over mastership.
 */
static DFBResult
driver_open_device(CoreInputDevice * device,
		   unsigned int number,
		   InputDeviceInfo * info, void **driver_data)
{
	printf("come to drvier_open_device\n");
	PBPInputData *data;
	DFBPB *dfb_pb = dfb_system_data();

	/* set device name */
	snprintf(info->desc.name,
		 DFB_INPUT_DEVICE_DESC_NAME_LENGTH, "PBP Input");

	/* set device vendor */
	snprintf(info->desc.vendor, DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "PBP");

	/* set one of the primary input device IDs */
	info->prefered_id = DIDID_KEYBOARD;

	/* set type flags */
	info->desc.type = DIDTF_JOYSTICK | DIDTF_KEYBOARD | DIDTF_MOUSE;

	/* set capabilities */
	info->desc.caps = DICAPS_ALL;

	/* allocate and fill private data */
	data = D_CALLOC(1, sizeof(PBPInputData));

	data->device = device;
	data->dfb_pb = dfb_pb;

	printf("dfb:		driver_open_device %d\n", __LINE__);

	
	unlink(dfb_pb->input_fifo_name);
	if (0 != mkfifo(dfb_pb->input_fifo_name, 0666))
		perror("mkfifo /tmp/shm/dfbadapter-input failed \n");
	data->input_fifo_fd = open(dfb_pb->input_fifo_name, O_RDONLY);
	if (data->input_fifo_fd < 0) {
		printf("dfb:		driver_open_device %d\n", __LINE__);
		D_PERROR("Can't open %s\n", dfb_pb->input_fifo_name);
		//TODO: need to free resources ?
		return DFB_FAILURE;
	} 

	printf("dfb:		driver_open_device %d\n", __LINE__);

	/* start input thread */
	data->thread =
	    direct_thread_create(DTT_INPUT, pbpEventThread, data, "PBP Input");

	/* set private data pointer */
	*driver_data = data;

	return DFB_OK;
}

/*
 * Fetch one entry from the device's keymap if supported.
 */
static DFBResult
driver_get_keymap_entry(CoreInputDevice * device,
			void *driver_data, DFBInputDeviceKeymapEntry * entry)
{
	return DFB_UNSUPPORTED;
}

/*
 * End thread, close device and free private data.
 */
static void driver_close_device(void *driver_data)
{
	PBPInputData *data = (PBPInputData *) driver_data;

	/* stop input thread */
	data->stop = 1;

	direct_thread_join(data->thread);
	direct_thread_destroy(data->thread);

	/* free private data */
	D_FREE(data);
}
