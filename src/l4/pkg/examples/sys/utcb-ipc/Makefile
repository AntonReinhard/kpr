PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= ex_utcb_ipc
REQUIRES_LIBS   = l4re_c-util
REQUIRES_LIBS           += libc_be_mem
DEPENDS_PKGS    = $(REQUIRES_LIBS)
SRC_C		= main.c

include $(L4DIR)/mk/prog.mk
