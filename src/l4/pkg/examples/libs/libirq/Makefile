PKGDIR ?=	../..
L4DIR ?=	$(PKGDIR)/../..

SRC_C_ex_libirq_async = async_isr.c
SRC_C_ex_libirq_loop  = loop.c
TARGET                = ex_libirq_async ex_libirq_loop
REQUIRES_LIBS         = libirq libio
REQUIRES_LIBS           += libc_be_mem
DEPENDS_PKGS          = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk
