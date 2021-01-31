PKGDIR          ?= ../..
L4DIR           ?= $(PKGDIR)/../..

TARGET           = ex_uirq
SRC_CC           = ex_uirq.cc
REQUIRES_LIBS    = libstdc++ libpthread
REQUIRES_LIBS           += libc_be_mem
DEPENDS_PKGS     = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk
