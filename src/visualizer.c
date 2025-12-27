#include "visualizer.h"
#include "data.h"
#include "gif.h"
#include "macos_helper.h"

int device = -1;

void draw_visualizer(GtkWidget* widget, cairo_t* cr, AppData* data) {
	if(!data->visualizer) {
		return;
	}

    int width, height;
    gtk_window_get_size(GTK_WINDOW(widget), &width, &height);

    cairo_set_source_rgba(cr, data->settings->red, data->settings->green, data->settings->blue, data->settings->alpha);

    float cx = (float)width / 2;
    float cy = (float)height / 2;
    float r = data->settings->space;

    float radius = 0.0f;
    for (int i = 0; i < data->stream->channel_cnt; i++) {
        radius += data->stream->channels[i];
    }
    radius /= data->stream->channel_cnt;
	radius = sqrtf(radius);
    radius *= data->settings->radius;

    float angle = (data->stream->angle + 180) * (G_PI / 180.0f);

    float sx = cx + cosf(angle) * r;
    float sy = cy + sinf(angle) * r;

    cairo_arc(cr, sx, sy, radius, 0, 2 * G_PI);
    cairo_fill(cr);
}

void draw_gifs(GtkWidget* widget, cairo_t* cr, AppData* data) {
	for (GList* l = data->gifs; l != NULL; l = l->next) {
		Gif* gif = (Gif*)l->data;
		if (!gif || !gif->gif_iter)
			continue;

		GdkPixbuf* frame = gdk_pixbuf_animation_iter_get_pixbuf(gif->gif_iter);
		GdkPixbuf* scaled_frame = NULL;

		if (gif->gif_width > 0 && gif->gif_height > 0) {
			scaled_frame = gdk_pixbuf_scale_simple(frame,
												   gif->gif_width,
												   gif->gif_height,
												   GDK_INTERP_BILINEAR);
		}

		gdk_cairo_set_source_pixbuf(cr,
			scaled_frame ? scaled_frame : frame,
			gif->gif_x,
			gif->gif_y);


		cairo_paint(cr);

		if (scaled_frame)
			g_object_unref(scaled_frame);

		if (gdk_pixbuf_animation_iter_advance(gif->gif_iter, NULL))
			gtk_widget_queue_draw(widget);
	}
}

gboolean draw_overlay(GtkWidget* widget, cairo_t* cr, gpointer d) {
    AppData* data = (AppData*)d;

    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	draw_gifs(widget, cr, data);
	draw_visualizer(widget, cr, data);

	gtk_widget_queue_draw((GtkWidget*)widget);
	return FALSE;
}

void open_overlay(AppData* data) {
	// macOS overlay - simple window approach
	data->overlay = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_decorated(GTK_WINDOW(data->overlay), FALSE);
	gtk_widget_set_app_paintable(data->overlay, TRUE);
	gtk_widget_set_visual(data->overlay, gdk_screen_get_rgba_visual(gdk_screen_get_default()));
	gtk_window_set_default_size(GTK_WINDOW(data->overlay), 1920, 1080);
	gtk_window_set_keep_above(GTK_WINDOW(data->overlay), TRUE);
	gtk_window_set_accept_focus(GTK_WINDOW(data->overlay), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(data->overlay), GDK_WINDOW_TYPE_HINT_DOCK);
	
	GdkRectangle monitor_geometry;
	gdk_monitor_get_geometry(gdk_display_get_primary_monitor(gdk_display_get_default()), &monitor_geometry);
	gtk_window_move(GTK_WINDOW(data->overlay), monitor_geometry.x, monitor_geometry.y);
	gtk_window_resize(GTK_WINDOW(data->overlay), monitor_geometry.width, monitor_geometry.height);
	
	g_signal_connect(G_OBJECT(data->overlay), "draw", G_CALLBACK(draw_overlay), data);
	gtk_widget_show_all(data->overlay);
	
	gdk_window_set_pass_through(gtk_widget_get_window(data->overlay), TRUE);
	
	// Make window visible on all Spaces/Desktops
	macos_set_window_on_all_spaces(gtk_widget_get_window(data->overlay));
}

void close_overlay(AppData* data) {
	if (data->overlay) {
		gtk_widget_destroy(data->overlay);
		data->overlay = NULL;
	}
}
