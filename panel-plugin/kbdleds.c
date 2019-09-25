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

#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4panel/xfce-hvbox.h>
//#include <syslog.h>

#include "kbdleds.h"
#include "kbdleds-dialogs.h"
#include "xkbleds.h"

/* default settings */
#define DEFAULT_SETTING1 NULL
#define DEFAULT_SETTING2 1
#define DEFAULT_SETTING3 FALSE

kbdledsPlugin *kbdleds;

/* prototypes */
static void
kbdleds_construct (XfcePanelPlugin *plugin);

/* register the plugin */
XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (kbdleds_construct);

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

  if (G_LIKELY (rc != NULL))
    {
      /* save the settings */
      DBG(".");
      if (kbdleds->setting1)
        xfce_rc_write_entry    (rc, "setting1", kbdleds->setting1);

      xfce_rc_write_int_entry  (rc, "setting2", kbdleds->setting2);
      xfce_rc_write_bool_entry (rc, "setting3", kbdleds->setting3);

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

  if (G_LIKELY (file != NULL))
    {
      /* open the config file, readonly */
      rc = xfce_rc_simple_open (file, TRUE);

      /* cleanup */
      g_free (file);

      if (G_LIKELY (rc != NULL))
        {
          /* read the settings */
          value = xfce_rc_read_entry (rc, "setting1", DEFAULT_SETTING1);
          kbdleds->setting1 = g_strdup (value);

          kbdleds->setting2 = xfce_rc_read_int_entry (rc, "setting2", DEFAULT_SETTING2);
          kbdleds->setting3 = xfce_rc_read_bool_entry (rc, "setting3", DEFAULT_SETTING3);

          /* cleanup */
          xfce_rc_close (rc);

          /* leave the function, everything went well */
          return;
        }
    }

  /* something went wrong, apply default values */
  DBG ("Applying default settings");

  kbdleds->setting1 = g_strdup (DEFAULT_SETTING1);
  kbdleds->setting2 = DEFAULT_SETTING2;
  kbdleds->setting3 = DEFAULT_SETTING3;
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

  kbdleds->hvbox = xfce_hvbox_new (orientation, FALSE, 2);
  gtk_widget_show (kbdleds->hvbox);
  gtk_container_add (GTK_CONTAINER (kbdleds->ebox), kbdleds->hvbox);

  /* some kbdleds widgets */
  kbdleds->label = gtk_label_new (short_lock_names);
  gtk_widget_set_has_tooltip(kbdleds->label,TRUE);
  gtk_widget_show (kbdleds->label);
  gtk_box_pack_start (GTK_BOX (kbdleds->hvbox), kbdleds->label, FALSE, FALSE, 0);
/*
  label = gtk_label_new (_("Plugin"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (kbdleds->hvbox), label, FALSE, FALSE, 0);
*/
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
  if (G_LIKELY (kbdleds->setting1 != NULL))
    g_free (kbdleds->setting1);

  /* free the plugin structure */
  panel_slice_free (kbdledsPlugin, kbdleds);
}

static void
kbdleds_orientation_changed (XfcePanelPlugin *plugin,
                            GtkOrientation   orientation,
                            kbdledsPlugin    *kbdleds)
{
  /* change the orienation of the box */
  xfce_hvbox_set_orientation (XFCE_HVBOX (kbdleds->hvbox), orientation);
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

gboolean kbdleds_update_state(gpointer data) {
    int i;
    gchar *str;
    gchar *template_on="<span background=\"#00ff00\" foreground=\"#000000\">%c</span>";
    gchar *template_off="%c";
    gchar *tempstr[NUM_LEDS];
    gchar *templates[NUM_LEDS];
    gchar *on_off[2]={_("OFF"),_("ON")};
    gchar *tooltip={""};
    gchar *label_str={""};

    if (!xkbleds_get_state())
// stop g_timeout
        return FALSE;
//    syslog(LOG_DEBUG,"%d",kbd_state);

    if (xkb_state!=old_xkb_state) {
        str=g_strdup(short_lock_names);
        for(i = 0; i < NUM_LEDS; i++) {
            tempstr[i]=g_strdup_printf("%s : %s",lock_names[i],xkb_leds[i] ? on_off[1]:on_off[0]);
            if (xkb_leds[i]) {
//                str[i]=toupper(str[i]);
                templates[i]=g_strdup_printf(template_on,toupper(str[i]));
            } else
                templates[i]=g_strdup_printf(template_off,str[i]);
        }
        tooltip=g_strdup_printf("%s\n%s\n%s",tempstr[0],tempstr[1],tempstr[2]);
        label_str=g_strconcat(templates[0],templates[1],templates[2],NULL);
        for(i = 0; i < NUM_LEDS; i++) {
            g_free(tempstr[i]);
            g_free(templates[i]);
        }
        gtk_label_set_markup((GtkLabel*)kbdleds->label,label_str);
        gtk_widget_set_tooltip_text(kbdleds->label,tooltip);
        g_free(tooltip);
        g_free(str);
        g_free(label_str);
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
/*  xfce_panel_plugin_menu_show_configure (plugin);
  g_signal_connect (G_OBJECT (plugin), "configure-plugin",
                    G_CALLBACK (kbdleds_configure), kbdleds);
*/
  /* show the about menu item and connect signal */
/*  xfce_panel_plugin_menu_show_about (plugin);
  g_signal_connect     (G_OBJECT (plugin), "about",
                    G_CALLBACK (kbdleds_about), NULL);
*/
//  openlog("xkbleds",0,LOG_USER);
  xkbleds_init();
  g_timeout_add(250,kbdleds_update_state,NULL);

}
