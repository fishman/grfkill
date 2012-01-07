/**
 * Copyright (C) Reza Jelveh <reza.jelveh (at) tuhh.de>
 * Based on Gary Ching-Pang Lin's gtk example code
 * https://github.com/lcp/gtk-fun/tree/master/gtk-nodeco
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE. 
 */

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixdata.h>
#include <stdlib.h>

#define BACKGROUND_ALPHA 0.75
#define ICON_SAPCE 6
#define ICON_SIZE 96

#include "bt-blocked.h"
#include "bt-unblocked.h"
#include "close.h"
#include "close-red.h"
#include "wlan-blocked.h"
#include "wlan-unblocked.h"
#include "wwan-blocked.h"
#include "wwan-unblocked.h"

#define DEFAULT_WLAN "acer-wireless"
#define DEFAULT_BT   "acer-bluetooth"

static gboolean wwan_state = TRUE;
static gboolean bt_state = TRUE;
static gboolean wlan_state = TRUE;
static gint64 wwan_index = 0;
static gint64 bt_index = 0;
static gint64 wlan_index = 0;
static gboolean initialized = FALSE;

static GdkPixbuf *bt_blocked_pb;
static GdkPixbuf *bt_unblocked_pb;
static GdkPixbuf *close_icon_pb;
static GdkPixbuf *close_red_pb;
static GdkPixbuf *wlan_blocked_pb;
static GdkPixbuf *wlan_unblocked_pb;
static GdkPixbuf *wwan_blocked_pb;
static GdkPixbuf *wwan_unblocked_pb;

static gchar *wlan_device = NULL;
static gchar *bt_device   = NULL;

draw_rounded_rectangle (cairo_t *cr,
			gdouble  aspect,
			gdouble  x,
			gdouble  y,
			gdouble  corner_radius,
			gdouble  width,
			gdouble  height)
{
	gdouble radius = corner_radius / aspect;

	cairo_move_to (cr, x + radius, y);

	cairo_line_to (cr,
		       x + width - radius,
		       y);
	cairo_arc (cr,
		   x + width - radius,
		   y + radius,
		   radius,
		   -90.0f * G_PI / 180.0f,
		   0.0f * G_PI / 180.0f);
	cairo_line_to (cr,
		       x + width,
		       y + height - radius);
	cairo_arc (cr,
		   x + width - radius,
		   y + height - radius,
		   radius,
		   0.0f * G_PI / 180.0f,
		   90.0f * G_PI / 180.0f);
	cairo_line_to (cr,
		       x + radius,
		       y + height);
	cairo_arc (cr,
		   x + radius,
		   y + height - radius,
		   radius,
		   90.0f * G_PI / 180.0f,
		   180.0f * G_PI / 180.0f);
	cairo_line_to (cr,
		       x,
		       y + radius);
	cairo_arc (cr,
		   x + radius,
		   y + radius,
		   radius,
		   180.0f * G_PI / 180.0f,
		   270.0f * G_PI / 180.0f);
	cairo_close_path (cr);
}

static gboolean
draw_widget (GtkWidget *window,
	     cairo_t   *cr,
	     gpointer   user_data)
{
	GtkStyleContext	*context;
	GdkRGBA		 acolor;
	int		 width;
	int		 height;

	context = gtk_widget_get_style_context (window);

	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	gtk_window_get_size (GTK_WINDOW (window), &width, &height);

	cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
	cairo_paint (cr);

	draw_rounded_rectangle (cr, 1.0, 0.0, 0.0, height/10, width-1, height-1);
	gtk_style_context_get_background_color (context, GTK_STATE_NORMAL, &acolor);
	acolor.alpha = BACKGROUND_ALPHA;
	gdk_cairo_set_source_rgba (cr, &acolor);
	cairo_fill(cr);

	return FALSE;
}

static void
set_visual (GtkWidget *widget)
{
	GdkScreen *screen;
	GdkVisual *visual;

	screen = gtk_widget_get_screen (widget);
	visual = gdk_screen_get_rgba_visual (screen);
	if (visual == NULL)
		visual = gdk_screen_get_system_visual (screen);

	gtk_widget_set_visual (widget, visual);
}

static void
screen_change_cb (GtkWidget *widget,
		  GdkScreen *previous_screen,
		  gpointer   user_data)
{
	set_visual (widget);
}

