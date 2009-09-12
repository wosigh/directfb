#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <sys/types.h>    
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/mman.h>
#include "event.h"
       
#define MAX_MEM_CHUNCKS 6

#define PRIMARY_MEM_SIZE MAX_MEM_CHUNCKS*1024*1024

static int QUIET = 1;

#define myprintf(fmt...) 	    do {					\
									if (!QUIET) 		\
										printf(fmt); 	\
								} while (0)	

GdkWindow	*container = NULL; //screen

char	plugin_socket_name[50] = {0,};
char	dfb_socket_name[50] = {0,};
char	dfb_primarymem_name[50] = {0,};


void dfb_adapter_resources_alloc_name()
{
	long 		rand = (long)time(NULL);
	struct stat buf;

	sprintf(plugin_socket_name, "/tmp/dfbadapter-plugin");
	sprintf(dfb_socket_name, "/tmp/dfbadapter-dfb");
	sprintf(dfb_primarymem_name, "/tmp/dfbadapter.mem");
	

	/*do {
		sprintf(plugin_socket_name, "/tmp/dfbadapter-plugin-%ld", rand);
	} while (--rand < 0 || stat(plugin_socke_name, &buf) == 0)	
	
	do {
		sprintf(dfb_socket_name, "/tmp/dfbadapter-dfb-%ld", rand);
	} while (--rand < 0 || stat(dfb_socke_name, &buf) == 0);	

	do {
		sprintf(dfb_primarymem_name, "/dev/shm/dfbadapter-%ld.mem", rand);
	} while (--rand < 0 || stat(dfb_primarymem_name, &buf) == 0);*/	
}

static gboolean
window_expose_event (GtkWidget      *widget,
                     GdkEventExpose *event)
{
	myprintf("expose event: GdkWindow ptr =%p\n", widget->window);
	container = widget->window;
}

int exec_dfb_prog(int argc, char **argv)
{
	pid_t pid;
	int 	i;

	pid = fork();

	if (pid == -1) {
		perror("can not fork dfb process\n");
		return -1;
	}	

	if (pid == 0) {

		char **dfb_args = alloca(sizeof(char*) * (argc+3));
		if (!dfb_args) {
			myprintf("alloca: no memory\n");
			return -1;
		}
	
		for (i=0; i<argc; i++) 
			dfb_args[i] = argv[i+1];
		dfb_args[i] = plugin_socket_name;
		dfb_args[i+1] = dfb_socket_name;
		dfb_args[i+2] = dfb_primarymem_name;
		dfb_args[i+3] = NULL;

		if (execvp(argv[0], dfb_args) < 0) {
			perror("can not exec dfb program\n");
			return -1;
		}
	}
	
	return 0;
}

int fd = -1;

void *mapped_dfbmem_addr;
bool dfbmem_mapped = false;

gboolean dfb_event_listener( GIOChannel* chnl, GIOCondition cond, gpointer data )
{
	static once = 0;
	if(cond != G_IO_IN) {
		myprintf("plugin:			error state\n");
	}

	/*if (!once) {
		myprintf("get message \n");
		//once++;
	}*/	

	char buf[1024];
	struct sockaddr_un  addr;
	socklen_t           addr_len = sizeof(addr);

	if (recvfrom( fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len ) > 0) {

		DFBPBPEvent *e = (DFBPBPEvent *)buf;

		myprintf("plugin		-> event from '%s'...\n", addr.sun_path );

		switch (e->type) {
			case DFE_DrawRequest:
				if (!once) {
					myprintf("plugin:			get DFE_DrawRequest message \n");
					//once++;
				}
				PluginEvent pe;
				PluginDrawResponse draw_res;
				draw_res.type   = PE_DrawResponse;
				draw_res.serial = e->eDFBDrawRequest.serial;

				//drawing
				if (container) {
					if (!dfbmem_mapped) {
						int dfbmem_fd = open(dfb_primarymem_name, O_RDONLY);
						if (dfbmem_fd < 0) {
							myprintf("plugin:			Couldn't open dfbmem file!\n");
							return TRUE;
						}
						mapped_dfbmem_addr = mmap( NULL, PRIMARY_MEM_SIZE, 
												PROT_READ, MAP_SHARED, dfbmem_fd, 0 );
						if (mapped_dfbmem_addr == MAP_FAILED) {
							myprintf( "plugin: 		Mapping dfbmem file failed!\n" );
							close( dfbmem_fd );
							return TRUE;
						}

						dfbmem_mapped = true;
					}

					GdkPixbuf*  pixbuf = gdk_pixbuf_new_from_data(mapped_dfbmem_addr + e->eDFBDrawRequest.offset, 
																GDK_COLORSPACE_RGB,
                                                         		TRUE, 8,
                                                         		e->eDFBDrawRequest.w, e->eDFBDrawRequest.h,
                                                         		e->eDFBDrawRequest.pitch, NULL, NULL);
					gdk_draw_pixbuf(container, NULL, pixbuf,
									0, 0,
									0, 0,
									e->eDFBDrawRequest.w, e->eDFBDrawRequest.h,
									GDK_RGB_DITHER_NONE, 0, 0);	
				 }					

				draw_res.done   = true;
				pe.ePluginDrawResponse = draw_res;
				while (sendto( fd, &pe, sizeof(PluginEvent), 0, (struct sockaddr*)&addr, addr_len ) < 0) {
          			switch (errno) {
               			case EINTR:
							myprintf("plugin: 		sendto errno:EINTR\n");
                    		continue;
               			case ECONNREFUSED:
							myprintf("plugin:			sendto errno:ECONNREFUSEED\n");
			   				continue;
                    		//return DR_FUSION;
               			default:
                    		break;
        	 	 	}

          			myprintf( "plugin:		err sendto() dfb\n" );

     			}
				myprintf("plugin:		draw response event sent to dfb\n");
				break;
			default:
				myprintf("plugin:		can not resolve the dfb event\n");
				break;
		}
				
	}

	return TRUE;
}	

void start_gtk()
{
	gtk_init(NULL, NULL);
	
	GtkWidget *desktop = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size((GtkWindow*)desktop, 320, 480);

	g_signal_connect(desktop, "expose-event",
                          G_CALLBACK (window_expose_event), NULL);

	gtk_widget_show_all(desktop);
	
	struct sockaddr_un  addr;
	
	fd = socket( PF_LOCAL, SOCK_RAW, 0 );
	if (fd < 0) {
		perror ("Error creating local socket!\n" );
	}

	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", plugin_socket_name);

	unlink(plugin_socket_name);
	int err = bind(fd, (struct sockaddr*)&addr, sizeof(addr));

	g_assert(err == 0);

	GIOChannel *plugin_chnl = g_io_channel_unix_new(fd);
	g_assert(plugin_chnl);
	g_io_channel_set_encoding( plugin_chnl, NULL, NULL );
	g_io_add_watch( plugin_chnl, G_IO_IN|G_IO_ERR|G_IO_HUP, dfb_event_listener, NULL );

	gtk_main();
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		myprintf("DFB program name not set\n"
			   "Usage: gdkdfbrun [dfb-program-name] [args...]\n");
		exit(-1);
	}

	dfb_adapter_resources_alloc_name();

	exec_dfb_prog(argc-1, &argv[1]);

	start_gtk();

	//start_dfb_event_listener_thread();
	
}
