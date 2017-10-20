PKGDIR          ?= ../../..
L4DIR           ?= $(PKGDIR)/../..

TARGET                = ex_smap-server ex_smap-client
SRC_CC_ex_smap-server = server.cc
SRC_CC_ex_smap-client = client.cc
REQUIRES_LIBS += libc_be_mem

include $(L4DIR)/mk/prog.mk

