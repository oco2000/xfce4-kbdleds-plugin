AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"xfce4-kbdleds-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# kbdleds plugin
#
plugin_LTLIBRARIES = \
	libkbdleds.la

plugindir = \
	$(libdir)/xfce4/panel/plugins

libkbdleds_la_SOURCES = \
	kbdleds.c \
	kbdleds.h \
	xkbleds.c \
	xkbleds.h \
	kbdleds-dialogs.c \
	kbdleds-dialogs.h

libkbdleds_la_CFLAGS = \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
	$(PLATFORM_CFLAGS)

libkbdleds_la_LDFLAGS = \
       -avoid-version \
       -module \
       -no-undefined \
       -export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
       $(PLATFORM_LDFLAGS)

libkbdleds_la_LIBADD = \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(LIBXFCE4PANEL_LIBS)

#
# Desktop file
#
desktopdir =								\
	$(datadir)/xfce4/panel/plugins

desktop_DATA =								\
	kbdleds.desktop

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =								\
	kbdleds.desktop.in

CLEANFILES =								\
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
