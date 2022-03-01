PKGDIR          ?= .
L4DIR           ?= $(PKGDIR)/../..

TARGET                   = helloworld-client helloworld-server
SRC_CC_helloworld-server = server.cc
SRC_CC_helloworld-client = client.cc
REQUIRES_LIBS            = cxx_libc_io cxx_io libstdc++ libc_be_mem
DEPENDS_PKGS             = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk

