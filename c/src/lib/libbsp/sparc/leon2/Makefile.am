##
## $Id$
##

ACLOCAL_AMFLAGS = -I ../../../../aclocal

include $(top_srcdir)/../../../../automake/compile.am

include_bspdir = $(includedir)/bsp

dist_project_lib_DATA = bsp_specs

include_HEADERS = include/bsp.h
include_HEADERS += include/tm27.h
include_HEADERS += ../../sparc/shared/include/debug_defs.h

nodist_include_HEADERS = include/bspopts.h
nodist_include_bsp_HEADERS = ../../shared/include/bootcard.h
DISTCLEANFILES = include/bspopts.h

noinst_PROGRAMS =

# wrapup is the one that actually builds and installs the library
# from the individual .rel files built in other directories
SUBDIRS = . tools

include_HEADERS += include/leon.h
include_HEADERS += ../../shared/include/coverhd.h

noinst_LIBRARIES = libbspstart.a
libbspstart_a_SOURCES = ../../sparc/shared/start.S
project_lib_DATA = start.$(OBJEXT)

dist_project_lib_DATA += startup/linkcmds ../shared/startup/linkcmds.base

noinst_LIBRARIES += libbsp.a
libbsp_a_SOURCES =

# startup
libbsp_a_SOURCES += ../../shared/bspclean.c ../../shared/bsplibc.c \
    ../../shared/bsppost.c startup/bsppredriver.c \
    startup/bspstart.c ../../sparc/shared/bsppretaskinghook.c \
    ../../sparc/shared/bspgetworkarea.c ../../shared/bootcard.c \
    ../../shared/sbrk.c startup/setvec.c startup/spurious.c startup/bspidle.c \
    ../../sparc/shared/startup/early_malloc.c
# gnatsupp
libbsp_a_SOURCES += gnatsupp/gnatsupp.c ../../sparc/shared/gnatcommon.c
# console
libbsp_a_SOURCES += console/console.c console/debugputs.c
# clock
libbsp_a_SOURCES += clock/ckinit.c ../../../shared/clockdrv_shell.h
# IRQ
include_HEADERS += ../../sparc/shared/include/genirq.h
libbsp_a_SOURCES += ../../sparc/shared/irq/genirq.c
include_bsp_HEADERS = \
    ../../shared/include/irq-generic.h \
    ../../shared/include/irq-info.h \
    include/bsp/irq.h
libbsp_a_SOURCES += \
    ../../sparc/shared/irq/irq-shared.c \
    ../../shared/src/irq-generic.c \
    ../../shared/src/irq-legacy.c \
    ../../shared/src/irq-info.c \
    ../../shared/src/irq-shell.c \
    ../../shared/src/irq-server.c

# AMBA bus
include_HEADERS += ../../sparc/shared/include/ambapp.h
include_HEADERS += ../../sparc/shared/include/ambapp_ids.h
include_HEADERS += ../../sparc/shared/include/grlib.h
include_HEADERS += ../../sparc/shared/include/ahbstat.h
libbsp_a_SOURCES += ../../sparc/shared/amba/ambapp.c
libbsp_a_SOURCES += ../../sparc/shared/amba/ambapp_alloc.c
libbsp_a_SOURCES += ../../sparc/shared/amba/ambapp_count.c
libbsp_a_SOURCES += ../../sparc/shared/amba/ambapp_depth.c
libbsp_a_SOURCES += ../../sparc/shared/amba/ambapp_find_by_idx.c
libbsp_a_SOURCES += ../../sparc/shared/amba/ambapp_freq.c
libbsp_a_SOURCES += ../../sparc/shared/amba/ambapp_parent.c
libbsp_a_SOURCES += ../../sparc/shared/amba/ambapp_old.c
libbsp_a_SOURCES += ../../sparc/shared/amba/ambapp_names.c
libbsp_a_SOURCES += ../../sparc/shared/amba/ambapp_show.c
libbsp_a_SOURCES += ../../sparc/shared/amba/ahbstat.c

# Clock Driver and Timer Library
include_HEADERS += ../../sparc/shared/include/tlib.h
libbsp_a_SOURCES += ../../sparc/shared/timer/gptimer.c
libbsp_a_SOURCES += ../../sparc/shared/timer/tlib.c

# PCI
include_HEADERS += ../../sparc/shared/include/grpci2.h
libbsp_a_SOURCES += ../../sparc/shared/pci/grpci2.c
libbsp_a_SOURCES += ../../sparc/shared/pci/grpci.c
libbsp_a_SOURCES += ../../sparc/shared/pci/pcif.c
libbsp_a_SOURCES +=  ../../sparc/shared/pci/pci_memreg_sparc_le.c
libbsp_a_SOURCES +=  ../../sparc/shared/pci/pci_memreg_sparc_be.c
libbsp_a_SOURCES +=  pci/at697_pci.c

# PCI target devices
include_HEADERS += ../../sparc/shared/include/gr_701.h
include_HEADERS += ../../sparc/shared/include/gr_rasta_adcdac.h
include_HEADERS += ../../sparc/shared/include/gr_rasta_io.h
include_HEADERS += ../../sparc/shared/include/gr_rasta_tmtc.h
include_HEADERS += ../../sparc/shared/include/gr_leon4_n2x.h
libbsp_a_SOURCES +=  ../../sparc/shared/pci/gr_701.c
libbsp_a_SOURCES +=  ../../sparc/shared/pci/gr_rasta_adcdac.c
libbsp_a_SOURCES +=  ../../sparc/shared/pci/gr_rasta_io.c
libbsp_a_SOURCES +=  ../../sparc/shared/pci/gr_rasta_spw_router.c
libbsp_a_SOURCES +=  ../../sparc/shared/pci/gr_rasta_tmtc.c
libbsp_a_SOURCES +=  ../../sparc/shared/pci/gr_leon4_n2x.c

# B1553BRM
include_HEADERS += ../../sparc/shared/include/b1553brm.h
include_HEADERS += ../../sparc/shared/include/b1553rt.h
libbsp_a_SOURCES += ../../sparc/shared/1553/b1553brm.c
libbsp_a_SOURCES += ../../sparc/shared/1553/b1553rt.c

# GR1553B
include_HEADERS += ../../sparc/shared/include/gr1553b.h
include_HEADERS += ../../sparc/shared/include/gr1553bc.h
include_HEADERS += ../../sparc/shared/include/gr1553bc_list.h
include_HEADERS += ../../sparc/shared/include/gr1553bm.h
include_HEADERS += ../../sparc/shared/include/gr1553rt.h
libbsp_a_SOURCES += ../../sparc/shared/1553/gr1553b.c
libbsp_a_SOURCES += ../../sparc/shared/1553/gr1553bc.c
libbsp_a_SOURCES += ../../sparc/shared/1553/gr1553bm.c
libbsp_a_SOURCES += ../../sparc/shared/1553/gr1553rt.c

# CAN
include_HEADERS += ../../sparc/shared/include/occan.h
include_HEADERS += ../../sparc/shared/include/grcan.h
libbsp_a_SOURCES += ../../sparc/shared/can/occan.c \
    ../../sparc/shared/can/grcan.c

# SpaceWire
include_HEADERS += ../../sparc/shared/include/grspw.h
include_HEADERS += ../../sparc/shared/include/grspw_router.h
include_HEADERS += ../../sparc/shared/include/rmap.h
include_HEADERS += ../../sparc/shared/include/rmap_drv_grspw.h
libbsp_a_SOURCES += ../../sparc/shared/spw/grspw.c
libbsp_a_SOURCES += ../../sparc/shared/spw/grspw_router.c
libbsp_a_SOURCES += ../../sparc/shared/spw/rmap.c
libbsp_a_SOURCES += ../../sparc/shared/spw/rmap_async.c
libbsp_a_SOURCES += ../../sparc/shared/spw/rmap_common.c
libbsp_a_SOURCES += ../../sparc/shared/spw/rmap_drv_grspw.c

# UART (RAW)
include_HEADERS += ../../sparc/shared/include/apbuart.h
libbsp_a_SOURCES += ../../sparc/shared/uart/apbuart.c

# I2CMST
include_HEADERS += ../../sparc/shared/include/i2cmst.h
libbsp_a_SOURCES += ../../sparc/shared/i2c/i2cmst.c

# SPI
include_HEADERS += ../../sparc/shared/include/spictrl.h
libbsp_a_SOURCES += ../../sparc/shared/spi/spictrl.c

# TIME
include_HEADERS += ../../sparc/shared/include/spwcuc.h
include_HEADERS += ../../sparc/shared/include/spwtdp.h
include_HEADERS += ../../sparc/shared/include/grctm.h
libbsp_a_SOURCES += ../../sparc/shared/time/spwcuc.c
libbsp_a_SOURCES += ../../sparc/shared/time/spwtdp.c
libbsp_a_SOURCES += ../../sparc/shared/time/grctm.c

# GPIO
include_HEADERS += ../../sparc/shared/include/grgpio.h
include_HEADERS += ../../sparc/shared/include/gpiolib.h
libbsp_a_SOURCES += ../../sparc/shared/gpio/grgpio.c
libbsp_a_SOURCES += ../../sparc/shared/gpio/gpiolib.c

# PWM
include_HEADERS += ../../sparc/shared/include/grpwm.h
libbsp_a_SOURCES += ../../sparc/shared/pwm/grpwm.c

# ADC and DAC
include_HEADERS += ../../sparc/shared/include/gradcdac.h
libbsp_a_SOURCES += ../../sparc/shared/analog/gradcdac.c

# Memory controllers
libbsp_a_SOURCES += ../../sparc/shared/mem/mctrl.c
libbsp_a_SOURCES += ../../sparc/shared/mem/mctrl_rmap.c

# timer
libbsp_a_SOURCES += timer/timer.c

# TM/TC
include_HEADERS += ../../sparc/shared/include/grtc.h
include_HEADERS += ../../sparc/shared/include/grtm.h
libbsp_a_SOURCES += ../../sparc/shared/tmtc/grtc.c
libbsp_a_SOURCES += ../../sparc/shared/tmtc/grtc_rmap.c
libbsp_a_SOURCES += ../../sparc/shared/tmtc/grtm.c
libbsp_a_SOURCES += ../../sparc/shared/tmtc/grtm_rmap.c

# Driver Manager
include_drvmgrdir = $(includedir)/drvmgr
include_drvmgr_HEADERS = ../../sparc/shared/include/drvmgr/ambapp_bus.h
include_drvmgr_HEADERS += ../../sparc/shared/include/drvmgr/ambapp_bus_rmap.h
include_drvmgr_HEADERS += ../../sparc/shared/include/drvmgr/leon2_amba_bus.h
include_drvmgr_HEADERS += ../../sparc/shared/include/drvmgr/spw_bus.h
include_drvmgr_HEADERS += ../../sparc/shared/include/drvmgr/spw_bus_ids.h
libbsp_a_SOURCES += ../../sparc/shared/drvmgr/ambapp_bus.c
libbsp_a_SOURCES += ../../sparc/shared/drvmgr/ambapp_bus_leon2.c
libbsp_a_SOURCES += ../../sparc/shared/drvmgr/ambapp_bus_rmap.c
libbsp_a_SOURCES += ../../sparc/shared/drvmgr/leon2_amba_bus.c
libbsp_a_SOURCES += ../../sparc/shared/drvmgr/spw_bus.c

if HAS_NETWORKING
noinst_PROGRAMS += leon_smc91111.rel
leon_smc91111_rel_SOURCES = leon_smc91111/leon_smc91111.c
leon_smc91111_rel_CPPFLAGS = $(AM_CPPFLAGS)
leon_smc91111_rel_CPPFLAGS += -D__INSIDE_RTEMS_BSD_TCPIP_STACK__
leon_smc91111_rel_LDFLAGS = $(RTEMS_RELLDFLAGS)
endif

if HAS_NETWORKING
noinst_PROGRAMS += leon_open_eth.rel
leon_open_eth_rel_SOURCES = leon_open_eth/leon_open_eth.c
leon_open_eth_rel_CPPFLAGS = $(AM_CPPFLAGS)
leon_open_eth_rel_CPPFLAGS +=  -D__INSIDE_RTEMS_BSD_TCPIP_STACK__
leon_open_eth_rel_LDFLAGS = $(RTEMS_RELLDFLAGS)
endif

if HAS_NETWORKING
noinst_PROGRAMS += leon_greth.rel
leon_greth_rel_SOURCES = ../../sparc/shared/net/greth.c
include_HEADERS += ../../sparc/shared/include/greth.h
leon_greth_rel_CPPFLAGS = $(AM_CPPFLAGS)
leon_greth_rel_CPPFLAGS += -D__INSIDE_RTEMS_BSD_TCPIP_STACK__
leon_greth_rel_LDFLAGS = $(RTEMS_RELLDFLAGS)

# BSP Network configuration
include_HEADERS += ../../sparc/shared/include/network_interface_add.h
libbsp_a_SOURCES += ../../sparc/shared/net/network_interface_add.c
endif

libbsp_a_LIBADD = \
    ../../../libcpu/@RTEMS_CPU@/access.rel \
    ../../../libcpu/@RTEMS_CPU@/cache.rel \
    ../../../libcpu/@RTEMS_CPU@/reg_win.rel \
    ../../../libcpu/@RTEMS_CPU@/syscall.rel

if HAS_NETWORKING
libbsp_a_LIBADD += leon_greth.rel
libbsp_a_LIBADD += leon_open_eth.rel
libbsp_a_LIBADD += leon_smc91111.rel
endif

EXTRA_DIST = times

include $(srcdir)/preinstall.am
include $(top_srcdir)/../../../../automake/local.am
