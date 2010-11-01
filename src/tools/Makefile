
PROJECT_ROOT = ../..
SOURCE_SUBDIRS = shmw
include $(PROJECT_ROOT)/src/rules.mk

.PHONY: $(SOURCE_SUBDIRS)

$(SOURCE_SUBDIRS):
	$(V)for mdir in $(SOURCE_SUBDIRS); do make -C $$mdir; done

all: $(SOURCE_SUBDIRS)

.PHONY: clobber
clobber:
	$(V)for mdir in $(SOURCE_SUBDIRS); do make -C $$mdir clobber; done
.PHONY: clean
clean: clobber
