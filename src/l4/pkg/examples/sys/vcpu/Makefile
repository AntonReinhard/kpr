PKGDIR		?= ../..
L4DIR		?= $(PKGDIR)/../..

TARGET		= ex_vcpu
SRC_CC		= vcpu.cc
REQUIRES_LIBS   = libvcpu cxx_io cxx_libc_io
REQUIRES_LIBS           += libc_be_mem
DEPENDS_PKGS    = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk
