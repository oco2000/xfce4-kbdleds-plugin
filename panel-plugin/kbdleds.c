/*  xfce4-kbdleds-plugin - panel plugin for keyboard LEDs
 *
 *  Copyright (c) 2011-2019 OCo <oco2000@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <ctype.h>

#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>

#include "kbdleds.h"
#include "kbdleds-dialogs.h"
#include "xkbleds.h"

kbdledsPlugin *kbdleds;

/* prototypes */
static void
kbdleds_construct (XfcePanelPlugin *plugin);

/* register the plugin */
XFCE_PANEL_PLUGIN_REGISTER (kbdleds_construct);

guint timeoutId;

void show_error(gchar *message) {
  GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;
  GtkWidget *dialog = gtk_message_dialog_new (NULL,
                                   flags,
                                   GTK_MESSAGE_ERROR,
                                   GTK_BUTTONS_CLOSE,
                                   "%s",
                                   message);
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

void
kbdleds_save (XfcePanelPlugin *plugin,
             kbdledsPlugin    *kbdleds)
{
  XfceRc *rc;
  gchar  *file;

  /* get the config file location */
  file = xfce_panel_plugin_save_location (plugin, TRUE);

  if (G_UNLIKELY (file == NULL))
    {
       DBG ("Failed to open config file");
       return;
    }

  /* open the config file, read/write */
  rc = xfce_rc_simple_open (file, FALSE);
  g_free (file);

  if (G_LIKELY (rc != NULL)) {
    xfce_rc_write_entry (rc, "Foreground_Color", gdk_rgba_to_string(&kbdleds->foreground_color));
    xfce_rc_write_entry (rc, "Background_Color", gdk_rgba_to_string(&kbdleds->background_color));

    /* close the rc file */
    xfce_rc_close (rc);
  }
}

static void
kbdleds_read (kbdledsPlugin *kbdleds)
{
  XfceRc      *rc;
  gchar       *file;
  const gchar *value;

  /* get the plugin config file location */
  file = xfce_panel_plugin_save_location (kbdleds->plugin, TRUE);

  if (G_LIKELY (file != NULL)) {
    /* open the config file, readonly */
    rc = xfce_rc_simple_open (file, TRUE);

    /* cleanup */
    g_free (file);

    if (G_LIKELY (rc != NULL)) {
      /* read the settings */
      if ((value = xfce_rc_read_entry (rc, "Foreground_Color", NULL)) != NULL) {
        gdk_rgba_parse(&kbdleds->foreground_color, value);
      } else {
        gdk_rgba_parse(&kbdleds->foreground_color, DEFAULT_FOREGROUND_COLOR);
      }

      if ((value = xfce_rc_read_entry (rc, "Background_Color", NULL)) != NULL) {
        gdk_rgba_parse(&kbdleds->background_color, value);
      } else {
        gdk_rgba_parse(&kbdleds->background_color, DEFAULT_BACKGROUND_COLOR);
      }

      /* cleanup */
      xfce_rc_close (rc);

      /* leave the function, everything went well */
      return;
    }
  }

  /* something went wrong, apply default values */
  DBG ("Applying default settings");

  gdk_rgba_parse(&kbdleds->foreground_color, DEFAULT_FOREGROUND_COLOR);
  gdk_rgba_parse(&kbdleds->background_color, DEFAULT_BACKGROUND_COLOR);
}

static kbdledsPlugin *
kbdleds_new (XfcePanelPlugin *plugin)
{
  kbdledsPlugin   *kbdleds;
  GtkOrientation  orientation;
  GtkWidget      *label;

  /* allocate memory for the plugin structure */
  kbdleds = panel_slice_new0 (kbdledsPlugin);

  /* pointer to plugin */
  kbdleds->plugin = plugin;

  /* read the user settings */
  kbdleds_read (kbdleds);

  /* get the current orientation */
  orientation = xfce_panel_plugin_get_orientation (plugin);

  /* create some panel widgets */
  kbdleds->ebox = gtk_event_box_new ();
  gtk_widget_show (kbdleds->ebox);

  kbdleds->hvbox = gtk_box_new (orientation, 2);
  gtk_widget_show (kbdleds->hvbox);
  gtk_container_add (GTK_CONTAINER (kbdleds->ebox), kbdleds->hvbox);

  /* some kbdleds widgets */
  kbdleds->label = gtk_label_new (short_lock_names);
  gtk_widget_set_has_tooltip(kbdleds->label,TRUE);
  gtk_widget_show (kbdleds->label);
  gtk_box_pack_start (GTK_BOX (kbdleds->hvbox), kbdleds->label, FALSE, FALSE, 0);

  return kbdleds;
}

static void
kbdleds_free (XfcePanelPlugin *plugin,
             kbdledsPlugin    *kbdleds)
{
  GtkWidget *dialog;

  /* check if the dialog is still open. if so, destroy it */
  dialog = g_object_get_data (G_OBJECT (plugin), "dialog");
  if (G_UNLIKELY (dialog != NULL))
    gtk_widget_destroy (dialog);

  /* destroy the panel widgets */
  gtk_widget_destroy (kbdleds->hvbox);

  /* cleanup the settings */
  //if (G_LIKELY (kbdleds->setting1 != NULL))
    //g_free (kbdleds->setting1);

  /* free the plugin structure */
  panel_slice_free (kbdledsPlugin, kbdleds);

  /* free the timeout */
  if (timeoutId) {
    g_source_remove(timeoutId);
  }
}

static void
kbdleds_orientation_changed (XfcePanelPlugin *plugin,
                            GtkOrientation   orientation,
                            kbdledsPlugin    *kbdleds)
{
  /* change the orienation of the box */
  gtk_orientable_set_orientation(GTK_ORIENTABLE(kbdleds->hvbox), orientation);
}

static gboolean
kbdleds_size_changed (XfcePanelPlugin *plugin,
                     gint             size,
                     kbdledsPlugin    *kbdleds)
{
  GtkOrientation orientation;

  /* get the orientation of the plugin */
  orientation = xfce_panel_plugin_get_orientation (plugin);

  /* set the widget size */
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, size);
  else
    gtk_widget_set_size_request (GTK_WIDGET (plugin), size, -1);

  /* we handled the orientation */
  return TRUE;
}

