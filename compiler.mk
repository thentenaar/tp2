#
# tp2 - Tiled Powers of 2
# Copyright (C) 2015 Tim Hentenaar.
#
# This code is licenced under the Simplified BSD License.
# See the LICENSE file for details.
#
CFLAGS?=

##########################################################################
# compiler.mk - Check for compiler type and features
#
# Options:
#
# C_STANDARD - Set this to your desired standard. ANSI C is the default.
#
##########################################################################

ifeq ($(origin COMPILER_MK), undefined)

# Protect against double-inclusion.
COMPILER_MK=1

# Check for cc type and version
ifneq (,$(CC))
  IS_GCC   := $(shell $(CC) -v 2>&1 | tail -1 | grep 'gcc version')
  IS_CLANG := $(shell $(CC) -v 2>&1 | head -1 | grep 'clang version')

  # Check for gcc
  ifneq (,$(IS_GCC))
    GCC_VERSION := $(shell $(CC) -v 2>&1 | tail -1 | cut -d' ' -f 3)
    GCC_MAJOR   := $(shell echo "$(GCC_VERSION)" | cut -d. -f 1)
    GCC_MINOR   := $(shell echo "$(GCC_VERSION)" | cut -d. -f 2)
    GCC_PATCH   := $(shell echo "$(GCC_VERSION)" | cut -d. -f 3)
  endif

  # Check for clang
  ifneq (,$(IS_CLANG))
    CLANG_VERSION := $(shell $(CC) -v 2>&1 | head -1 | cut -d' ' -f 3)
    CLANG_MAJOR   := $(shell echo "$(CLANG_VERSION)" | cut -d. -f 1)
    CLANG_MINOR   := $(shell echo "$(CLANG_VERSION)" | cut -d. -f 2)
    CLANG_PATCH   := $(shell echo "$(CLANG_VERSION)" | cut -d. -f 3)
  endif

  # Is it gcc or clang?
  IS_GCC_OR_CLANG := $(IS_GCC)$(IS_CLANG)
endif

# Print out a warning if we don't know what $(CC) is.
ifeq (,$(IS_GCC_OR_CLANG))
$(warning compiler $(CC) not recognized.)
endif

# Set the standards compliance mode
XCFLAGS?=
ifneq (,$(IS_GCC_OR_CLANG))
  # Enable ANSI mode by default.
  ifeq (,$(C_STANDARD))
    XCFLAGS += -ansi
  else
    # C89 / C90 are synonyms for -ansi
    ifeq (c89,$(C_STANDARD))
      XCFLAGS += -ansi
    else
      ifeq (c90,$(C_STANDARD))
        XCFLAGS += -ansi
      else
        XCFLAGS += -std=$(C_STANDARD)
      endif
    endif
  endif
endif

# Figure out CFLAGS
ifneq (,$(IS_GCC))
  # These should work on gcc >= 2.95. '-W' has been called '-Wextra'
  # since gcc 3.4.4, however '-W' is still valid.
  XCFLAGS += -pedantic -Werror -Wall -W -Wconversion -Wstrict-prototypes
  XCFLAGS += -Wmissing-prototypes -Wmissing-declarations -Wnested-externs
  XCFLAGS += -Wshadow -Wcast-align -Wwrite-strings -Wcomment -Wcast-qual
  XCFLAGS += -Wbad-function-cast -Wredundant-decls

  # -Wformat-security was introduced in gcc 3.0.4
  ifeq ($(shell test $(GCC_MAJOR) -ge 4; echo $$?), 0)
    XCFLAGS += -Wformat-security
  else
    ifeq ($(GCC_MAJOR), 3)
      XCFLAGS += -Wformat-security
    endif
  endif

  # Warn about things not present in ANSI C.
  ifeq ($(shell test $(GCC_MAJOR) -ge 5; echo $$?), 0)
  ifneq (,$(findstring -ansi,$(CFLAGS)))
    XCFLAGS += -Wc90-c99-compat
  endif
  endif
endif

# For clang, turn on every warning, then remove a few.
ifneq (,$(IS_CLANG))
  XCFLAGS += -pedantic -Weverything -Werror -Wno-padded -Wno-switch-enum
  XCFLAGS += -Wno-missing-variable-declarations -Wno-format-nonliteral
  XCFLAGS += -Wno-disabled-macro-expansion -Qunused-arguments
endif

# Add our flags to CFLAGS
CFLAGS += $(XCFLAGS)
export XCFLAGS
export COMPILER_MK
else
  CFLAGS += $(XCFLAGS)
endif # COMPILER_MK

