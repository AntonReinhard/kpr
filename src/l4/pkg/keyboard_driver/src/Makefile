PKGDIR          ?= ..
L4DIR           ?= $(PKGDIR)/../..

KBUILD_CFLAGS += -Wno-pointer-arith

TARGET                   = keyboard-driver
SRC_CC_keyboard-driver   = driver.cc
REQUIRES_LIBS            = cxx_libc_io cxx_io libstdc++ libc_be_mem l4re_c-util libio-io
DEPENDS_PKGS             = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk
