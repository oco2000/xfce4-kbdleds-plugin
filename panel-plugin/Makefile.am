INCLUDES = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"xfce4-kbdleds-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# kbdleds plugin
#
plugin_PROGRAMS = \
	xfce4-kbdleds-plugin

#plugindir = \
#	$(libexecdir)/xfce4/panel-plugins

plugindir = \
	$(libdir)/xfce4/panel-plugins

xfce4_kbdleds_plugin_SOURCES = \
	kbdleds.c \
	kbdleds.h \
	xkbleds.c \
	xkbleds.h \
	kbdleds-dialogs.c \
	kbdleds-dialogs.h

xfce4_kbdleds_plugin_CFLAGS = \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
	$(PLATFORM_CFLAGS)

xfce4_kbdleds_plugin_LDADD = \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(LIBX11_LIBS) \
	$(LIBXFCE4PANEL_LIBS)

#
# Desktop file
#
desktopdir =								\
	$(datadir)/xfce4/panel-plugins

desktop_in_in_files =							\
	kbdleds.desktop.in.in

desktop_in_files =							\
	$(desktop_in_in_files:.desktop.in.in=.desktop.in)

%.desktop.in: %.desktop.in.in
	sed -e "s,\@libexecdir\@,$(libdir),g" < $< > $@

desktop_DATA =								\
	$(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =								\
	$(desktop_in_in_files)

CLEANFILES =								\
	$(desktop_in_files)						\
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