static void
load_pixbuf (GtkImage   *icon,
	     const gchar *icon_name)
{
	GtkIconTheme *icon_theme;
	GdkPixbuf *pixbuf;

	/* g_print ("button pressed %s\n", icon_name); */

	icon_theme = gtk_icon_theme_new ();
	gtk_icon_theme_prepend_search_path (icon_theme, ".");

	pixbuf = gtk_icon_theme_load_icon (icon_theme,
					   icon_name,
					   ICON_SIZE,
					   0,
					   NULL);
	gtk_image_set_from_pixbuf (icon, pixbuf);
	g_object_unref (icon_theme);
}

static gboolean
switch_activate_cb (GtkSwitch *gtk_switch,
					GtkImage *gtk_icon,
					char *name)
{
	if (gtk_switch_get_active (gtk_switch)){
		load_pixbuf (gtk_icon, g_strconcat(name, "-unblocked",NULL));
		return FALSE;
	}
	else {
		load_pixbuf (gtk_icon, g_strconcat(name, "-blocked",NULL));
		return TRUE;
	}
}

static void
wlan_switch_activate_cb (GtkSwitch *g_switch,
			 gboolean   active,
			 GtkImage  *icon)
{
	gboolean blocked;
	gchar index[UCHAR_MAX];
	blocked = switch_activate_cb (g_switch, icon, "wlan");
	if (gtk_switch_get_active (g_switch))
		gtk_image_set_from_pixbuf(icon, wlan_unblocked_pb);
	else
		gtk_image_set_from_pixbuf(icon, wlan_blocked_pb);

	if(initialized /* if we don't do this rfkill is gonna toggle on startup */){
		g_ascii_dtostr(index, UCHAR_MAX, wlan_index);
		system(g_strconcat("rfkill ", blocked ? "block " : "unblock ", index, NULL));
	}
}

static void
bt_switch_activate_cb (GtkSwitch *g_switch,
		       gboolean   active,
		       GtkImage  *icon)
{
	gboolean blocked;
	gchar index[UCHAR_MAX];
	blocked = switch_activate_cb (g_switch, icon, "bt");

	if (gtk_switch_get_active (g_switch))
		gtk_image_set_from_pixbuf(icon, bt_unblocked_pb);
	else
		gtk_image_set_from_pixbuf(icon, bt_blocked_pb);

	if(initialized /* if we don't do this rfkill is gonna toggle on startup */){
		g_ascii_dtostr(index, UCHAR_MAX, bt_index);
		system(g_strconcat("rfkill ", blocked ? "block " : "unblock ", index, NULL));
	}
}

static void
wwan_switch_activate_cb (GtkSwitch *g_switch,
			 gboolean   active,
			 GtkImage  *icon)
{
	switch_activate_cb (g_switch, icon, "wwan");
}


gboolean
on_event_cb (GtkWidget *widget,
	     GdkEvent  *event,
	     GtkWidget *close_icon)
{
	switch (event->type) {
	case GDK_BUTTON_PRESS:
		/* g_print ("button pressed\n"); */
		gtk_main_quit ();
		break;
	case GDK_ENTER_NOTIFY:
		gtk_image_set_from_pixbuf (GTK_IMAGE (close_icon), close_red_pb);
		break;
	case GDK_LEAVE_NOTIFY:
		gtk_image_set_from_pixbuf (GTK_IMAGE (close_icon), close_icon_pb);
		break;
	}

	return FALSE;
}

void init_close_button(GtkWidget **eventbox){
	GtkWidget *close_icon;

	*eventbox = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (*eventbox), FALSE);
	close_icon = gtk_image_new_from_pixbuf (close_icon_pb);

	gtk_container_add (GTK_CONTAINER (*eventbox), close_icon);
	gtk_widget_add_events (*eventbox, GDK_BUTTON_PRESS_MASK);
	gtk_widget_add_events (*eventbox, GDK_ENTER_NOTIFY_MASK);
	gtk_widget_add_events (*eventbox, GDK_LEAVE_NOTIFY_MASK);

	g_signal_connect (G_OBJECT (*eventbox), "button-press-event",
			  G_CALLBACK (on_event_cb), close_icon);
	g_signal_connect (G_OBJECT (*eventbox), "enter-notify-event",
			  G_CALLBACK (on_event_cb), close_icon);
	g_signal_connect (G_OBJECT (*eventbox), "leave-notify-event",
			  G_CALLBACK (on_event_cb), close_icon);
}

