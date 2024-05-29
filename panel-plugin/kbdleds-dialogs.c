/*  xfce4-kbdleds-plugin - panel plugin for keyboard LEDs
 *
 *  Copyright (c) 2011-2024 OCo <oco2000@gmail.com>
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

#include <string.h>
#include <gtk/gtk.h>

#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4panel/libxfce4panel.h>

#include "kbdleds.h"
#include "kbdleds-dialogs.h"

/* the website url */
#define PLUGIN_WEBSITE "https://github.com/oco2000/xfce4-kbdleds-plugin"

static void
kbdleds_configure_response (GtkWidget    *dialog,
                           gint          response,
                           KbdledsPlugin *kbdleds)
{
  gboolean result;

  if (response == GTK_RESPONSE_HELP)
    {
      /* show help */
      result = g_spawn_command_line_async ("exo-open --launch WebBrowser " PLUGIN_WEBSITE, NULL);

      if (G_UNLIKELY (result == FALSE))
        g_warning (_("Unable to open the following url: %s"), PLUGIN_WEBSITE);
    }
  else
    {
      /* remove the dialog data from the plugin */
      g_object_set_data (G_OBJECT (kbdleds->plugin), "dialog", NULL);

      /* unlock the panel menu */
      xfce_panel_plugin_unblock_menu (kbdleds->plugin);

      /* save the plugin */
      kbdleds_save (kbdleds->plugin, kbdleds);

      /* destroy the properties dialog */
      gtk_widget_destroy (dialog);
    }
}

static void change_foreground_color(GtkWidget *button, KbdledsPlugin *kbdleds)
{
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(button), &kbdleds->foreground_color);
  kbdleds_save (kbdleds->plugin, kbdleds);
  refresh();
}

static void change_background_color(GtkWidget *button, KbdledsPlugin *kbdleds)
{
  gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(button), &kbdleds->background_color);
  kbdleds_save (kbdleds->plugin, kbdleds);
  refresh();
}

void
kbdleds_configure (XfcePanelPlugin *plugin,
                  KbdledsPlugin    *kbdleds)
{
  GtkWidget *dialog;

  GtkBox *global_vbox, *foreground_vbox, *background_vbox;
  GtkLabel *foreground_label, *background_label;
  GtkColorButton *foreground_button, *background_button;

  /* block the plugin menu */
  xfce_panel_plugin_block_menu (plugin);

  /* create the dialog */
  dialog = xfce_titled_dialog_new_with_buttons (_("Kbdleds Plugin"),
                                                GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (plugin))),
                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                "gtk-help", GTK_RESPONSE_HELP,
                                                "gtk-close", GTK_RESPONSE_OK,
                                                NULL);

  /* center dialog on the screen */
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);

  /* set dialog icon */
  gtk_window_set_icon_name (GTK_WINDOW (dialog), "xfce4-settings");

  /* link the dialog to the plugin, so we can destroy it when the plugin
   * is closed, but the dialog is still open */
  g_object_set_data (G_OBJECT (plugin), "dialog", dialog);

  global_vbox = GTK_BOX (gtk_box_new(GTK_ORIENTATION_VERTICAL, 6));
  gtk_container_set_border_width (GTK_CONTAINER (global_vbox), 12);
  gtk_widget_show(GTK_WIDGET (global_vbox));
  gtk_box_pack_start(GTK_BOX (gtk_dialog_get_content_area(GTK_DIALOG(dialog))), GTK_WIDGET (global_vbox), TRUE, TRUE, 0);

  /* foreground color */
  foreground_vbox = GTK_BOX (gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12));
  gtk_widget_show(GTK_WIDGET(foreground_vbox));
  gtk_box_pack_start(global_vbox, GTK_WIDGET(foreground_vbox), FALSE, FALSE, 0);

  foreground_label = GTK_LABEL (gtk_label_new_with_mnemonic(_("Active Foreground Color")));
  gtk_label_set_xalign (foreground_label, 0.0f);
  gtk_widget_set_valign(GTK_WIDGET(foreground_label), GTK_ALIGN_CENTER);
  gtk_widget_show(GTK_WIDGET(foreground_label));
  gtk_box_pack_start(GTK_BOX(foreground_vbox), GTK_WIDGET(foreground_label), FALSE, FALSE, 0);

  foreground_button = GTK_COLOR_BUTTON (gtk_color_button_new_with_rgba(&kbdleds->foreground_color));
  gtk_label_set_mnemonic_widget(GTK_LABEL(foreground_label), GTK_WIDGET(foreground_button));
  gtk_widget_show(GTK_WIDGET(foreground_button));
  gtk_box_pack_start(GTK_BOX(foreground_vbox), GTK_WIDGET(foreground_button), FALSE, FALSE, 0);

  /* background color */
  background_vbox = GTK_BOX (gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12));
  gtk_widget_show(GTK_WIDGET(background_vbox));
  gtk_box_pack_start(global_vbox, GTK_WIDGET(background_vbox), FALSE, FALSE, 0);

  background_label = GTK_LABEL (gtk_label_new_with_mnemonic(_("Active Background Color")));
  gtk_label_set_xalign (background_label, 0.0f);
  gtk_widget_set_valign(GTK_WIDGET(background_label), GTK_ALIGN_CENTER);
  gtk_widget_show(GTK_WIDGET(background_label));
  gtk_box_pack_start(GTK_BOX(background_vbox), GTK_WIDGET(background_label), FALSE, FALSE, 0);

  background_button = GTK_COLOR_BUTTON (gtk_color_button_new_with_rgba(&kbdleds->background_color));
  gtk_label_set_mnemonic_widget(GTK_LABEL(background_label), GTK_WIDGET(background_button));
  gtk_widget_show(GTK_WIDGET(background_button));
  gtk_box_pack_start(GTK_BOX(background_vbox), GTK_WIDGET(background_button), FALSE, FALSE, 0);


  /* connect the reponse signal to the dialog */
  g_signal_connect (G_OBJECT (dialog), "response",
                    G_CALLBACK(kbdleds_configure_response), kbdleds);

  g_signal_connect (GTK_WIDGET(foreground_button), "color-set",
            G_CALLBACK(change_foreground_color), kbdleds);

  g_signal_connect (GTK_WIDGET(background_button), "color-set",
            G_CALLBACK(change_background_color), kbdleds);

  /* show the entire dialog */
  gtk_widget_show (dialog);
}

void
kbdleds_about (XfcePanelPlugin *plugin)
{
  /* about dialog code. you can use the GtkAboutDialog
   * or the XfceAboutInfo widget */
  GdkPixbuf *icon;

  const gchar *auth[] =
    {
      "OCo <oco2000@gmail.com>",
      NULL
    };

  icon = xfce_panel_pixbuf_from_source ("kbdleds-plugin", NULL, 32);
  gtk_show_about_dialog (NULL,
                         "logo",         icon,
                         "license",      xfce_get_license_text (XFCE_LICENSE_TEXT_GPL),
                         "version",      PACKAGE_VERSION,
                         "program-name", PACKAGE_NAME,
                         "comments",     _("Kbdleds Plugin"),
                         "website",      PLUGIN_WEBSITE,
                         "copyright",    "Copyright \xc2\xa9 2011-2024 OCo\n",
                         "authors",      auth,
                         NULL);

  if (icon)
    g_object_unref (G_OBJECT (icon));
}
