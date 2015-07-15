HHVM_DEFINE_EXTENSION("memcache"
  SOURCES
    ext_memcache.cpp
  SYSTEMLIB
    ext_memcache.php
  DEPENDS
    libMemcached
)
