PKGDIR ?=	../..
L4DIR ?=	$(PKGDIR)/../..

TARGET        = ex_shmc
SRC_C         = prodcons.c
REQUIRES_LIBS = shmc libpthread
REQUIRES_LIBS           += libc_be_mem
DEPENDS_PKGS  = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk
