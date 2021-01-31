PKGDIR		?= ..
L4DIR		?= $(PKGDIR)/../..

LDFLAGS               += --wrap=malloc --wrap=free --wrap=realloc
TARGET := include include-acpica src

include $(L4DIR)/mk/subdir.mk

src: include include-acpica
