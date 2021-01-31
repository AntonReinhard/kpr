PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= ex_eb_leds
SRC_C		= eb_leds.c
REQUIRES_LIBS   = libio
REQUIRES_LIBS           += libc_be_mem
DEPENDS_PKGS    = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk
