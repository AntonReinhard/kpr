PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET           = ex_ipc1
SRC_C		 = ipc_example.c
REQUIRES_LIBS    = libpthread
REQUIRES_LIBS           += libc_be_mem
DEPENDS_PKGS     = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk
