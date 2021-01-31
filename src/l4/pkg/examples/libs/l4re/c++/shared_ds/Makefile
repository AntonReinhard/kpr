PKGDIR                ?= ../../../..
L4DIR                 ?= $(PKGDIR)/../..

TARGET                 = ex_l4re_ds_clnt ex_l4re_ds_srv
REQUIRES_LIBS           += libc_be_mem
SRC_CC_ex_l4re_ds_clnt = ds_clnt.cc
SRC_CC_ex_l4re_ds_srv  = ds_srv.cc

include $(L4DIR)/mk/prog.mk
