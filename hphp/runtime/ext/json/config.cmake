HHVM_DEFINE_EXTENSION("json" REQUIRED
  SOURCES
    ext_json.cpp
    JSON_parser.cpp
  HEADERS
    ext_json.h
    JSON_parser.h
  SYSTEMLIB
    ext_json.php
)
