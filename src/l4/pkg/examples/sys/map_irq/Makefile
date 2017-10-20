PKGDIR          ?= ../..
L4DIR           ?= $(PKGDIR)/../..

TARGET                   = ex_map_irq_client ex_map_irq_server
SRC_CC_ex_map_irq_client = client.cc
SRC_CC_ex_map_irq_server = server.cc
REQUIRES_LIBS += libc_be_mem

include $(L4DIR)/mk/prog.mk
