PROJECT_ROOT = $(HPHP_HOME)

SWIG_FILE = $(shell echo `find ffi -name "*.i"`)
NAME = $(patsubst ffi/%.i,%,$(SWIG_FILE))

WRAP_FILE = $(patsubst %.i,%_wrap.c,$(SWIG_FILE))

CPPFLAGS += -I. -I/usr/local/include/python2.6
LIBS = $(HPHP_LIB)/libhphp_runtime.a $(ALL_LIBS)

include $(HPHP_HOME)/src/rules.mk

SOURCES = $(WRAP_FILE)
EXTERNAL += $(HPHP_LIB)/lib$(NAME).so
SHARED_LIB = ffi/python/_$(NAME).so

all : $(SHARED_LIB)

$(WRAP_FILE) : $(SWIG_FILE)
	$(V)mkdir -p ffi/python
	$(V)swig -python -outdir ffi/python -I$(HPHP_HOME)/src/ffi/swig $(SWIG_FILE)

$(SHARED_LIB) : $(WRAP_FILE)
	$(V)$(CXX) -shared -fPIC $(CPPFLAGS) $(CXXFLAGS) $(DEBUG_SYMBOL) -Wall -Werror \
    -Wl,-soname,_$(NAME).so -o ffi/python/_$(NAME).so $^ $(EXTERNAL)
