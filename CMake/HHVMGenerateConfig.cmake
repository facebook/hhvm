# The core function for generating `hphp-config.h`. This is done here to keep
# the main CMake tree clean.
#
# If you add any defines here or in hphp-config.h.in, you must make sure
# to update the non-CMake side. If you are not sure if a particular define
# should be present, ask your FB reviewer.

include(CheckCXXSourceCompiles)
include(CheckFunctionExists)
include(CheckIncludeFile)

# Build a string to define the appropriate macros for the headers that
# we are able to find.
function(HHVM_GENERATE_CONFIG_HEADERS_FOUND_DEFINE_STRING destVarName)
  # This is a list of headers to check for and add defines in the form of
  # HAVE_*_H if they are present. Note that all slashes will be replaced
  # with a single underscore. Please keep ordered such that all headers
  # in a root directory are first, followed by headers in a subdirectory.
  # Note that .h will automatically be appended to the names of these before
  # searching for them.
  set(HHVM_HEADERS_TO_CHECK)
  list(APPEND HHVM_HEADERS_TO_CHECK
    "inttypes"
    "limits"
    "utime"
    "wchar"
    "wctype"
    "sys/mman"
    "sys/utime"
  )

  set(builtString "")
  list(LENGTH HHVM_HEADERS_TO_CHECK headersCount)
  set(i 0)
  while (i LESS ${headersCount})
    list(GET HHVM_HEADERS_TO_CHECK ${i} curHeader)
    string(REPLACE "/" "_" curHeaderClean "${curHeader}")
    string(TOUPPER "${curHeaderClean}" curHeaderUpper)
    CHECK_INCLUDE_FILE("${curHeader}.h" HAVE_${curHeaderUpper}_H)
    if (${HAVE_${curHeaderUpper}_H})
      set(builtString "${builtString}\n#define HAVE_${curHeaderUpper}_H 1")
    else()
      set(builtString "${builtString}\n/* #undef HAVE_${curHeaderUpper}_H */")
    endif()
    math(EXPR i "${i} + 1")
  endwhile()

  set(${destVarName} "${builtString}" PARENT_SCOPE)
endfunction()

# Build a string to define the appropriate macros for the functions that
# we are able to find.
function(HHVM_GENERATE_CONFIG_FUNCTIONS_FOUND_DEFINE_STRING destVarName)
  # This is a list of functions that need to be checked for.
  set(HHVM_FUNCTIONS_TO_CHECK)
  list(APPEND HHVM_FUNCTIONS_TO_CHECK
    "getline"
    "mbrtowc"
    "mkstemp"
    "mmap"
    "strerror"
    "strlcpy"
    "strtof"
    "strtoul"
    "utime"
    "utimes"
  )

  # This is a list of functions that are known to be present under MSVC
  # because they are implemented via Folly's portability headers. For an
  # item in this list to have any effect, it must first fail to be found
  # when checking the item in the main list.
  set(HHVM_FUNCTIONS_KNOWN_TO_BE_PRESENT_MSVC)
  list(APPEND HHVM_FUNCTIONS_KNOWN_TO_BE_PRESENT_MSVC
    "mkstemp"
    "mmap"
  )

  set(builtString "")
  list(LENGTH HHVM_FUNCTIONS_TO_CHECK functionCount)
  set(i 0)
  while (i LESS ${functionCount})
    list(GET HHVM_FUNCTIONS_TO_CHECK ${i} curFunc)
    string(TOUPPER "${curFunc}" curFuncUpper)
    CHECK_FUNCTION_EXISTS("${curFunc}" HAVE_${curFuncUpper})
    if (${HAVE_${curFuncUpper}})
      set(builtString "${builtString}\n#define HAVE_${curFuncUpper} 1")
    else()
      list(FIND HHVM_FUNCTIONS_KNOWN_TO_BE_PRESENT_MSVC "${curFunc}" curFuncIdx)
      if (curFuncIdx EQUAL -1 OR NOT MSVC)
        set(builtString "${builtString}\n/* #undef HAVE_${curFuncUpper} */")
      else()
        set(builtString "${builtString}\n#define HAVE_${curFuncUpper} 1 /* Implemented via Folly Portability header */")
      endif()
    endif()
    math(EXPR i "${i} + 1")
  endwhile()

  set(${destVarName} "${builtString}" PARENT_SCOPE)
endfunction()

# Check if the source passed in compiles and add the appropriate define.
function(HHVM_GENERATE_CONFIG_CHECK_COMPILES destVarName defineName sourceToCheck)
  CHECK_CXX_SOURCE_COMPILES("${sourceToCheck}" ${defineName})
  if (${defineName})
    set(${destVarName} "${${destVarName}}\n#define ${defineName} 1" PARENT_SCOPE)
  else()
    set(${destVarName} "${${destVarName}}\n/* #undef ${defineName} */" PARENT_SCOPE)
  endif()
