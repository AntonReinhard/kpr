PKGDIR ?=	../..
L4DIR ?=	$(PKGDIR)/../..

TARGET           = ex_libio
SRC_C            = main.c
REQUIRES_LIBS    = libio libirq
REQUIRES_LIBS           += libc_be_mem
DEPENDS_PKGS     = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk
