PKGDIR		?= ..
L4DIR		?= $(PKGDIR)/../..

vpath %.bmp ..

MODE		= static
#MODE		= shared
TARGET		= pong-client
SRC_CC		= main.cc
LIBS           +=  -lcxx_base -lcxx_thread_e -lcxx_libc_io_e

REQUIRES_LIBS += libpthread cxx_io cxx_base libstdc++ libc_be_mem

include $(L4DIR)/mk/prog.mk