void init_button(GtkWidget *icon, GtkWidget *rf_switch, void *callback){
	g_signal_connect (G_OBJECT (rf_switch), "notify::active",
			  G_CALLBACK (callback), icon);
	gtk_switch_set_active (GTK_SWITCH (rf_switch), TRUE);
}

gboolean get_rfkill_state(gchar *soft_state){
	if (g_str_has_prefix(soft_state, "0")) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

void parse_directory(){
	const gchar path[] = "/sys/class/rfkill";
	GDir *d = g_dir_open( path,0,NULL );
	gchar *contents;
	gchar *soft_state;
	gchar *index;
	gsize length;

	const gchar * files;
	while ( ( files = g_dir_read_name( d ) ) != NULL ) {
		gchar *filename = g_build_filename( path,files,NULL );
		/* rfkill folder is full of symlinks */
		if ( g_file_test( filename,G_FILE_TEST_IS_SYMLINK ) ) {
			g_file_get_contents(g_strconcat(filename, "/name",NULL), &contents, &length, NULL);
			g_file_get_contents(g_strconcat(filename, "/index",NULL), &index, &length, NULL);
			g_file_get_contents(g_strconcat(filename, "/soft",NULL), &soft_state, &length, NULL);

			if (g_strrstr (contents, wlan_device)) {
				wlan_state = get_rfkill_state(soft_state);
				wlan_index = g_ascii_strtoll(index, NULL, 10);
			}
			else if (g_strrstr (contents, bt_device)) {
				bt_state = get_rfkill_state(soft_state);
				bt_index = g_ascii_strtoll(index, NULL, 10);
			}
			g_free(index);
			g_free(contents);
			g_free(soft_state);
			g_free( filename );
		}
	}
	g_dir_close( d );
}

void init_pixbufs(){
	bt_blocked_pb     = gdk_pixbuf_from_pixdata(&bt_blocked_inline, TRUE, NULL);
	bt_unblocked_pb   = gdk_pixbuf_from_pixdata(&bt_unblocked_inline, TRUE, NULL);
	close_icon_pb     = gdk_pixbuf_from_pixdata(&close_inline, TRUE, NULL);
	close_red_pb      = gdk_pixbuf_from_pixdata(&close_red_inline, TRUE, NULL);
	wlan_blocked_pb   = gdk_pixbuf_from_pixdata(&wlan_blocked_inline, TRUE, NULL);
	wlan_unblocked_pb = gdk_pixbuf_from_pixdata(&wlan_unblocked_inline, TRUE, NULL);
	wwan_blocked_pb   = gdk_pixbuf_from_pixdata(&wwan_blocked_inline, TRUE, NULL);
	wwan_unblocked_pb = gdk_pixbuf_from_pixdata(&wwan_unblocked_inline, TRUE, NULL);
}

static gboolean
quit_timeout_handler(GtkWidget *window)
{
	if (window == NULL) return FALSE;

	gtk_widget_destroy (window);
	exit(0);

	return TRUE;
}

static void
parse_option(gint *pargc, gchar **pargv[]){
	GOptionContext *context;
	GError *err = NULL;

	GOptionEntry entries[] = {
		{ "wlan", 'w', 0, G_OPTION_ARG_STRING, &wlan_device,
			"set the rfkill name for your wireless device", "acer-wireless" },
		{ "bluetooth", 'b', 0, G_OPTION_ARG_STRING, &bt_device,
			"set the rfkill name for your bluetooth device", "acer-bluetooth" },
		{ NULL }
	};

	context = g_option_context_new ("");
	g_option_context_add_main_entries (context, entries, NULL);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	if (!g_option_context_parse(context, pargc, pargv, &err)) {
		g_print ("Failed to initialize: %s\n", err->message);
		exit(0);
	}

	if (wlan_device == NULL) {
		wlan_device = DEFAULT_WLAN;
	}
	if (bt_device == NULL) {
		bt_device = DEFAULT_BT;
	}
}

int
main (int argc, char *argv[])
{
	GtkSettings *settings;

	GtkCssProvider *css_provider;
	GtkStyleContext *style_context;

	GtkWidget *window;
	GtkWidget *eventbox;
	GtkWidget *wifi_icon;
	GtkWidget *bt_icon;
	GtkWidget *wwan_icon;
	GtkWidget *rf_switch1;
	GtkWidget *rf_switch2;
	GtkWidget *rf_switch3;
	GtkWidget *grid;

	GtkBorder padding;

	gtk_init (&argc, &argv);

	/* parse commandline options */
	parse_option(&argc, &argv);

	/* initialize states */
	parse_directory();
	init_pixbufs();

	settings = gtk_settings_get_default ();
	g_object_set (G_OBJECT (settings),
		      "gtk-application-prefer-dark-theme", TRUE,
		      NULL);

	css_provider = gtk_css_provider_new ();
	if (!gtk_css_provider_load_from_path (css_provider, "window.css", NULL)) {
		g_warning ("Failed to load css");
		return -1;
	}

	window = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_widget_set_app_paintable(window, TRUE);

	style_context = gtk_widget_get_style_context (window);
	gtk_style_context_add_provider (style_context,
					GTK_STYLE_PROVIDER (css_provider),
					GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	set_visual (window);

	g_signal_connect (G_OBJECT (window), "draw", G_CALLBACK (draw_widget), NULL);
	g_signal_connect (G_OBJECT (window), "screen_changed", G_CALLBACK (screen_change_cb), NULL);

	grid = gtk_grid_new ();

	wifi_icon = gtk_image_new ();
	rf_switch1 = gtk_switch_new ();
	init_button(wifi_icon, rf_switch1, wlan_switch_activate_cb);
	gtk_switch_set_active (GTK_SWITCH (rf_switch1), wlan_state);

	bt_icon = gtk_image_new ();
	rf_switch2 = gtk_switch_new ();
	init_button(bt_icon, rf_switch2, bt_switch_activate_cb);
	gtk_switch_set_active (GTK_SWITCH (rf_switch2), bt_state);

	wwan_icon = gtk_image_new ();
	rf_switch3 = gtk_switch_new ();
	init_button(wwan_icon, rf_switch3, wwan_switch_activate_cb);

	// rfkill will now be used
	initialized = TRUE;

	init_close_button(&eventbox);

	gtk_grid_set_row_spacing ((GtkGrid *)grid, 10);
	gtk_grid_set_column_spacing ((GtkGrid *)grid, 5);
	gtk_grid_attach ((GtkGrid *)grid, eventbox, ICON_SAPCE*3-1, 0, 1, 1);
	gtk_grid_attach ((GtkGrid *)grid, wifi_icon, 0, 1, ICON_SAPCE, 1);
	gtk_grid_attach_next_to ((GtkGrid *)grid, rf_switch1, wifi_icon, GTK_POS_BOTTOM, ICON_SAPCE, 1);


	/* show bt icon */
	gtk_grid_attach_next_to ((GtkGrid *)grid, bt_icon, wifi_icon, GTK_POS_RIGHT, ICON_SAPCE, 1);
	gtk_grid_attach_next_to ((GtkGrid *)grid, rf_switch2, bt_icon, GTK_POS_BOTTOM, ICON_SAPCE, 1);

	/* show wwan icon */
	if (wwan_index != 0){
		gtk_grid_attach_next_to ((GtkGrid *)grid, wwan_icon, bt_icon, GTK_POS_RIGHT, ICON_SAPCE, 1);
		gtk_grid_attach_next_to ((GtkGrid *)grid, rf_switch3, wwan_icon, GTK_POS_BOTTOM, ICON_SAPCE, 1);
	}

	gtk_container_add (GTK_CONTAINER (window), grid);

//	gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
//	gtk_window_set_keep_above (GTK_WINDOW (window), TRUE);
//	gtk_window_set_has_resize_grip (GTK_WINDOW (window), FALSE);
	gtk_style_context_get_padding (style_context, GTK_STATE_NORMAL, &padding);
	gtk_container_set_border_width (GTK_CONTAINER (window), 12 + MAX (padding.left, padding.top));
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ALWAYS);

	gtk_widget_grab_focus (window);

	g_timeout_add(4000, (GSourceFunc) quit_timeout_handler, (gpointer) window);
	gtk_widget_show_all (window);

	gtk_main ();

	gtk_widget_destroy (window);

	return 0;
}

/* vim: set sts=4 sw=4 ts=4: */
