PKGDIR ?=	../..
L4DIR ?=	$(PKGDIR)/../..

SRC_C           = main.c
TARGET          = rtc_test
DEPENDS_PKGS    = rtc
REQUIRES_LIBS           += libc_be_mem
REQUIRES_LIBS   = rtc
REQUIRES_LIBS += libc_be_mem

include $(L4DIR)/mk/prog.mk
