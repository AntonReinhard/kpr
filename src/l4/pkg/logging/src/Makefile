PKGDIR	= ..
L4DIR	?= $(PKGDIR)/../..

CPPFLAGS += -Wno-pointer-arith

TARGET                   = logging-client logging-server
SRC_CC_logging-server    = server.cc
SRC_CC_logging-client    = client.cc
REQUIRES_LIBS            = cxx_libc_io cxx_io libstdc++ libc_be_mem libgfxbitmap
DEPENDS_PKGS             = $(REQUIRES_LIBS)

include $(L4DIR)/mk/prog.mk
