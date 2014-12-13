# Functions for use when building extensions dynamically

# These functions also exist in CMake/EXTFunctions.cmake
# Their signatures should be kept consistent, though their behavior
# will differ slightly.

function(HHVM_LINK_LIBRARIES EXTNAME)
  target_link_libraries(${EXTNAME} ${ARGN})
endfunction()

function(HHVM_ADD_INCLUDES EXTNAME)
  include_directories(${ARGN})
endfunction()

# Add an extension
function(HHVM_EXTENSION EXTNAME)
  set(EXTENSION_NAME ${EXTNAME} CACHE INTERNAL "")
  add_library(${EXTNAME} SHARED ${ARGN})
  set_target_properties(${EXTNAME} PROPERTIES PREFIX "")
  set_target_properties(${EXTNAME} PROPERTIES SUFFIX ".so")
  install(TARGETS ${EXTNAME} DESTINATION "${CMAKE_INSTALL_LIBDIR}/hhvm/extensions/${HHVM_API_VERSION}")
endfunction()

# Add an extension that uses the Zend compatibility layer.
function(HHVM_COMPAT_EXTENSION EXTNAME)
  if(NOT ENABLE_ZEND_COMPAT)
    message(FATAL_ERROR "HHVM was not configured with ENABLE_ZEND_COMPAT, "
      "and so cannot be used to compile Zend-compatible extensions")
  endif()
  HHVM_EXTENSION(${EXTNAME} ${ARGN})
  # Compile all source files as C++
  set_source_files_properties(${ARGN} PROPERTIES LANGUAGE "CXX")
  # Define COMPILE_DL_<EXTNAME> so that ZEND_GET_MODULE() will be invoked
  string(TOUPPER ${EXTNAME} EXTNAME_UPPER)
  add_definitions("-DCOMPILE_DL_${EXTNAME_UPPER}")
  # Add extra include directories
  set(EZC_DIR "${CMAKE_INSTALL_PREFIX}/include/hphp/runtime/ext_zend_compat")
  include_directories("${EZC_DIR}/php-src")
  include_directories("${EZC_DIR}/php-src/main")
  include_directories("${EZC_DIR}/php-src/Zend")
  include_directories("${EZC_DIR}/php-src/TSRM")
endfunction()

function(HHVM_SYSTEMLIB EXTNAME)
  foreach (SLIB ${ARGN})
    embed_systemlib_byname(${EXTNAME} ${SLIB})
  endforeach()
  embed_systemlibs(${EXTNAME} "${EXTNAME}.so")
endfunction()

function(HHVM_DEFINE EXTNAME)
  add_definitions(${ARGN})
endfunction()
