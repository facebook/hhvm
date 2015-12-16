set(C_SOURCES)
auto_sources(files "*.c" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND C_SOURCES ${files})

set(CXX_SOURCES)
auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

set(HEADER_SOURCES)
auto_sources(files "*.h" "RECURSE")
list(APPEND HEADER_SOURCES ${files})

if (NOT MSVC AND NOT APPLE)
  list(REMOVE_ITEM CXX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/crypt-freesec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/crypt-sha256.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/crypt-sha512.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/php-crypt_r.cpp
  )
  list(REMOVE_ITEM HEADER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/crypt-freesec.h
    ${CMAKE_CURRENT_SOURCE_DIR}/php-crypt_r.h
  )
endif()

HHVM_PUBLIC_HEADERS(zend ${HEADER_SOURCES})

add_library(hphp_zend STATIC ${C_SOURCES} ${CXX_SOURCES} ${HEADER_SOURCES})
auto_source_group("hphp_zend" "${CMAKE_CURRENT_SOURCE_DIR}"
  ${C_SOURCES} ${CXX_SOURCES} ${HEADER_SOURCES})
