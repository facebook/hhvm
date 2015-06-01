# Functions/Macros for use when building extensions statically

# These functions also exist in CMake/HPHPIZEFunctions.cmake
# Their signatures should be kept consistent, though their behavior
# will differ slightly.

macro(HHVM_LINK_LIBRARIES EXTNAME)
  foreach (lib ${ARGN})
    list(APPEND HRE_LIBRARIES ${lib})
  endforeach()
endmacro()

function(HHVM_ADD_INCLUDES EXTNAME)
  include_directories(${ARGN})
endfunction()

macro(HHVM_EXTENSION EXTNAME)
  foreach (src ${ARGN})
    list(APPEND CXX_SOURCES "${HRE_CURRENT_EXT_PATH}/${src}")
  endforeach()
endmacro()

function(HHVM_SYSTEMLIB EXTNAME SOURCE_FILE)
  # Ignore it, embed_all_systemlibs will pick this up
  # TODO: Make this cleaner so that we don't embed systemlibs
  # which aren't going to be used
endfunction()

function(HHVM_DEFINE EXTNAME)
  add_definitions(${ARGN})
endfunction()

function (HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED ExtVar ExtName IsEnabled)
  if (${IsEnabled})
    set(${ExtVar} ON CACHE BOOL "Enable the ${ExtName} extension.")
  else()
    set(${ExtVar} OFF CACHE BOOL "Enable the ${ExtName} extension." FORCE)
  endif()
endfunction()

