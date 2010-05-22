JAVA_PATH = /usr/local/jdk-6u7-64
JAVA_FFI_CP = $(HPHP_HOME)/src/ffi/java/classes

JAVA_CLASSES = \
  $(shell find ffi -name '*.java' | sed -e 's/\.java/\.class/g')

#include $(HPHP_HOME)/src/rules.mk

all : $(JAVA_CLASSES)

%.class: %.java
	cd ffi && \
  $(JAVA_PATH)/bin/javac -cp $(JAVA_FFI_CP):. -sourcepath . \
    $(patsubst ffi/%, %, $^)
