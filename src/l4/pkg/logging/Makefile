PKGDIR          ?= .
L4DIR           ?= $(PKGDIR)/../..

#LDFLAGS += --wrap=malloc --wrap=free --wrap=realloc

TARGET                   = logging-client logging-server
SRC_CC_logging-server    = server.cc
SRC_CC_logging-client    = client.cc
REQUIRES_LIBS            = cxx_libc_io cxx_io libstdc++ libc_be_mem
DEPENDS_PKGS             = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk
