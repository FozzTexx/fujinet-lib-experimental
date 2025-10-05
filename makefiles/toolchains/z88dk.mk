CC_DEFAULT ?= zcc
AS_DEFAULT ?= z80asm
LD_DEFAULT ?= $(CC_DEFAULT)
AR_DEFAULT ?= $(AS_DEFAULT)

include $(MWD)/tc-common.mk

CFLAGS += +coleco -subtype=adam
AFLAGS +=
LDFLAGS += +coleco -subtype=adam

define include-dir-flag
  -I$1
endef

define asm-include-dir-flag
  -I$1
endef

define library-dir-flag
  -L$1
endef

define library-flag
  -l$1
endef

define link-lib
  $(AR) -xeos $1 $2
endef

define link-bin
  $(LD) $(LDFLAGS) $2 $(LIBS) -o $1
endef

define compile
  $(CC) -c $(CFLAGS) -o $1 $2
endef

define assemble
  $(AS) -c $(AFLAGS) -o $1 $2
endef
