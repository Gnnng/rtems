##
## $Id$
##

MANAGERS = all

rtems_tests_PROGRAMS = sp54
sp54_SOURCES = init.c

dist_rtems_tests_DATA = sp54.scn
dist_rtems_tests_DATA += sp54.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

sp54_LDADD = $(MANAGERS_NOT_WANTED:%=$(PROJECT_LIB)/no-%.rel)

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(sp54_OBJECTS) $(sp54_LDADD)
LINK_LIBS = $(sp54_LDLIBS)

sp54$(EXEEXT): $(sp54_OBJECTS) $(sp54_DEPENDENCIES)
	@rm -f sp54$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
