HHVM_DEFINE_EXTENSION("readline"
  SOURCES
    ext_readline.cpp
  EXTENSION_LIBRARY
    ext_readline.php
  DEPENDS_UPON
    ext_std
    libReadline OPTIONAL
    libEditline OPTIONAL
)
