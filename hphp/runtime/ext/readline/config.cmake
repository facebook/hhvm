HHVM_DEFINE_EXTENSION("readline"
  SOURCES
    ext_readline.cpp
  SYSTEMLIB
    ext_readline.php
  DEPENDS
    libReadline OPTIONAL
    libEditline OPTIONAL
)
