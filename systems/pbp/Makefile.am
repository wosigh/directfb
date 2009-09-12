## Makefile.am for DirectFB/systems/pbp

INCLUDES = \
	-I$(top_srcdir)/include		\
	-I$(top_builddir)/include	\
	-I$(top_builddir)/lib		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src     

systemsdir = $(MODULEDIR)/systems

systems_LTLIBRARIES = libdirectfb_pbp.la

if BUILD_STATIC
systems_DATA = libdirectfb_pbp.o
endif


libdirectfb_pbp_la_LDFLAGS = \
	-avoid-version	\
	-module		

libdirectfb_pbp_la_SOURCES = \
	pbp.h		\
	pbp.c		\
	primary.c	\
	primary.h	\
	pb_surface_pool.c \
	primary_mem.c \
	primary_mem.h \
	event.h 	


libdirectfb_pbp_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/lib/fusion/libfusion.la \
	$(top_builddir)/src/libdirectfb.la		

inputdriversdir = $(MODULEDIR)/inputdrivers

inputdrivers_LTLIBRARIES = libdirectfb_pbpinput.la

if BUILD_STATIC
inputdrivers_DATA = libdirectfb_pbpinput.o
endif


libdirectfb_pbpinput_la_LDFLAGS = \
	-avoid-version	\
	-module

libdirectfb_pbpinput_la_SOURCES = \
	pbpinput.c \
	nppalmdefs.h

libdirectfb_pbpinput_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/lib/fusion/libfusion.la \
	$(top_builddir)/src/libdirectfb.la

#bin_PROGRAMS=dfbgdkrun
#dfbgdkrun: dfbgdkcontainer.c event.h
#	gcc -o dfbgdkrun `pkg-config --cflags --libs gtk+-2.0 gdk-2.0 gthread-2.0` dfbgdkcontainer.c

bin_PROGRAMS=dfbgdkrun
dfbgdkrun_SOURCES =  dfbgdkcontainer.c event.h
dfbgdkrun_LDFLAGS =  `pkg-config --libs gtk+-2.0 gdk-2.0 gthread-2.0`
dfbgdkrun_CFLAGS =  `pkg-config --cflags gtk+-2.0 gdk-2.0 gthread-2.0`

include $(top_srcdir)/rules/libobject.make	

