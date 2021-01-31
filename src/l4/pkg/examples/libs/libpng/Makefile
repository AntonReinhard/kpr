PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= ex_libpng
SRC_C		= main.c
REQUIRES_LIBS   = libl4png_wrap l4re_c-util
REQUIRES_LIBS           += libc_be_mem
DEPENDS_PKGS    = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk
