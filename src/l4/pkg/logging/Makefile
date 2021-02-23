PKGDIR          ?= .
L4DIR           ?= $(PKGDIR)/../..

TARGET                   = logging-client logging-server
SRC_CC_logging-server    = server.cc
SRC_CC_logging-client    = client.cc
REQUIRES_LIBS            = cxx_libc_io cxx_io libstdc++
DEPENDS_PKGS             = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk
