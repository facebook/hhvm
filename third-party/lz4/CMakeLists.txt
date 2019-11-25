add_definitions(-fPIC)
add_library(lz4 STATIC src/lib/lz4.c src/lib/lz4.h src/lib/lz4hc.c src/lib/lz4hc.h)
