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

#include <stdlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <glib.h>

#include <xkbleds.h>

int xkb_leds[NUM_LEDS] = {0,0,0};
int xkb_state = -1;
int old_xkb_state = 0;

  Display* d;
  unsigned int states = 0,old_states = 0;
  int key_syms[NUM_LEDS] = {XK_Caps_Lock, XK_Num_Lock, XK_Scroll_Lock};
  char *lock_names[NUM_LEDS] = {"Caps Lock","Num Lock", "Scroll Lock"};
  char short_lock_names[NUM_LEDS] = "cns";

  int i;
  int masks[NUM_LEDS]; /* NUM, CAPS, SCROLL: indicator mask, for XKB*/

// TRUE - success
gboolean xkbleds_get_state() {
    d = XOpenDisplay(NULL);
    if (!d)
        return FALSE;
    if(XkbGetIndicatorState(d, XkbUseCoreKbd, &states) != Success) {
        XCloseDisplay(d);
        return FALSE;
    }
    XCloseDisplay(d);
    old_xkb_state=xkb_state;
    xkb_state=states;
    for(i = 0; i < NUM_LEDS; i++) {
        xkb_leds[i]=((states & masks[i])!=0);
    }
    return TRUE;
}

// 0 - Success
int xkbleds_init()
{
  KeyCode keys[NUM_LEDS];
  XkbDescPtr xkb;
  char *ind_name = NULL;
  int j, mask;
  int idx[NUM_LEDS];/* NUM, CAPS, SCROLL: indicator index, for XKB */

// open X display
  d = XOpenDisplay(NULL);
  if (!d)
    return 1;
// get keycodes
  for(i = 0; i < NUM_LEDS; i++)
    keys[i] = XKeysymToKeycode(d, key_syms[i]);
// get the keyboard
  xkb = XkbAllocKeyboard();
  if(!xkb){
    XCloseDisplay(d);
    return 1;
  }

  if(XkbGetNames(d, XkbIndicatorNamesMask, xkb) != Success){
    XkbFreeKeyboard(xkb, 0, True);
    XCloseDisplay(d);
    return 1;
  }
// get masks and indexes of indicators
  for(i = 0; i < XkbNumIndicators; i++) {
    if(xkb->names->indicators[i])
      ind_name = XGetAtomName(d, xkb->names->indicators[i]);
    for(j = 0; j < NUM_LEDS; j++){
      if(ind_name && !strcmp(lock_names[j], ind_name)){
       if(XkbGetNamedIndicator(d, xkb->names->indicators[i], &mask,
        NULL, NULL, NULL) == True){
            masks[j] = 1 << mask;
            idx[j] = mask;
         } else {
            XkbFreeKeyboard(xkb, 0, True);
            XCloseDisplay(d);
            return 1;
         }
      }
    }
    if(ind_name){
      free(ind_name);
      ind_name = NULL;
    }
  }
// cleanup
  XkbFreeKeyboard(xkb, 0, True);
  XCloseDisplay(d);
  return 0;
}
