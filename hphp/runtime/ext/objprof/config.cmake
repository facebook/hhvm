HHVM_DEFINE_EXTENSION("objprof"
  SOURCES
    ext_heapgraph.cpp
    ext_objprof.cpp
  SYSTEMLIB
    ext_heapgraph.php
    ext_objprof.php
)
