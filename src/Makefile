# target marcros
TARGET := AIService
MAIN_SRC := main.cpp

# compile marcros
DIRS := sys_ctrl base_fun local_fun third_fun usr_apps tools
OBJS := 

# intermedia compile marcros
ALL_OBJS := 
CLEAN_FILES := $(TARGET) $(OBJS)
DIST_CLEAN_FILES := $(OBJS)

# recursive wildcard
rwildcard=$(foreach d,$(wildcard $(addsuffix *,$(1))),$(call rwildcard,$(d)/,$(2))$(filter $(subst *,%,$(2)),$(d)))

# default target
default: show-info all

# non-phony targets
#@echo -e "\t" CC $(ALL_OBJS) $(MAIN_SRC) -o $@ $(CCFLAG)
$(TARGET): build-subdirs $(OBJS) find-all-objs
	@echo -e "\t" CC $@
	@$(CC) $(ALL_OBJS) $(MAIN_SRC) -o $@ $(CCFLAG) 

# phony targets
.PHONY: all
all: $(TARGET)
	@echo Target $(TARGET) build finished.

.PHONY: clean
clean: clean-subdirs
	@echo CLEAN $(CLEAN_FILES)
	@rm -f $(CLEAN_FILES)

.PHONY: distclean
distclean: clean-subdirs
	@echo CLEAN $(DIST_CLEAN_FILES)
	@rm -f $(DIST_CLEAN_FILES)

# phony funcs
.PHONY: find-all-objs
find-all-objs:
	$(eval ALL_OBJS += $(call rwildcard,$(DIRS),*.o))

.PHONY: show-info
show-info:
	@echo Building Project

# need to be placed at the end of the file
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
export ROOT_DIR := $(shell pwd)
export PROJECT_PATH := $(patsubst %/,%,$(dir $(mkfile_path)))
export MAKE_INCLUDE=$(PROJECT_PATH)/mkconfig/make.global
export SUB_MAKE_INCLUDE=$(PROJECT_PATH)/mkconfig/submake.global
include $(MAKE_INCLUDE)
