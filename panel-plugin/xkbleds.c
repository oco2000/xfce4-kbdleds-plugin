/*  xfce4-kbdleds-plugin - panel plugin for keyboard LEDs
 *
 *  Copyright (c) 2011-2021 OCo <oco2000@gmail.com>
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

#include <stdlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include <xkbleds.h>

int xkb_leds[NUM_LEDS] = {0,0,0};
int xkb_state = -1;
int old_xkb_state = 0;

unsigned int states = 0, old_states = 0;
int key_syms[NUM_LEDS] = {XK_Caps_Lock, XK_Num_Lock, XK_Scroll_Lock};
char *lock_names[NUM_LEDS] = {"Caps Lock", "Num Lock", "Scroll Lock"};
char short_lock_names[NUM_LEDS] = "cns";

int i;
int masks[NUM_LEDS]; /* NUM, CAPS, SCROLL: indicator mask, for XKB*/

void xkbleds_get_initial_state(Display *d) {

  if (XkbGetIndicatorState(d, XkbUseCoreKbd, &states) != Success) {
    return;
  }

  old_xkb_state = xkb_state;
  xkb_state = states;
  for(i = 0; i < NUM_LEDS; i++) {
    xkb_leds[i]=((states & masks[i]) != 0);
  }
  refresh();
}

static int kbdEventBase;

GdkFilterReturn kbd_msg_filter_func(GdkXEvent *xevent, GdkEvent *event, gpointer data)
{
    XEvent *xev = (GdkXEvent *)xevent;
    if (xev->type == kbdEventBase + XkbEventCode)
    {
        XkbAnyEvent *kev = (XkbAnyEvent *)xev;
        if (kev->xkb_type == XkbIndicatorStateNotify)
        {
            XkbIndicatorNotifyEvent *knev = (XkbIndicatorNotifyEvent*)kev;
            old_xkb_state = xkb_state;
            xkb_state = knev->state;
            if (xkb_state != old_xkb_state) {
                for(i = 0; i < NUM_LEDS; i++) {
                    xkb_leds[i]=((xkb_state & masks[i]) != 0);
                }
                refresh();
            }
        }
    }
    return GDK_FILTER_CONTINUE;
}

// 0 - Success
int xkbleds_init()
{
  KeyCode keys[NUM_LEDS];
  XkbDescPtr xkb;
  char *ind_name = NULL;
  int j, mask;
  int idx[NUM_LEDS];/* NUM, CAPS, SCROLL: indicator index, for XKB */
  GdkDisplay *disp;
  Display *d;
  int opcode = 0, errorBase = 0, major = XkbMajorVersion, minor = XkbMinorVersion;

// open X display

  disp = gdk_display_get_default();
  if (!disp) {
      return 1;
  }
  d = gdk_x11_display_get_xdisplay(disp);
  if (!d) {
    return 1;
  }

  if (!XkbQueryExtension(d, &opcode, &kbdEventBase, &errorBase, &major, &minor)) {
    return 1;
  }

// get keycodes
  for (i = 0; i < NUM_LEDS; i++) {
    keys[i] = XKeysymToKeycode(d, key_syms[i]);
  }
// get the keyboard
  xkb = XkbAllocKeyboard();
  if (!xkb) {
    return 1;
  }

  if (XkbGetNames(d, XkbIndicatorNamesMask, xkb) != Success){
    XkbFreeKeyboard(xkb, 0, True);
    return 1;
  }
// get masks and indexes of indicators
  for (i = 0; i < XkbNumIndicators; i++) {
    if (xkb->names->indicators[i]) {
      ind_name = XGetAtomName(d, xkb->names->indicators[i]);
    }
    for (j = 0; j < NUM_LEDS; j++) {
      if (ind_name && !strcmp(lock_names[j], ind_name)){
        if (XkbGetNamedIndicator(d, xkb->names->indicators[i], &mask, NULL, NULL, NULL) == True) {
          masks[j] = 1 << mask;
          idx[j] = mask;
        } else {
          XkbFreeKeyboard(xkb, 0, True);
          return 1;
        }
      }
    }
    if (ind_name) {
      free(ind_name);
      ind_name = NULL;
    }
  }
// cleanup
  XkbFreeKeyboard(xkb, 0, True);
// ask for indicator changes
  XkbSelectEvents(d, XkbUseCoreKbd, XkbIndicatorStateNotifyMask, XkbIndicatorStateNotifyMask);
  gdk_window_add_filter(NULL, kbd_msg_filter_func, NULL);
  xkbleds_get_initial_state(d);
  return 0;
}

void xkbleds_finish()
{
  gdk_window_remove_filter(NULL, kbd_msg_filter_func, NULL);
}
