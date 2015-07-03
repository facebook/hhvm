HHVM_DEFINE_EXTENSION("hotprofiler" REQUIRED
  PRETTY_NAME "Hot Profiler"
  SOURCES
    ext_hotprofiler.cpp
  HEADERS
    ext_hotprofiler.h
  DEPENDS_UPON
    ext_std
    ext_xdebug
    libZLib
)
