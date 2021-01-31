PKGDIR          ?= ..
L4DIR           ?= $(PKGDIR)/../..

TARGET                   = ex_clntsrv-server ex_clntsrv-client
SRC_CC_ex_clntsrv-server = server.cc
SRC_CC_ex_clntsrv-client = client.cc
REQUIRES_LIBS            = cxx_libc_io cxx_io
REQUIRES_LIBS           += libc_be_mem
DEPENDS_PKGS             = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk

