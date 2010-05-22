PROJECT_ROOT = ../../../

PROJECT_NAME = hphp_java

SRC = $(shell echo `find hphp -name '*.java'`)

CLS = $(shell echo $(patsubst %.java, %.class, $(SRC)) \
        | sed -e "s/hphp/classes\/hphp/g")

HEADER = hphp_ffi_java.h

AUTO_SOURCES = 1

include $(PROJECT_ROOT)/src/rules.mk

EXTERNAL += $(LIB_DIR)/libhphp_runtime.so

TARGETS = $(SHARED_LIB)

all : $(CLS) $(HEADER) $(TARGETS)

$(OBJECTS) : $(HEADER)

JAVA_PATH = /usr/local/jdk-6u7-64

CPPFLAGS += -I $(JAVA_PATH)/include -I $(JAVA_PATH)/include/linux \

$(CLS) : $(SRC)
	$(V)mkdir -p classes
	$(V)$(JAVA_PATH)/bin/javac -sourcepath . -d classes $^

${HEADER} : ${CLS}
	$(V)$(JAVA_PATH)/bin/javah -classpath classes -o ${HEADER} \
  `echo ${CLS} | sed -e 's/classes\///g' -e 's/\.class//g' -e 's/\//\./g'`

clobber : clean
	$(V)rm -rf $(SHARED_LIB) classes $(HEADER)
