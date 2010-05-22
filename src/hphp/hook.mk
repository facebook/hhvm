CXX_SOURCES += $(OUT_DIR)fb_compiler_hooks.cpp
INTERMEDIATE_FILES += $(OUT_DIR)fb_compiler_hooks.cpp \
	$(OUT_DIR)fb_compiler_hooks.h

$(OUT_DIR)fb_compiler_hooks.cpp: \
  ../../facebook/src/compiler/fb_compiler_hooks.cpp \
  $(OUT_DIR)fb_compiler_hooks.h
	$(V)cp $< $@

$(OUT_DIR)fb_compiler_hooks.h: \
  ../../facebook/src/compiler/fb_compiler_hooks.h
	$(V)cp $< $@

$(OUT_TOP)hphp: $(OUT_DIR)fb_compiler_hooks.o

$(OUT_DIR)fb_compiler_hooks.o : $(OUT_DIR)fb_compiler_hooks.d
-include $(OUT_DIR)fb_compiler_hooks.d
