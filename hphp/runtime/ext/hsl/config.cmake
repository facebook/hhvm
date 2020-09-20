HHVM_DEFINE_EXTENSION("hsl_io"
  SOURCES
    ext_hsl_io.cpp
  SYSTEMLIB
    ext_hsl_io.php
)

HHVM_DEFINE_EXTENSION("hsl_os"
  SOURCES
    ext_hsl_os.cpp
  SYSTEMLIB
    ext_hsl_os.php
)

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

HHVM_DEFINE_EXTENSION("hsl_regex"
  SOURCES
    ext_hsl_regex.cpp
  SYSTEMLIB
    ext_hsl_regex.php
)
