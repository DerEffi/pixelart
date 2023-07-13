#
# Makefile
#

CIRCLEHOME = ../../..

OBJS	= ds3231.o

libds3231.a: $(OBJS)
	@echo "  AR    $@"
	@rm -f $@
	@$(AR) cr $@ $(OBJS)

include $(CIRCLEHOME)/Rules.mk

-include $(DEPS)
