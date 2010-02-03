
CXX_SOURCES += fb_lib_hooks.cpp
INTERMEDIATE_FILES += fb_lib_hooks.cpp fb_lib_hooks.h

fb_lib_hooks.cpp: \
  ../../facebook/src/lib/fb_lib_hooks.cpp \
  fb_lib_hooks.h
	@cp $< $@

fb_lib_hooks.h: \
  ../../facebook/src/lib/fb_lib_hooks.h
	@cp $< $@

hphp: fb_lib_hooks.o
