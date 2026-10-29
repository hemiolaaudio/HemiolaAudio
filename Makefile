# If RACK_DIR is not defined when calling the Makefile, default to the SDK next to this folder.
RACK_DIR ?= ../Rack-SDK

# Header-only logic cores live in core/ and know nothing about Rack.
FLAGS += -Icore
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment.
LDFLAGS +=

SOURCES += $(wildcard src/*.cpp)

DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)

include $(RACK_DIR)/plugin.mk
