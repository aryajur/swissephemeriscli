LUA_PKG ?= luajit
# Optionally set LUA_DIR to a prefix containing inc/ and lib/
# Example: make LUA_DIR=$(HOME)/lj
LUA_DIR ?=~/lj
LUA_RPATH ?=

# Use CURDIR (make's cwd) instead of inherited shell PWD
SWE_DIR ?= $(CURDIR)/swisseph-src
# Handle both GitHub and traditional directory structures
SWE_LIB := $(shell if [ -f "$(SWE_DIR)/libswe.a" ]; then echo "$(SWE_DIR)/libswe.a"; else echo "$(SWE_DIR)/src/libswe.a"; fi)

# Allow overrides
LUA_INCLUDE ?= $(shell pkg-config --cflags $(LUA_PKG) 2>/dev/null)
LUA_LIBS ?= $(shell pkg-config --libs $(LUA_PKG) 2>/dev/null)

# If pkg-config did not provide flags and LUA_DIR is set, detect headers and lib names
ifeq ($(strip $(LUA_INCLUDE)),)
ifneq ($(strip $(LUA_DIR)),)
  ifneq (,$(wildcard $(LUA_DIR)/inc/lua.h))
    LUA_INCLUDE += -I$(LUA_DIR)/inc
  else ifneq (,$(wildcard $(LUA_DIR)/lua.h))
    LUA_INCLUDE += -I$(LUA_DIR)
  else ifneq (,$(wildcard $(LUA_DIR)/include/lua.h))
    LUA_INCLUDE += -I$(LUA_DIR)/include
  else ifneq (,$(wildcard $(LUA_DIR)/include/luajit-2.1/lua.h))
    LUA_INCLUDE += -I$(LUA_DIR)/include/luajit-2.1
  else ifneq (,$(wildcard $(LUA_DIR)/include/luajit-2.0/lua.h))
    LUA_INCLUDE += -I$(LUA_DIR)/include/luajit-2.0
  endif
endif
endif

ifeq ($(strip $(LUA_LIBS)),)
ifneq ($(strip $(LUA_DIR)),)
  LUA_RPATH ?= $(LUA_DIR)/lib
  ifneq (,$(wildcard $(LUA_DIR)/lib/libluajit.so))
    LUA_LIBS += -L$(LUA_DIR)/lib -lluajit
  else ifneq (,$(wildcard $(LUA_DIR)/lib/liblua5.4*.so))
    LUA_LIBS += -L$(LUA_DIR)/lib -llua5.4
  else ifneq (,$(wildcard $(LUA_DIR)/lib/liblua54*.so))
    LUA_LIBS += -L$(LUA_DIR)/lib -llua54
  else ifneq (,$(wildcard $(LUA_DIR)/lib/liblua5.3*.so))
    LUA_LIBS += -L$(LUA_DIR)/lib -llua5.3
  else ifneq (,$(wildcard $(LUA_DIR)/lib/liblua5.2*.so))
    LUA_LIBS += -L$(LUA_DIR)/lib -llua5.2
  else ifneq (,$(wildcard $(LUA_DIR)/lib/liblua5.1*.so))
    LUA_LIBS += -L$(LUA_DIR)/lib -llua5.1
  else ifneq (,$(wildcard $(LUA_DIR)/lib/liblua.so))
    LUA_LIBS += -L$(LUA_DIR)/lib -llua
  else
    # Fallback by LUA_PKG convention
    ifeq ($(LUA_PKG),luajit)
      LUA_LIBS += -L$(LUA_DIR)/lib -lluajit
    else ifeq ($(LUA_PKG),lua5.4)
      LUA_LIBS += -L$(LUA_DIR)/lib -llua5.4
    else ifeq ($(LUA_PKG),lua5.3)
      LUA_LIBS += -L$(LUA_DIR)/lib -llua5.3
    else ifeq ($(LUA_PKG),lua5.2)
      LUA_LIBS += -L$(LUA_DIR)/lib -llua5.2
    else
      LUA_LIBS += -L$(LUA_DIR)/lib -llua5.1
    endif
  endif
endif
endif

CFLAGS += -O2 -fPIC $(LUA_INCLUDE)
LDFLAGS += -shared
ifneq ($(strip $(LUA_RPATH)),)
LDFLAGS += -Wl,-rpath,$(LUA_RPATH)
endif
LIBS += $(SWE_LIB) -lm $(LUA_LIBS)

all: swisseph.so

swisseph.so: lua_swisseph.o $(SWE_LIB)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

lua_swisseph.o: lua_swisseph.c
	$(CC) $(CFLAGS) -I$(SWE_DIR) $(shell if [ -d "$(SWE_DIR)/src" ]; then echo "-I$(SWE_DIR)/src"; fi) -c -o $@ $<

clean:
	rm -f lua_swisseph.o swisseph.so

.PHONY: all clean
