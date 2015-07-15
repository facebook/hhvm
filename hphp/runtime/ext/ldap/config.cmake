HHVM_DEFINE_EXTENSION("ldap" IMPLICIT
  SOURCES
    ext_ldap.cpp
  HEADERS
    ext_ldap.h
  SYSTEMLIB
    ext_ldap.php
  DEPENDS
    libFolly
    libLdap
)
