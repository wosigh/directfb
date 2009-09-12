#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

void PB_Init(GdkWindow *window, GdkPixbuf *screen)
{
	gtk_init(NULL, NULL);

	GtkWidget *desktop = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(desktop, 320, 480);
	window = desktop->window;

	gtk_widget_show_all(desktop);

	// Init pixbuf
	

	gtk_main();
}


/*void PB_SetMode(GdkPixbuf *screen, int bits_per_sample, int w, int h){
	

}*/


void PB_UpdateRect(GdkPixbuf *screen, int x, int y, int w, int h)
{
	gdk_draw_pixbuf(dfb_pb->container,
					NULL,
					screen,
					x, y,
					x, y,
					w, h,
					GDK_RGB_DITHER_NONE, 0, 0);
}
