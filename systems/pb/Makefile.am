## Makefile.am for DirectFB/systems/pb

INCLUDES = \
	-I$(top_srcdir)/include		\
	-I$(top_builddir)/include	\
	-I$(top_builddir)/lib		\
	-I$(top_srcdir)/lib		\
	-I$(top_srcdir)/src     \
	$(PB_CFLAGS)

systemsdir = $(MODULEDIR)/systems

systems_LTLIBRARIES = libdirectfb_pb.la

if BUILD_STATIC
systems_DATA = libdirectfb_pb.o
endif


libdirectfb_pb_la_LDFLAGS = \
	-avoid-version	\
	-module		

libdirectfb_pb_la_SOURCES = \
	pb.c		\
	pb.h		\
	primary.c	\
	primary.h	\
	pb_surface_pool.c


libdirectfb_pb_la_LIBADD = \
	$(top_builddir)/lib/direct/libdirect.la \
	$(top_builddir)/lib/fusion/libfusion.la \
	$(top_builddir)/src/libdirectfb.la		\
	$(PB_LIBS)

#inputdriversdir = $(MODULEDIR)/inputdrivers

#inputdrivers_LTLIBRARIES = libdirectfb_pbinput.la

#if BUILD_STATIC
#inputdrivers_DATA = libdirectfb_pbinput.o
#endif


#libdirectfb_pbinput_la_LDFLAGS = \
#	-framework Carbon	\
#	-avoid-version	\
#	-module

#libdirectfb_pbinput_la_SOURCES = \
#	pbinput.c

#libdirectfb_pbinput_la_LIBADD = \
#	$(top_builddir)/lib/direct/libdirect.la \
#	$(top_builddir)/lib/fusion/libfusion.la \
#	$(top_builddir)/src/libdirectfb.la


include $(top_srcdir)/rules/libobject.make	

