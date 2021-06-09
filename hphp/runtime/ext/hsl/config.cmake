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

HHVM_DEFINE_EXTENSION("hsl_locale"
  SOURCES
    ext_hsl_locale.cpp
    hsl_locale_byte_ops.cpp
    hsl_locale_icu_ops.cpp
    hsl_locale_libc_ops.cpp
  SYSTEMLIB
    ext_hsl_locale.php
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

HHVM_DEFINE_EXTENSION("hsl_str"
  SOURCES
    ext_hsl_str.cpp
  SYSTEMLIB
    ext_hsl_str.php
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

HHVM_DEFINE_EXTENSION("hsl_systemlib"
  SOURCES
    ext_hsl_systemlib.cpp
  HACK_SYSTEMLIB_DIR
    # Relative paths also work if you wanted the lib source to be in
    # hphp/runtime/ext
    "${CMAKE_SOURCE_DIR}/hphp/hsl/src"
) 