gchar* getHexColor(GdkRGBA rgba) {
  return g_strdup_printf("#%02X%02X%02X", (int)(rgba.red*255), (int)(rgba.green*255), (int)(rgba.blue*255));
}

void refresh() {
  int i;
  gchar *str;
  gchar *template_on="<span background=\"%s\" foreground=\"%s\">%c</span>";
  gchar *template_off="%c";
  gchar *led_labels[NUM_LEDS + 1];
  gchar *tooltip_labels[NUM_LEDS + 1];
  gchar *on_off[2]={_("OFF"),_("ON")};
  gchar *tooltip_str={""};
  gchar *label_str={""};

  for(i = 0; i < NUM_LEDS; i++) {
    led_labels[i] = g_strdup_printf("%s : %s", lock_names[i], xkb_leds[i] ? on_off[1] : on_off[0]);

    if (xkb_leds[i]) {
      gchar *fColor = getHexColor(kbdleds->foreground_color);
      gchar *bColor = getHexColor(kbdleds->background_color);

      tooltip_labels[i] = g_strdup_printf(template_on, bColor, fColor, toupper(short_lock_names[i]));

      g_free(bColor);
      g_free(fColor);
    } else {
      tooltip_labels[i] = g_strdup_printf(template_off, short_lock_names[i]);
    }
  }
  led_labels[NUM_LEDS] = NULL;
  tooltip_labels[NUM_LEDS] = NULL;

  tooltip_str = g_strjoinv("\n", led_labels);
  label_str = g_strjoinv(NULL, tooltip_labels);

  gtk_label_set_markup((GtkLabel*)kbdleds->label, label_str);
  gtk_widget_set_tooltip_text(kbdleds->label, tooltip_str);

  for(i = 0; i < NUM_LEDS; i++) {
    g_free(led_labels[i]);
    g_free(tooltip_labels[i]);
  }

  g_free(tooltip_str);
  g_free(label_str);
}

gboolean kbdleds_update_state(gpointer data) {

  if (!xkbleds_get_state()) {
    // stop g_timeout
    return FALSE;
  }
//    syslog(LOG_DEBUG,"%d",kbd_state);

  if (xkb_state != old_xkb_state) {

    refresh();

  }
  return TRUE;
}

static void
kbdleds_construct (XfcePanelPlugin *plugin)
{

  /* setup transation domain */
  xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* create the plugin */
  kbdleds = kbdleds_new (plugin);

  /* add the ebox to the panel */
  gtk_container_add (GTK_CONTAINER (plugin), kbdleds->ebox);

  /* show the panel's right-click menu on this ebox */
  xfce_panel_plugin_add_action_widget (plugin, kbdleds->ebox);

  /* connect plugin signals */
  g_signal_connect (G_OBJECT (plugin), "free-data",
                    G_CALLBACK (kbdleds_free), kbdleds);

  g_signal_connect (G_OBJECT (plugin), "save",
                    G_CALLBACK (kbdleds_save), kbdleds);

  g_signal_connect (G_OBJECT (plugin), "size-changed",
                    G_CALLBACK (kbdleds_size_changed), kbdleds);

  g_signal_connect (G_OBJECT (plugin), "orientation-changed",
                    G_CALLBACK (kbdleds_orientation_changed), kbdleds);

  /* show the configure menu item and connect signal */
  xfce_panel_plugin_menu_show_configure (plugin);
  g_signal_connect (G_OBJECT (plugin), "configure-plugin",
                    G_CALLBACK (kbdleds_configure), kbdleds);

  /* show the about menu item and connect signal */
  xfce_panel_plugin_menu_show_about (plugin);
  g_signal_connect (G_OBJECT (plugin), "about",
                    G_CALLBACK (kbdleds_about), NULL);

  xkbleds_init();
  timeoutId = g_timeout_add(250, kbdleds_update_state, NULL);

}
