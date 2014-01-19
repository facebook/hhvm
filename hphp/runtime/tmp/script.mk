
PROJECT_ROOT = $(HPHP_HOME)

include $(PROJECT_ROOT)/src/rules.mk

all :
	echo "#/bin/sh" > run.sh
	echo "export LD_LIBRARY_PATH=\`dirname \$${!#}\`:.:\$$HPHP_LIB" >> run.sh
	for i in $(LIB_PATHS); do \
	  test "$$i" '!=' "$$HPHP_LIB" && echo "LD_LIBRARY_PATH=\$${LD_LIBRARY_PATH}:$$i" >> run.sh; \
	done
	echo "runtime/tmp/run \$${@:1:\$$((\$$#-1))}" >> run.sh
