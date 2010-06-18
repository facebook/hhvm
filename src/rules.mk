###############################################################################
#
# Master Makefile Rules
#
# Author: hzhao (09/2007)
#
###############################################################################
#
# If you want to take advantage of this rules.mk, define one of these prefined
# project types, and there is very minimum Makefile lines to write.
#
#    EXCLUDES =              # any .c or .cpp files to exclude from build
#    include rules.mk
#    all: $(TARGETS)         # one should always have this line unchanged
#
# 1. Static Library Project:
#
#    PROJECT_NAME = xxx      # final lib is named as libxxx.a
#    TARGETS = $(STATIC_LIB) # add more targets than libxxx.a
#
# 2. Shared Library Project:
#
#    PROJECT_NAME = xxx      # final lib is named as libxxx.so
#    TARGETS = $(SHARED_LIB) # add more targets than libxxx.so
#
# 3. Application Project:
#
#    PROJECT_NAME = xxx      # final executable is named as xxx
#    TARGETS = $(APP_TARGET) # add more targets than xxx
#
# 4. Multi-Target Project:
#
#    CODEGEN_TARGETS = <list of code generation targets>
#    LIB_TARGETS = <list of subdirs that build libraries>
#    PROGRAMS = <list of subdirs that build executables>
#    TARGETS = $(PROGRAMS)   # add more targets than xxx
#
# 5. Mono-Target Project:
#
#    CODEGEN_TARGETS = <list of code generation targets>
#    LIB_TARGETS = <list of subdirs that build libraries>
#    MONO_TARGETS = <each of .cpp builds one executable>
#    TARGETS = $(MONO_TARGETS) # add more targets than xxx
#
# The following targets are automatically defined:
#
#  make clobber: delete all intermediate files and built targets
#  make clean: delete all intermediate files without deleting targets
#  make cleartargets: delete targets only
#  (check default.mk for more)
#
# If there are extra files to remove when "make clobber" or "make clean", add
# them to $(INTERMEDIATE_FILES).
#
###############################################################################
# Machine specific information

OS := $(shell head -1 /etc/issue | cut -d' ' -f1)

ifeq ($(OS), CentOS)

ifndef HPHP_DEV
OS = centos
else
OS = centos-dev
endif

else
OS = fedora
endif

GCC_VERSION := $(shell gcc --version | head -1 | cut -d ' ' -f3)

###############################################################################
# Directories an command line switches

include $(PROJECT_ROOT)/src/dirs.mk

###############################################################################
# Source Files

ifdef AUTO_SOURCES

CXX_NOOPT_SOURCES += \
  $(filter-out $(GENERATED_CXX_NOOPT_SOURCES), \
	$(wildcard *.no.cpp) \
	$(wildcard $(patsubst %, %/*.no.cpp, $(SOURCE_SUBDIRS))))

CXX_SOURCES += \
  $(filter-out $(GENERATED_CXX_SOURCES) $(CXX_NOOPT_SOURCES), \
	$(wildcard *.cpp) \
	$(wildcard $(patsubst %, %/*.cpp, $(SOURCE_SUBDIRS))))

C_SOURCES += \
  $(filter-out $(GENERATED_C_SOURCES) $(GENERATED_CPP_SOURCES), \
	$(wildcard *.c) \
	$(wildcard $(patsubst %, %/*.c, $(SOURCE_SUBDIRS))))

endif

ifdef AUTO_SOURCES_RECURSIVE

CXX_NOOPT_SOURCES += \
  $(filter-out $(GENERATED_CXX_NOOPT_SOURCES), \
	$(shell find . -name "*.no.cpp"))

CXX_SOURCES += \
  $(filter-out $(GENERATED_CXX_SOURCES) $(CXX_NOOPT_SOURCES), \
	$(shell find . -name "*.cpp"))

C_SOURCES += \
  $(filter-out $(GENERATED_C_SOURCES) $(GENERATED_CPP_SOURCES), \
	$(shell find . -name "*.c"))

endif

GENERATED_SOURCES = \
  $(GENERATED_CXX_NOOPT_SOURCES) \
  $(GENERATED_CXX_SOURCES) \
  $(GENERATED_C_SOURCES) \
  $(GENERATED_CPP_SOURCES)

ALL_SOURCES += \
  $(CXX_NOOPT_SOURCES) \
  $(CXX_SOURCES) \
  $(C_SOURCES) \
  $(GENERATED_SOURCES)

INTERMEDIATE_FILES += $(GENERATED_SOURCES) time_build.out
SOURCES += $(filter-out $(EXCLUDES), $(ALL_SOURCES))
OBJECTS += $(addprefix $(OUT_DIR),$(patsubst %.cpp, %.o, $(SOURCES:.c=.o)))
OBJECT_DIR_DEPS := $(if $(OUT_DIR),$(addsuffix .mkdir, \
	$(sort $(dir $(OBJECTS)))))
OBJECT_DIRS_REQUIRED := $(filter-out $(wildcard $(OBJECT_DIR_DEPS)), \
	$(OBJECT_DIR_DEPS))
ifneq ($(OBJECT_DIRS_REQUIRED),)
$(shell $(MKDIR) $(OBJECT_DIRS_REQUIRED))
endif

STATIC_LIB = $(LIB_DIR)/lib$(PROJECT_NAME).a
SHARED_LIB = $(LIB_DIR)/lib$(PROJECT_NAME).so
APP_TARGET = $(OUT_TOP)$(PROJECT_NAME)
MONO_TARGETS = $(filter-out $(PROJECT_NAME), $(patsubst %.cpp, %, $(wildcard *.cpp)))

# external shared libraries
EXTERNAL =

###############################################################################
# Compilation

ifneq ($(USE_CCACHE),)
override USE_CCACHE := $(wildcard /usr/bin/ccache)
endif

# To time compilation time and link time, run "TIME_BUILD=1 make -j1", and it
# will generate time_build.out for analysis.
ifdef TIME_BUILD
TIMECMD = /usr/bin/time -f "%e %C" -o time_build.out --append
NO_DISTCC = 1
USE_CCACHE :=
else
TIMECMD =
endif
ifneq ($(USE_CCACHE),)
ifndef NO_DISTCC
export CCACHE_PREFIX=distcc
endif
endif

PREFIX := $(TIMECMD)$(if $(USE_CCACHE), ccache,$(if $(NO_DISTCC),, distcc))

CC = $(PREFIX) gcc
CXX = $(PREFIX) g++

# Both $(CC) and $(CXX) will now generate .d dependency files.
CPPFLAGS += -MMD -fPIC

# Include frame pointers to make it easier to generate callgraphs in oprofile
CPPFLAGS += -fno-omit-frame-pointer

ifdef MAC_OS_X

CPPFLAGS += \
  -I /usr/local/include \
  -I /usr/local/include/boost-1_37 \
  -I /usr/local/mysql/include \

CXXFLAGS += \
  -DMAC_OS_X \
  -D_GNU_SOURCE \
  -ftemplate-depth-60 \

else

CPPFLAGS += \
  -isystem $(EXT_DIR)/binutils \
  -isystem $(EXT_DIR)/pcre/include \
  -isystem $(EXT_DIR)/libevent/include \
  -isystem $(EXT_DIR)/libcurl/include \
  -isystem $(EXT_DIR)/libafdt/include \
  -isystem $(EXT_DIR)/gd/include \
  -isystem /usr/local/include \
  -isystem $(EXT_DIR)/boost/include/boost-1_37 \
  -isystem $(EXT_DIR)/mysql/include \
  -isystem $(EXT_DIR)/sqlite/include \
  -isystem /usr/include/libxml2 \
  -isystem $(EXT_DIR)/libfbml/include \
  -isystem $(EXT_DIR)/libmbfl/include \
  -isystem $(EXT_DIR)/oniguruma/include \
  -isystem $(EXT_DIR)/oracle/include \
  -isystem $(EXT_DIR)/icu/include \
  -isystem $(EXT_DIR)/xhp/include \
  -isystem $(EXT_DIR)/libmcc/include \
  -isystem $(EXT_DIR)/libch/include \
  -isystem $(EXT_DIR)/timelib/include \
  -isystem $(EXT_DIR)/tbb/include \
  -isystem $(EXT_DIR)/libmcrypt/include \
  -isystem $(EXT_DIR)/libfbi/include \
  -I $(PROJECT_ROOT)/src \
  -I $(PROJECT_ROOT)/src/system/gen \

ifdef USE_JEMALLOC
CPPFLAGS += -isystem $(EXT_DIR)/jemalloc/include
endif

ifdef GOOGLE_CPU_PROFILER
GOOGLE_TOOLS = 1
endif
ifdef GOOGLE_HEAP_PROFILER
GOOGLE_TOOLS = 1
endif
ifdef GOOGLE_TCMALLOC
GOOGLE_TOOLS = 1
endif
ifdef GOOGLE_TOOLS
CPPFLAGS += -isystem $(EXT_DIR)/google-perftools/include
endif

CPPFLAGS += -D_GNU_SOURCE -D_REENTRANT=1 -D_PTHREADS=1 -pthread
CXXFLAGS += -ftemplate-depth-60

endif

ifndef NO_WALL
CXXFLAGS += -Wall -Woverloaded-virtual -Wno-deprecated -Wno-parentheses -Wno-strict-aliasing -Wno-write-strings -Wno-invalid-offsetof
endif

ifndef NO_WERROR
CXXFLAGS += -Werror
endif

ifndef NO_DEBUG_SYMBOL
DEBUG_SYMBOL = -g
else
DEBUG_SYMBOL =
endif
CPPFLAGS += $(DEBUG_SYMBOL)

ifdef HPHP_DEV
CPPFLAGS += -DHPHP_DEV
endif

ifdef DEBUG
CPPFLAGS += -DDEBUG
OPT =
endif

ifdef DEBUG_MEMORY_LEAK
CPPFLAGS += -DDEBUG_MEMORY_LEAK
OPT =
endif

ifdef DEBUG_APC_LEAK
CPPFLAGS += -DDEBUG_APC_LEAK
endif

ifdef DEBUG_RACE_CONDITION
CPPFLAGS += -DDEBUG_RACE_CONDITION
OPT =
endif

ifdef RELEASE
CPPFLAGS += -DRELEASE
ifdef WWW_INTERN
OPT = -Os
else
OPT = -O3
endif
endif

ifdef INFINITE_LOOP_DETECTION
CPPFLAGS += -DINFINITE_LOOP_DETECTION
endif
ifdef INFINITE_RECURSION_DETECTION
CPPFLAGS += -DINFINITE_RECURSION_DETECTION
endif
ifdef REQUEST_TIMEOUT_DETECTION
CPPFLAGS += -DREQUEST_TIMEOUT_DETECTION
endif
ifdef HOTPROFILER
CPPFLAGS += -DHOTPROFILER
endif
ifdef HOTPROFILER_NO_BUILTIN
CPPFLAGS += -DHOTPROFILER_NO_BUILTIN
endif
ifdef STACK_FRAME_INJECTION
CPPFLAGS += -DSTACK_FRAME_INJECTION
endif
ifdef ENABLE_LATE_STATIC_BINDING
CPPFLAGS += -DENABLE_LATE_STATIC_BINDING
endif

ifdef GOOGLE_CPU_PROFILER
CPPFLAGS += -DGOOGLE_CPU_PROFILER
endif
ifdef GOOGLE_HEAP_PROFILER
CPPFLAGS += -DGOOGLE_HEAP_PROFILER
endif
ifdef GOOGLE_TCMALLOC
CPPFLAGS += -DGOOGLE_TCMALLOC
endif
ifdef USE_JEMALLOC
CPPFLAGS += -DUSE_JEMALLOC
endif
ifdef PROFILE
CPPFLAGS += -pg
endif

ifdef COVERAGE
CPPFLAGS += -fprofile-arcs -ftest-coverage
endif

ifdef HPHP_BUILD_LIBRARY
CPPFLAGS += -DHPHP_BUILD_LIBRARY
endif

ifdef HPHP_BUILD_FFI
JAVA_PATH = /usr/local/jdk-6u7-64
CPPFLAGS += -I $(JAVA_PATH)/include -I $(JAVA_PATH)/include/linux
endif

ifdef NO_TLS
CPPFLAGS += -DNO_TLS
endif

# facebook specific stuff
CPPFLAGS += -DHAVE_PHPT

###############################################################################
# Linking

AR = $(TIMECMD) ar -crs
LD = $(TIMECMD) g++ -o

ifndef NO_GOLD
LD = $(TIMECMD) g++ -B$(EXT_DIR)/binutils/ -o
LDFLAGS += -Xlinker --export-dynamic -Xlinker --no-warn-search-mismatch
else
LDFLAGS += -rdynamic
endif

ifndef NO_RPATH
LDFLAGS += -Wl,-rpath -Wl,/usr/local/hphp/lib
endif

# Add library search paths here.
LDFLAGS	+= \
  -L$(LIB_DIR) \
  -L/usr/local/lib

ifdef PROFILE
LDFLAGS += -pg
endif

ifdef COVERAGE
LDFLAGS += -fprofile-arcs
endif

###############################################################################
# Libraries
#
# 1. Base Libraries
#
# These have to be libraries that nearly ALL programs need to link with. Do
# NOT add something that not everyone wants.

ifdef MAC_OS_X
LINK_LIBS = -lpthread -lstdc++ -lz -ldl
else
BFD_LIBS = -L$(EXT_DIR)/binutils/ -lbfd -liberty -ldl -lz
LINK_LIBS = -lpthread $(BFD_LIBS) -lrt -lstdc++ -lresolv -lcap
endif

# 2. Common Libraries
#
# Common but not essential.

ifeq ($(GCC_VERSION), 4.4.0)
BOOST_LIBS = \
	$(EXT_DIR)/boost/lib/libboost_program_options-gcc44-mt.a \
	$(EXT_DIR)/boost/lib/libboost_filesystem-gcc44-mt.a \
	$(EXT_DIR)/boost/lib/libboost_system-gcc44-mt.a \

else
BOOST_LIBS = \
	$(EXT_DIR)/boost/lib/libboost_program_options-gcc40-mt.a \
	$(EXT_DIR)/boost/lib/libboost_filesystem-gcc40-mt.a \
	$(EXT_DIR)/boost/lib/libboost_system-gcc40-mt.a \

endif

ifdef MAC_OS_X

MYSQL_LIBS = -L/usr/local/mysql/lib -lmysqlclient

else

MYSQL_LIBS = $(EXT_DIR)/mysql/lib/mysql/libmysqlclient_r.a \
	-lssl -lcrypto -lcrypt
ORACLE_LIBS = $(EXT_DIR)/oracle/lib/libclntsh.so.11.1

SQLITE_LIBS = $(EXT_DIR)/sqlite/lib/libsqlite3.a

PCRE_LIBS = $(EXT_DIR)/pcre/lib/libpcre.a

HTTP_LIBS = $(EXT_DIR)/libafdt/lib/libafdt.a $(EXT_DIR)/libevent/lib/libevent.a

MCC_LIBS = $(EXT_DIR)/libmcc/lib/libmcc.a $(EXT_DIR)/libch/lib/libch.a \
	$(EXT_DIR)/libevent/lib/libevent.a

GD_LIBS = $(EXT_DIR)/gd/lib/libgd.a -lpng -ljpeg -lfreetype -lfontconfig

MOZILLA_LIBS = $(EXT_DIR)/mozilla/libmozutil_s.a \
               $(EXT_DIR)/mozilla/libexpat_s.a \
               $(EXT_DIR)/mozilla/libsaxp.a \
               $(EXT_DIR)/mozilla/libunicharutil_s.a \
               $(EXT_DIR)/mozilla/libxptcmd.a \
               $(EXT_DIR)/mozilla/libxptcall.a \
               $(EXT_DIR)/mozilla/libxptinfo.a \
               $(EXT_DIR)/mozilla/libxpt.a \
               $(EXT_DIR)/mozilla/libxpcomcomponents_s.a \
               $(EXT_DIR)/mozilla/libxpcomproxy_s.a \
               $(EXT_DIR)/mozilla/libxpcomio_s.a \
               $(EXT_DIR)/mozilla/libxpcomds_s.a \
               $(EXT_DIR)/mozilla/libxpcomglue.a \
               $(EXT_DIR)/mozilla/libxpcombase_s.a \
               $(EXT_DIR)/mozilla/libxpcomthreads_s.a \
               $(EXT_DIR)/mozilla/libstring_s.a \
               $(EXT_DIR)/mozilla/libplc4.a \
               $(EXT_DIR)/mozilla/libplds4.a \
               $(EXT_DIR)/mozilla/libnspr4.a

CURL_LIBS = $(EXT_DIR)/libcurl/lib/libcurl.a

LIBXML_LIBS = -lxml2 -lexpat

TIME_LIBS = $(EXT_DIR)/timelib/lib/libtimelib.a

FBML_LIBS = $(EXT_DIR)/libfbml/lib/libfbml.a $(MOZILLA_LIBS)

MBFL_LIBS = $(EXT_DIR)/libmbfl/lib/libmbfl.a \
	$(EXT_DIR)/oniguruma/lib/libonig.a

LIB_UNWIND = $(EXT_DIR)/libunwind/lib/libunwind.a

ifdef USE_JEMALLOC
JEMALLOC_LIBS = $(EXT_DIR)/jemalloc/lib/libjemalloc.a
endif

ifdef GOOGLE_HEAP_PROFILER
GOOGLE_LIBS = $(EXT_DIR)/google-perftools/lib/libprofiler.a \
	$(EXT_DIR)/google-perftools/lib/libtcmalloc.a $(LIB_UNWIND)
else
GOOGLE_LIBS =
ifdef GOOGLE_CPU_PROFILER
GOOGLE_LIBS = $(EXT_DIR)/google-perftools/lib/libprofiler.a $(LIB_UNWIND)
endif
ifdef GOOGLE_TCMALLOC
GOOGLE_LIBS += $(EXT_DIR)/google-perftools/lib/libtcmalloc_minimal.a \
	$(LIB_UNWIND)
endif
endif

ICU_LIBS = \
	$(EXT_DIR)/icu/lib/libsicui18n.a \
	$(EXT_DIR)/icu/lib/libsicuuc.a \
	$(EXT_DIR)/icu/lib/libsicudata.a \

endif

XHP_LIBS = $(EXT_DIR)/xhp/lib/libxhp.a

TBB_LIBS = -L$(EXT_DIR)/tbb/lib/ $(EXT_DIR)/tbb/lib/libtbb.a

MCRYPT_LIBS = $(EXT_DIR)/libmcrypt/lib/libmcrypt.a

FBI_LIBS = $(EXT_DIR)/libfbi/lib/libfbi.a

LDAP_LIBS = -lldap -llber

ALL_LIBS = $(CURL_LIBS) $(PCRE_LIBS) $(BOOST_LIBS) \
	$(MYSQL_LIBS) $(SQLITE_LIBS) $(MCC_LIBS) \
	$(GD_LIBS) $(LIBXML_LIBS) $(FBML_LIBS) $(MBFL_LIBS) \
	$(MCRYPT_LIBS) $(JEMALLOC_LIBS) $(GOOGLE_LIBS) $(ICU_LIBS) \
	$(HTTP_LIBS) $(XHP_LIBS) $(TIME_LIBS) $(TBB_LIBS) $(FBI_LIBS) \
	$(LDAP_LIBS) $(ORACLE_LIBS)

LIB_PATHS = $(HPHP_LIB) \
  $(EXT_DIR)/libcurl/lib \
  $(EXT_DIR)/mysql/lib/mysql \
  $(EXT_DIR)/boost/lib \
  $(EXT_DIR)/libmcc/lib \
  $(EXT_DIR)/gd/lib \
  $(EXT_DIR)/iconv/lib \
  $(EXT_DIR)/libevent/lib \
  $(EXT_DIR)/libmbfl/lib \
  $(EXT_DIR)/oniguruma/lib \
  $(EXT_DIR)/google-perftools/lib \
  $(EXT_DIR)/libunwind/lib \
  $(EXT_DIR)/icu/lib \
  $(EXT_DIR)/libch/lib \
  $(EXT_DIR)/libafdt/lib \
  $(EXT_DIR)/xhp/lib \
  $(EXT_DIR)/binutils \
  $(EXT_DIR)/libfbml/lib \
  $(EXT_DIR)/mozilla \
  $(EXT_DIR)/timelib/lib \
  $(EXT_DIR)/sqlite/lib \
  $(EXT_DIR)/tbb/lib \
  $(EXT_DIR)/libfbi/lib \
  $(EXT_DIR)/jemalloc/lib

###############################################################################
# Dependencies

# This is to make sure "make" without any target will actually "make all".
overall: all

# Suppressing no rule errors
%.d:
	@true

DEPEND_FILES := $(OBJECTS:.o=.d)

ifneq ($(DEPEND_FILES),)
$(OBJECTS) : %.o : %.d

-include $(DEPEND_FILES)
endif

dep_libs = $(filter $(patsubst -L%,, $(patsubst -l%, $(LIB_DIR)/lib%.a, $(1))), $(wildcard $(LIB_DIR)/*))

DEP_LIBS += $(call dep_libs, $(LIBS))

###############################################################################
# Predefined Targets

ifndef SHOW_COMPILE
ECHO_COMPILE = @echo 'Compiling $< ...'
CV = $(V)
else
ECHO_COMPILE =
CV =
endif

ifndef SHOW_LINK
ECHO_LINK = @echo 'Linking $@ ...'
LV = $(V)
else
ECHO_LINK =
LV =
endif

define COMPILE_IT
$(ECHO_COMPILE)
$(CV)$(1) -c $(if $(OUT_TOP),-I$(OUT_TOP)src) $(CPPFLAGS) $(2) -o $@ -MT $@ -MF $(patsubst %.o, %.d, $@) $<
endef

define LINK_OBJECTS
$(ECHO_LINK)
$(LV)$(LD) $@ $(LDFLAGS) $(filter %.o,$^) $(LIBS)
endef

OBJECT_FILES = $(addprefix $(OUT_DIR),$(patsubst %.$(2),%.o,$(1)))

ifdef NOT_NOW
%:%.o

%:%.c

%:%.cpp
endif

$(call OBJECT_FILES,$(CXX_NOOPT_SOURCES) $(GENERATED_CXX_NOOPT_SOURCES),cpp): $(OUT_DIR)%.o:%.cpp
	$(call COMPILE_IT,$(CXX),$(CXXFLAGS))

$(call OBJECT_FILES,$(CXX_SOURCES) $(GENERATED_CXX_SOURCES),cpp): $(OUT_DIR)%.o:%.cpp
	$(call COMPILE_IT,$(CXX),$(OPT) $(CXXFLAGS))

$(call OBJECT_FILES,$(C_SOURCES) $(GENERATED_C_SOURCES),c): $(OUT_DIR)%.o:%.c
	$(call COMPILE_IT,$(CC),$(OPT))

$(call OBJECT_FILES,$(GENERATED_CPP_SOURCES),c): $(OUT_DIR)%.o:%.c
	$(call COMPILE_IT,$(CXX),$(OPT) $(CXXFLAGS))

$(OUT_DIR)%.o:$(OUT_DIR)%.cpp
	$(call COMPILE_IT,$(CXX),$(OPT) $(CXXFLAGS))

.EXPORT_ALL_VARIABLES:;
unexport CXX_NOOPT_SOURCES CXX_SOURCES C_SOURCES GENERATED_CXX_NOOPT_SOURCES GENERATED_CXX_SOURCES GENERATED_C_SOURCES GENERATED_CPP_SOURCES ALL_SOURCES SOURCES OBJECTS DEPEND_FILES CPPFLAGS CXXFLAGS LDFLAGS PROGRAMS LIB_TARGETS DEP_LIBS

# Since these variables start with += in this file, when calling submake,
# they will not start with empty list. SUB_XXX will always start with empty.
SUB_SOURCE_SUBDIRS = $(SOURCE_SUBDIRS)
SUB_PROGRAMS = $(PROGRAMS)
SUB_LIB_TARGETS = $(LIB_TARGETS)
SUB_OBJECTS = $(OBJECTS)
SUB_INTERMEDIATE_FILES = $(INTERMEDIATE_FILES)

# This trick allows Makefiles in recursive directories each defines its own
# SUB_CLEAN_DIRS without passing into child directories

.DEFAULT:
	$(V)$(MAKE) $(NO_PRINT) -f $(PROJECT_ROOT)/src/default.mk $@

$(OBJECTS): $(GENERATED_SOURCES)

STRIP_ROOT = $(if $(OUT_TOP), $(patsubst -L$(PROJECT_ROOT)/%,-L%,$(patsubst $(PROJECT_ROOT)/%,%,$(patsubst $(ABS_PROJECT_ROOT)/%,%, $(1)))), $(1))

ifdef SHOW_LINK

$(SHARED_LIB): $(OBJECTS)
	$(if $(OUT_TOP),cd $(PROJECT_ROOT) &&) \
		$(CXX) -shared -fPIC $(DEBUG_SYMBOL) -Wall -Werror -Wno-invalid-offsetof -Wl,-soname,lib$(PROJECT_NAME).so \
			-o $(call STRIP_ROOT,$@ $(OBJECTS) $(EXTERNAL))

$(STATIC_LIB): $(OBJECTS)
	@echo $(words $(OBJECTS))
	$(if $(OUT_TOP),cd $(PROJECT_ROOT) &&) \
		$(AR) $(call STRIP_ROOT,$@ $(OBJECTS))

$(MONO_TARGETS): %:%.o $(DEP_LIBS)
	$(LD) $@ $(LDFLAGS) $< $(LIBS)

else

$(SHARED_LIB): $(OBJECTS)
	@echo 'Linking $@ ...'
	$(V)$(if $(OUT_TOP),cd $(PROJECT_ROOT) &&) \
		$(CXX) -shared -fPIC $(DEBUG_SYMBOL) -Wall -Werror -Wno-invalid-offsetof -Wl,-soname,lib$(PROJECT_NAME).so \
			-o $(call STRIP_ROOT,$@ $(OBJECTS) $(EXTERNAL))

$(STATIC_LIB): $(OBJECTS)
	@echo 'Linking $@ ...'
	$(V)$(if $(OUT_TOP),cd $(PROJECT_ROOT) &&) \
		$(AR) $(call STRIP_ROOT,$@ $(OBJECTS))

$(MONO_TARGETS): %:%.o $(DEP_LIBS)
	@echo 'Linking $@ ...'
	$(V)$(LD) $@ $(LDFLAGS) $< $(LIBS)

endif

.PHONY:out-of-date
$(APP_TARGET): $(OBJECTS) $(DEP_LIBS) $(FORCE_RELINK)
	$(LINK_OBJECTS) $(LINK_LIBS)

.PHONY: $(LIB_TARGETS)
$(LIB_TARGETS): $(CODEGEN_TARGETS)
	$(V)$(MAKE) $(NO_PRINT) -C $@

.PHONY: $(PROGRAMS)
$(PROGRAMS): $(LIB_TARGETS)
	$(V)$(MAKE) $(NO_PRINT) -C $@

.PHONY: report
report:
	@echo "Time    PID  Source File"
	@echo "---------------------------------------------"
	$(V)ps wwaxo pid,etime,command k start | grep distcc | grep -v grep | \
	sed 's/^\( *[0-9]\+\) \+\([0-9:]\+\) .* \([^ ]\+\)$$/\2 \1  \3/' | \
	head -`tput lines`

.PHONY: top
top:
	$(V)watch $(MAKE) -s report