endfunction()

# A quick check for use by HHVM_GENERATE_CONFIG_COMPILES_DEFINE_STRING to
# check if a struct defined in a particular header has a member with
# the specified name.
function(HHVM_GENERATE_CONFIG_STRUCT_HAS_MEMBER destVarName defineName headers structName memberName)
  HHVM_GENERATE_CONFIG_CHECK_COMPILES(${destVarName} ${defineName} "
  ${headers}
  int main(void) {
    (void)((${structName}*)0)->${memberName};
    return 0;
  }")

  set(${destVarName} ${${destVarName}} PARENT_SCOPE)
endfunction()

# A quick check for use by HHVM_GENERATE_CONFIG_COMPILES_DEFINE_STRING to
# check if a symbol is defined in a particular header.
function(HHVM_GENERATE_CONFIG_SYMBOL_EXISTS destVarName defineName headers symbolName)
  HHVM_GENERATE_CONFIG_CHECK_COMPILES(${destVarName} ${defineName} "
  ${headers}
  int main(void) {
  #ifdef ${symbolName}
    return 0;
  #else
    return ((int*)(&${symbolName}))[0];
  #endif
  }")

  set(${destVarName} ${${destVarName}} PARENT_SCOPE)
endfunction()

# Build a define string for checks that require checking if some source code
# compiles.
function(HHVM_GENERATE_CONFIG_COMPILES_DEFINE_STRING destVarName)
  set(builtString "")

  HHVM_GENERATE_CONFIG_STRUCT_HAS_MEMBER(builtString HAVE_TM_ISDST "#include <time.h>" "tm" "tm_isdst")
  HHVM_GENERATE_CONFIG_STRUCT_HAS_MEMBER(builtString HAVE_STRUCT_TM_TM_GMTOFF "#include <time.h>" "tm" "tm_gmtoff")
  HHVM_GENERATE_CONFIG_STRUCT_HAS_MEMBER(builtString HAVE_STRUCT_TM_TM_ZONE "#include <time.h>" "tm" "tm_zone")
  HHVM_GENERATE_CONFIG_SYMBOL_EXISTS(builtString HAVE_DAYLIGHT "#include <time.h>" "daylight")
  HHVM_GENERATE_CONFIG_SYMBOL_EXISTS(builtString MAJOR_IN_MKDEV "#include <mkdev.h>" "major")
  HHVM_GENERATE_CONFIG_SYMBOL_EXISTS(builtString MAJOR_IN_SYSMACROS "#include <sysmacros.h>" "major")
  HHVM_GENERATE_CONFIG_CHECK_COMPILES(builtString HAVE_VISIBILITY "
  __attribute__ ((__visibility__(\"hidden\"))) void someMethod(void);
  int main(void) {
    return 0;
  }")

  set(${destVarName} "${builtString}" PARENT_SCOPE)
endfunction()

# Builds a string to define the macros for all enabled extensions.
function(HHVM_GENERATE_CONFIG_EXTENSIONS_ENABLED_DEFINE_STRING destVarName)
  set(builtString "/* Extensions */")
  set(i 0)
  while (i LESS HHVM_EXTENSION_COUNT)
    string(TOUPPER ${HHVM_EXTENSION_${i}_NAME} upperExtName)
    if (${HHVM_EXTENSION_${i}_ENABLED_STATE} EQUAL 1)
      set(builtString "${builtString}\n#define ENABLE_EXTENSION_${upperExtName} 1")
    else()
      set(builtString "${builtString}\n/* #undef ENABLE_EXTENSION_${upperExtName} */")
    endif()
    math(EXPR i "${i} + 1")
  endwhile()

  set(${destVarName} "${builtString}" PARENT_SCOPE)
endfunction()

# Generate the config file for HHVM.
function(HHVM_GENERATE_CONFIG dest)
  HHVM_GENERATE_CONFIG_HEADERS_FOUND_DEFINE_STRING(HHVM_HEADERS_FOUND_DEFINE_STRING)
  HHVM_GENERATE_CONFIG_FUNCTIONS_FOUND_DEFINE_STRING(HHVM_FUNCTIONS_FOUND_DEFINE_STRING)
  HHVM_GENERATE_CONFIG_COMPILES_DEFINE_STRING(HHVM_COMPILES_DEFINE_STRING)
  HHVM_GENERATE_CONFIG_EXTENSIONS_ENABLED_DEFINE_STRING(HHVM_EXTENSIONS_ENABLED_DEFINE_STRING)
  configure_file("${HPHP_HOME}/hphp/util/hphp-config.h.in" "${dest}")
endfunction()
