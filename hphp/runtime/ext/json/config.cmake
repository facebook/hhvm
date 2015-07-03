# The strange set-up here is because Debian doesn't like the license on the the
# bundled parser. The built in one is still preferred though, so only use json-c
# if the USE_JSONC option is set

option(USE_JSONC "Use json-c parser instead of the bundled parser" OFF)

if (USE_JSONC)
  message(STATUS "Using libjson-c as JSON parser")
  HHVM_DEFINE_EXTENSION("json" REQUIRED
    SOURCES
      ext_json.cpp
      jsonc_parser.cpp
    HEADERS
      ext_json.h
    EXTENSION_LIBRARY
      ext_json.php
    DEPENDS_UPON
      ext_collections
      ext_string
      libJsonc
  )
else()
  message(STATUS "Using built-in JSON parser")
  HHVM_DEFINE_EXTENSION("json" REQUIRED
    SOURCES
      ext_json.cpp
      JSON_parser.cpp
    HEADERS
      ext_json.h
      JSON_parser.h
    EXTENSION_LIBRARY
      ext_json.php
    DEPENDS_UPON
      ext_collections
      ext_string
      systemlib
  )
endif()
