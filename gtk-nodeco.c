#include <gtk/gtk.h>

#define BACKGROUND_ALPHA 0.75
#define ICON_SAPCE 6
#define ICON_SIZE 96

void
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
	     const char *icon_name)
{
	GtkIconTheme *icon_theme;
	GdkPixbuf *pixbuf;

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

static void
wlan_switch_activate_cb (GtkSwitch *wlan_switch,
			 gboolean   active,
			 GtkImage  *wlan_icon)
{
	if (gtk_switch_get_active (wlan_switch))
		load_pixbuf (wlan_icon, "wlan-unblocked");
	else
		load_pixbuf (wlan_icon, "wlan-blocked");
}

static void
bt_switch_activate_cb (GtkSwitch *bt_switch,
		       gboolean   active,
		       GtkImage  *bt_icon)
{
	if (gtk_switch_get_active (bt_switch))
		load_pixbuf (bt_icon, "bt-unblocked");
	else
		load_pixbuf (bt_icon, "bt-blocked");
}

static void
wwan_switch_activate_cb (GtkSwitch *wwan_switch,
			 gboolean   active,
			 GtkImage  *wwan_icon)
{
	if (gtk_switch_get_active (wwan_switch))
		load_pixbuf (wwan_icon, "wwan-unblocked");
	else
		load_pixbuf (wwan_icon, "wwan-blocked");
}


gboolean
on_event_cb (GtkWidget *widget,
	     GdkEvent  *event,
	     GtkWidget *close_icon)
{
	switch (event->type) {
	case GDK_BUTTON_PRESS:
		g_print ("button pressed\n");
		gtk_main_quit ();
		break;
	case GDK_ENTER_NOTIFY:
		gtk_image_set_from_file (GTK_IMAGE (close_icon), "close-red.svg");
		break;
	case GDK_LEAVE_NOTIFY:
		gtk_image_set_from_file (GTK_IMAGE (close_icon), "close.svg");
		break;
	}

	return FALSE;
}

int
main (int argc, char *argv[])
{
	GtkSettings *settings;

	GtkCssProvider *css_provider;
	GtkStyleContext *style_context;

	GtkWidget *window;
	GtkWidget *eventbox;
	GtkWidget *close_icon;
	GtkWidget *wifi_icon;
	GtkWidget *bt_icon;
	GtkWidget *wwan_icon;
	GtkWidget *rf_switch1;
	GtkWidget *rf_switch2;
	GtkWidget *rf_switch3;
	GtkWidget *grid;

	GtkBorder padding;

	char *text;

	gtk_init (&argc, &argv);

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
	eventbox = gtk_event_box_new ();
	gtk_event_box_set_visible_window (GTK_EVENT_BOX (eventbox), FALSE);
	close_icon = gtk_image_new_from_file ("close.svg");

	wifi_icon = gtk_image_new ();
	rf_switch1 = gtk_switch_new ();
	g_signal_connect (G_OBJECT (rf_switch1), "notify::active",
			  G_CALLBACK (wlan_switch_activate_cb), wifi_icon);
	bt_icon = gtk_image_new ();
	rf_switch2 = gtk_switch_new ();
	g_signal_connect (G_OBJECT (rf_switch2), "notify::active",
			  G_CALLBACK (bt_switch_activate_cb), bt_icon);
	wwan_icon = gtk_image_new ();
	rf_switch3 = gtk_switch_new ();
	g_signal_connect (G_OBJECT (rf_switch3), "notify::active",
			  G_CALLBACK (wwan_switch_activate_cb), wwan_icon);

	gtk_switch_set_active (GTK_SWITCH (rf_switch1), TRUE);
	gtk_switch_set_active (GTK_SWITCH (rf_switch2), TRUE);
	gtk_switch_set_active (GTK_SWITCH (rf_switch3), TRUE);

	gtk_container_add (GTK_CONTAINER (eventbox), close_icon);
	gtk_widget_add_events (eventbox, GDK_BUTTON_PRESS_MASK);
	gtk_widget_add_events (eventbox, GDK_ENTER_NOTIFY_MASK);
	gtk_widget_add_events (eventbox, GDK_LEAVE_NOTIFY_MASK);

	g_signal_connect (G_OBJECT (eventbox), "button-press-event",
			  G_CALLBACK (on_event_cb), close_icon);
	g_signal_connect (G_OBJECT (eventbox), "enter-notify-event",
			  G_CALLBACK (on_event_cb), close_icon);
	g_signal_connect (G_OBJECT (eventbox), "leave-notify-event",
			  G_CALLBACK (on_event_cb), close_icon);

	gtk_grid_set_row_spacing ((GtkGrid *)grid, 10);
	gtk_grid_set_column_spacing ((GtkGrid *)grid, 5);
	gtk_grid_attach ((GtkGrid *)grid, eventbox, ICON_SAPCE*3-1, 0, 1, 1);
	gtk_grid_attach ((GtkGrid *)grid, wifi_icon, 0, 1, ICON_SAPCE, 1);
	gtk_grid_attach_next_to ((GtkGrid *)grid, bt_icon, wifi_icon, GTK_POS_RIGHT, ICON_SAPCE, 1);
	gtk_grid_attach_next_to ((GtkGrid *)grid, wwan_icon, bt_icon, GTK_POS_RIGHT, ICON_SAPCE, 1);
	gtk_grid_attach_next_to ((GtkGrid *)grid, rf_switch1, wifi_icon, GTK_POS_BOTTOM, ICON_SAPCE, 1);
	gtk_grid_attach_next_to ((GtkGrid *)grid, rf_switch2, bt_icon, GTK_POS_BOTTOM, ICON_SAPCE, 1);
	/* gtk_grid_attach_next_to ((GtkGrid *)grid, rf_switch3, wwan_icon, GTK_POS_BOTTOM, ICON_SAPCE, 1); */

	gtk_container_add (GTK_CONTAINER (window), grid);

//	gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
//	gtk_window_set_keep_above (GTK_WINDOW (window), TRUE);
//	gtk_window_set_has_resize_grip (GTK_WINDOW (window), FALSE);
	gtk_style_context_get_padding (style_context, GTK_STATE_NORMAL, &padding);
	gtk_container_set_border_width (GTK_CONTAINER (window), 12 + MAX (padding.left, padding.top));
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ALWAYS);

	gtk_widget_grab_focus (window);

	gtk_widget_show_all (window);

	gtk_main ();

	gtk_widget_destroy (window);

	return 0;
}
