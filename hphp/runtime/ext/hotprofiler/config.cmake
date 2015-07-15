HHVM_DEFINE_EXTENSION("hotprofiler" REQUIRED
  PRETTY_NAME "Hot Profiler"
  SOURCES
    ext_hotprofiler.cpp
  HEADERS
    ext_hotprofiler.h
  DEPENDS
    libZLib
)
