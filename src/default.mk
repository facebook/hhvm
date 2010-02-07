###############################################################################
#
# Predefined Targets
#
# Author: hzhao (09/2007)
#
###############################################################################
#
# This file is included as sub-make in rules.mk. It is slower than defining
# these targets inline in each Makefile, but it just saves a lot of work
# preparing these targets for each directory.

unexport SUB_CLEAN_DIRS

# delete all intermediate files and built targets
.PHONY: clobber
clobber:
	@$(RM) $(SUB_INTERMEDIATE_FILES) $(SUB_OBJECTS)
	@$(RM) *.merge-left.* *.merge-right.* *.working www.pid
	@$(RM) lib$(PROJECT_NAME).so lib$(PROJECT_NAME).a *~ $(OBJECTS:.o=)
	@$(RM) $(filter-out $(SUB_PROGRAMS) $(SUB_LIB_TARGETS), $(TARGETS))
	@$(RM) $(shell echo `find . -name "*.o"`)
	@$(RM) $(shell echo `find . -name "*.d"`)
	@$(RM) $(shell echo `find . -name "*~"`)
	@$(RMDIR) gen-cpp
	@for mdir in $(SUB_CLEAN_DIRS); do $(MAKE) -C $$mdir clobber; done
	@for mdir in $(SUB_PROGRAMS); do $(MAKE) -C $$mdir clobber; done
	@for mdir in $(SUB_LIB_TARGETS); do $(MAKE) -C $$mdir clobber; done
	@for mdir in $(INTERMEDIATE_DIRS); do rm -fR $$mdir; done

.PHONY: clean
clean: clobber

# delete targets only
.PHONY: clear-targets
cleartargets:
	@$(RM) $(TARGETS)
	@for mdir in $(SUB_PROGRAMS); do $(MAKE) -C $$mdir cleartargets; done
	@for mdir in $(SUB_LIB_TARGETS); do $(MAKE) -C $$mdir cleartargets; done

# default no-op "make install"
.PHONY: install
install:
	@for mdir in $(SUB_PROGRAMS); do $(MAKE) -C $$mdir install; done
	@for mdir in $(SUB_LIB_TARGETS); do $(MAKE) -C $$mdir install; done

.PHONY: list-targets
list-targets:
	@echo $(TARGETS) | tr ' ' '\n'
	@for mdir in $(SUB_PROGRAMS); do $(MAKE) -C $$mdir list-targets; done
	@for mdir in $(SUB_LIB_TARGETS); do $(MAKE) -C $$mdir list-targets; done

.PHONY: list-sources
list-sources:
	@echo $(SOURCES) | tr ' ' '\n'
	@for mdir in $(SUB_PROGRAMS); do $(MAKE) -C $$mdir list-sources; done
	@for mdir in $(SUB_LIB_TARGETS); do $(MAKE) -C $$mdir list-sources; done

.EXPORT_ALL_VARIABLES:;
unexport SUB_PROGRAMS SUB_LIB_TARGETS
