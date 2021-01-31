PKGDIR          ?= ../..
L4DIR           ?= $(PKGDIR)/../../..

TARGET        = ned-prompt
SRC_CC        = ned-prompt.cc
REQUIRES_LIBS := readline
DEPENDS_PKGS  := readline
LDFLAGS      += --wrap=malloc --wrap=free --wrap=realloc

include $(L4DIR)/mk/prog.mk
