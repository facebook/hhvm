# - Try to find the LDAP client libraries
# Once done this will define
#
#  LDAP_FOUND - system has libldap
#  LDAP_INCLUDE_DIR - the ldap include directory
#  LDAP_LIBRARIES - libldap + liblber (if found) library
#  LBER_LIBRARIES - liblber library

if(LDAP_INCLUDE_DIR AND LDAP_LIBRARIES)
    # Already in cache, be silent
    set(Ldap_FIND_QUIETLY TRUE)
endif(LDAP_INCLUDE_DIR AND LDAP_LIBRARIES)

if(UNIX)
   FIND_PATH(LDAP_INCLUDE_DIR ldap.h)
   FIND_LIBRARY(LDAP_LIBRARIES NAMES ldap)
   FIND_LIBRARY(LBER_LIBRARIES NAMES lber)

else(UNIX)
   FIND_PATH(LDAP_INCLUDE_DIR winldap.h)
   FIND_LIBRARY(LDAP_LIBRARIES NAMES wldap32)
endif(UNIX)

if(LDAP_INCLUDE_DIR AND LDAP_LIBRARIES)
   set(LDAP_FOUND TRUE)
   if(LBER_LIBRARIES)
     set(LDAP_LIBRARIES ${LDAP_LIBRARIES} ${LBER_LIBRARIES})
   endif(LBER_LIBRARIES)
endif(LDAP_INCLUDE_DIR AND LDAP_LIBRARIES)

if(LDAP_FOUND)
   if(NOT Ldap_FIND_QUIETLY)
      message(STATUS "Found ldap: ${LDAP_LIBRARIES}")
   endif(NOT Ldap_FIND_QUIETLY)
else(LDAP_FOUND)
   if (Ldap_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find ldap")
   endif (Ldap_FIND_REQUIRED)
endif(LDAP_FOUND)

MARK_AS_ADVANCED(LDAP_INCLUDE_DIR LDAP_LIBRARIES LBER_LIBRARIES)
