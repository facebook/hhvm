HHVM_DEFINE_EXTENSION("hsl_random"
  SOURCES
    ext_hsl_random.cpp
  SYSTEMLIB
    ext_hsl_random.php
  DEPENDS
    libBoost
    libFolly
)

HHVM_DEFINE_EXTENSION("hsl_time"
  SOURCES
    ext_hsl_time.cpp
  SYSTEMLIB
    ext_hsl_time.php
)