# Check for a library's existence, and add it's paths if requested.
# This is the core of the automated enabling of extensions.
function (HHVM_EXTENSION_INTERNAL_CHECK_DEPENDENCY InputVar ExtName LibraryName TargetName)
  if (DEFINED "${InputVar}" AND NOT "${InputVar}")
    message("Returning early")
    return()
  endif()
  
  if ("${TargetName}" STREQUAL "OFF")
    set(TARGET_ENABLED OFF)
  else()
    set(TARGET_ENABLED ON)
  endif()
  
  # MySQL has to be handled specially
  if ("${LibraryName}" STREQUAL "MySQL")
    # mysql checks - if we're using async mysql, we use webscalesqlclient from
    # third-party/ instead
    if (ENABLE_ASYNC_MYSQL)
      set(MYSQL_CLIENT_LIB_DIR ${TP_DIR}/webscalesqlclient/src/)
      # Unlike the .so, the static library intentionally does not link against
      # yassl, despite building it :/
      set(MYSQL_CLIENT_LIBS
        ${MYSQL_CLIENT_LIB_DIR}/libmysql/libwebscalesqlclient_r.a
        ${MYSQL_CLIENT_LIB_DIR}/extra/yassl/libyassl.a
        ${MYSQL_CLIENT_LIB_DIR}/extra/yassl/taocrypt/libtaocrypt.a
      )
      
      if (TARGET_ENABLED)
        target_include_directories(${TargetName} PRIVATE
          ${TP_DIR}/re2/src/
          ${TP_DIR}/squangle/src/
          ${TP_DIR}/webscalesqlclient/src/include/
        )
      endif()
    else()
      if (TARGET_ENABLED)
        find_package(MySQL REQUIRED)
      else()
        find_package(MySQL)
      endif()
      
      if (NOT MYSQL_LIB_DIR OR NOT MYSQL_INCLUDE_DIR OR NOT MYSQL_CLIENT_LIBS)
        HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED(${InputVar} ${ExtName} OFF)
        return()
      endif()
      
      if (TARGET_ENABLED)
        target_link_directories(${TargetName} ${MYSQL_LIB_DIR})
        target_include_directories(${TargetName} PRIVATE ${MYSQL_INCLUDE_DIR})
      endif()
    endif()
    
    MYSQL_SOCKET_SEARCH()
    if (MYSQL_UNIX_SOCK_ADDR)
      target_compile_definitions(${TargetName} PUBLIC "PHP_MYSQL_UNIX_SOCK_ADDR=\"${MYSQL_UNIX_SOCK_ADDR}\"")
    elseif (NOT TARGET_ENABLED)
      HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED(${InputVar} ${ExtName} OFF)
      return()
    else()
      message(FATAL_ERROR "Could not find MySQL socket path - if you install a MySQL server, this should be automatically detected. Alternatively, specify -DMYSQL_UNIX_SOCK_ADDR=/path/to/mysql.socket ; if you don't care about unix socket support for MySQL, specify -DMYSQL_UNIX_SOCK_ADDR=/dev/null")
    endif()
    
    if (TARGET_ENABLED)
      target_link_libraries(${TargetName} ${MYSQL_CLIENT_LIBS})
    endif()
    return()
  endif()
  
  if (TARGET_ENABLED)
    find_package(${LibraryName} REQUIRED)
  else()
    find_package(${LibraryName})
  endif()
  
  # This should always be in alphabetical order.
  if ("${LibraryName}" STREQUAL "CURL")
    if (NOT CURL_INCLUDE_DIR OR NOT CURL_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED(${InputVar} ${ExtName} OFF)
      return()
    endif()

    if (TARGET_ENABLED)
      set(CMAKE_REQUIRED_LIBRARIES "${CURL_LIBRARIES}")
      set(CMAKE_REQUIRED_INCLUDES "${CURL_INCLUDE_DIR}")
      CHECK_FUNCTION_EXISTS("curl_multi_select" HAVE_CURL_MULTI_SELECT)
      CHECK_FUNCTION_EXISTS("curl_multi_wait" HAVE_CURL_MULTI_WAIT)
      if (HAVE_CURL_MULTI_SELECT)
        target_compile_definitions(${TargetName} PRIVATE "HAVE_CURL_MULTI_SELECT")
      endif()
      if (HAVE_CURL_MULTI_WAIT)
        target_compile_definitions(${TargetName} PRIVATE "HAVE_CURL_MULTI_WAIT")
      endif()
      set(CMAKE_REQUIRED_LIBRARIES)
      set(CMAKE_REQUIRED_INCLUDES)
      
      target_include_directories(${TargetName} PRIVATE ${CURL_INCLUDE_DIR})
      target_link_libraries(${TargetName} ${CURL_LIBRARIES})
    endif()
  elseif ("${LibraryName}" STREQUAL "EXPAT")
    if (NOT EXPAT_INCLUDE_DIRS OR NOT EXPAT_LIBRARY)
      HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED(${InputVar} ${ExtName} OFF)
      return()
    endif()
    
    if (TARGET_ENABLED)
      target_include_directories(${TargetName} PRIVATE ${EXPAT_INCLUDE_DIRS})
      target_link_libraries(${TargetName} ${EXPAT_LIBRARY})
    endif()
  elseif ("${LibraryName}" STREQUAL "ICU")
    if (NOT ICU_FOUND OR NOT ICU_DATA_LIBRARIES OR NOT ICU_I18N_LIBRARIES OR NOT ICU_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED(${InputVar} ${ExtName} OFF)
      return()
    endif()
    
    if (ICU_VERSION VERSION_LESS "4.2")
      unset(ICU_FOUND CACHE)
      unset(ICU_INCLUDE_DIRS CACHE)
      unset(ICU_LIBRARIES CACHE)
      if (TARGET_ENABLED)
        message(FATAL_ERROR "ICU is too old, found ${ICU_VERSION} and we need 4.2")
      else()
        HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED(${InputVar} ${ExtName} OFF)
        return()
      endif()
    endif ()
    
    if (TARGET_ENABLED)
      target_include_directories(${TargetName} PRIVATE ${ICU_INCLUDE_DIRS})
      target_link_libraries(${TargetName} ${ICU_DATA_LIBRARIES} ${ICU_I18N_LIBRARIES} ${ICU_LIBRARIES})
    endif()
  elseif ("${LibraryName}" STREQUAL "Ldap")
    if (NOT LDAP_INCLUDE_DIR OR NOT LDAP_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED(${InputVar} ${ExtName} OFF)
      return()
    endif()
    
    if (TARGET_ENABLED)
      target_include_directories(${TargetName} PRIVATE ${LDAP_INCLUDE_DIR})
      target_link_libraries(${TargetName} ${LDAP_LIBRARIES})
    endif()
  elseif ("${LibraryName}" STREQUAL "Libmemcached")
    if (NOT LIBMEMCACHED_INCLUDE_DIR OR NOT LIBMEMCACHED_LIBRARY)
      HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED(${InputVar} ${ExtName} OFF)
      return()
    endif()
    
    if (LIBMEMCACHED_VERSION VERSION_LESS "0.39")
      unset(LIBMEMCACHED_INCLUDE_DIR CACHE)
      unset(LIBMEMCACHED_LIBRARY CACHE)
      unset(LIBMEMCACHED_VERSION CACHE)
      if (TARGET_ENABLED)
        message(FATAL_ERROR "libmemcached is too old, found ${LIBMEMCACHED_VERSION} and we need 0.39")
      else()
        HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED(${InputVar} ${ExtName} OFF)
        return()
      endif()
    endif()
    
    if (TARGET_ENABLED)
      target_include_directories(${TargetName} PRIVATE ${LIBMEMCACHED_INCLUDE_DIR})
      target_link_libraries(${TargetName} ${LIBMEMCACHED_LIBRARY})
    endif()
  elseif ("${LibraryName}" STREQUAL "LibXml2")
    if (NOT LIBXML2_INCLUDE_DIR OR NOT LIBXML2_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED(${InputVar} ${ExtName} OFF)
      return()
    endif()
    
    if (TARGET_ENABLED)
      target_include_directories(${TargetName} PRIVATE ${LIBXML2_INCLUDE_DIR})
      target_compile_definitions(${TargetName} PRIVATE ${LIBXML2_DEFINITIONS})
      target_link_libraries(${TargetName} ${LIBXML2_LIBRARIES})
    endif()
  elseif ("${LibraryName}" STREQUAL "LibXslt")
    if (NOT LIBXSLT_INCLUDE_DIR OR NOT LIBXSLT_DEFINITIONS OR NOT LIBXSLT_LIBRARIES OR NOT LIBXSLT_EXSLT_LIBRARIES)
      HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED(${InputVar} ${ExtName} OFF)
      return()
    endif()
    
    if (TARGET_ENABLED)
      target_include_directories(${TargetName} PRIVATE ${LIBXSLT_INCLUDE_DIR})
      target_compile_definitions(${TargetName} PRIVATE ${LIBXSLT_DEFINITIONS})
      target_link_libraries(${TargetName} ${LIBXSLT_LIBRARIES} ${LIBXSLT_EXSLT_LIBRARIES})
    endif()
  elseif ("${LibraryName}" STREQUAL "Mcrypt")
    if (NOT Mcrypt_INCLUDE_DIR OR NOT Mcrypt_LIB)
      HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED(${InputVar} ${ExtName} OFF)
      return()
    endif()
    
    if (TARGET_ENABLED)
      target_include_directories(${TargetName} PRIVATE ${Mcrypt_INCLUDE_DIR})
      target_link_libraries(${TargetName} ${Mcrypt_LIB})
    endif()
  else()
    message(FATAL_ERROR "Unknown library dependency '${LibraryName}'!")
  endif()
endfunction()

macro (HHVM_EXTENSION_CHECK_DEPENDENCIES ExtName)
  string(TOUPPER "${ExtName}" UPPER_EXT_NAME)
  set(EXTENSION_VAR "ENABLE_EXTENSION_${UPPER_EXT_NAME}")
  if (NOT DEFINED ${EXTENSION_VAR})
    HHVM_EXTENSION_INTERNAL_SET_EXTENSION_ENABLED(${EXTENSION_VAR} ${ExtName} ON)
    foreach (lib ${ARGN})
      HHVM_EXTENSION_INTERNAL_CHECK_DEPENDENCY(${EXTENSION_VAR} ${ExtName} ${lib} OFF)
    endforeach()
  endif()
  
  if (${EXTENSION_VAR})
    foreach (lib ${ARGN})
      HHVM_EXTENSION_INTERNAL_CHECK_DEPENDENCY(${EXTENSION_VAR} ${ExtName} ${lib} hphp_runtime_ext)
    endforeach()
  endif()
endmacro()