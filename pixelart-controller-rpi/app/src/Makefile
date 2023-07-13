#
# Makefile
#

CIRCLEHOME = ../..

OBJS	= main.o kernel.o

LIBS	= $(CIRCLEHOME)/lib/libcircle.a \
	  $(CIRCLEHOME)/lib/net/libnet.a \
	  $(CIRCLEHOME)/lib/sched/libsched.a \
	  ../lib/ds3231/libds3231.a

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)
