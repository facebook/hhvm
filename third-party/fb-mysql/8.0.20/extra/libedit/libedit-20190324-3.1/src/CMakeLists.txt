# See src/Makefile.am for original build dependencies.

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_SOURCE_DIR} )
INCLUDE(CheckIncludeFile)
include(CheckFunctionExists)

CHECK_INCLUDE_FILES (sys/cdefs.h HAVE_SYS_CDEFS_H)
CHECK_INCLUDE_FILES(term.h HAVE_TERM_H)

IF(SOLARIS)
  #On Solaris, default awk is next to unusable while the xpg4 one is ok.
  IF(EXISTS /usr/xpg4/bin/awk)
    SET(AWK_EXECUTABLE /usr/xpg4/bin/awk)
  ENDIF()
ENDIF()

IF(NOT AWK_EXECUTABLE)
  FIND_PROGRAM(AWK_EXECUTABLE NAMES gawk awk DOC "path to the awk executable")
ENDIF()
MESSAGE(STATUS "AWK_EXECUTABLE is ${AWK_EXECUTABLE}")

MARK_AS_ADVANCED(AWK_EXECUTABLE)
SET(AWK ${AWK_EXECUTABLE})
CONFIGURE_FILE(makelist.in ${CMAKE_CURRENT_BINARY_DIR}/makelist @ONLY)

SET(MAKELIST ${CMAKE_CURRENT_BINARY_DIR}/makelist)

CHECK_C_SOURCE_COMPILES("
  #include <stdlib.h>
  #include <sys/types.h>
  #include <pwd.h>
  int main() {
    int ret = getpwnam_r(NULL, NULL, NULL, (size_t)0, NULL);
    return ret;
  }"
  HAVE_GETPW_R_POSIX
  )

# POSIX.1c Draft 6
CHECK_C_SOURCE_COMPILES("
  #include <stdlib.h>
  #include <sys/types.h>
  #include <pwd.h>
  int main() {
    struct passwd *pwd = getpwnam_r(NULL, NULL, NULL, 0);
    return pwd != NULL;
  }"
  HAVE_GETPW_R_DRAFT
)

CHECK_FUNCTION_EXISTS(getline         HAVE_GETLINE)
#    _FUNCTION_EXISTS(strlcpy         HAVE_STRLCPY) in configure.cmake
#    _FUNCTION_EXISTS(strlcat         HAVE_STRLCAT) in configure.cmake
CHECK_FUNCTION_EXISTS(vis             HAVE_VIS)
CHECK_FUNCTION_EXISTS(unvis           HAVE_UNVIS)
CHECK_FUNCTION_EXISTS(__secure_getenv HAVE___SECURE_GETENV)
CHECK_FUNCTION_EXISTS(secure_getenv   HAVE_SECURE_GETENV)

FUNCTION(MAKELIST_TARGET target outfile options infiles)
  STRING(REPLACE ";" " " infile_string "${infiles}")
  ADD_CUSTOM_COMMAND(
    OUTPUT ${outfile}
    COMMAND sh ${MAKELIST} ${options} ${infile_string} > ${outfile}
    COMMENT "makelist ${options} ${infile_string} > ${outfile}"
    DEPENDS ${infiles}
    )
  ADD_CUSTOM_TARGET(${target} DEPENDS ${outfile})
ENDFUNCTION()

SET(BUILT_SOURCES vi.h emacs.h common.h fcns.h help.h func.h)
SET(AHDR vi.h emacs.h common.h)
SET(ASRC
  ${CMAKE_CURRENT_SOURCE_DIR}/vi.c
  ${CMAKE_CURRENT_SOURCE_DIR}/emacs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/common.c
  )

MAKELIST_TARGET(libedit_vi     vi.h     -h ${CMAKE_CURRENT_SOURCE_DIR}/vi.c)
MAKELIST_TARGET(libedit_emacs  emacs.h  -h ${CMAKE_CURRENT_SOURCE_DIR}/emacs.c)
MAKELIST_TARGET(libedit_common common.h -h ${CMAKE_CURRENT_SOURCE_DIR}/common.c)
MAKELIST_TARGET(libedit_fcns   fcns.h   -fh "${AHDR}")
MAKELIST_TARGET(libedit_help   help.h   -bh "${ASRC}")
MAKELIST_TARGET(libedit_func   func.h   -fc "${AHDR}")

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CURSES_INCLUDE_PATH}
)

SET(LIBEDIT_SOURCES
  chared.c
  chartype.c
  common.c
  el.c
  eln.c
  emacs.c
  filecomplete.c
  hist.c
  history.c
  historyn.c
  keymacro.c
  literal.c
  map.c
  parse.c
  prompt.c
  read.c
  readline.c
  refresh.c
  search.c
  sig.c
  terminal.c
  tokenizer.c
  tokenizern.c
  tty.c
  vi.c
  ${BUILT_SOURCES}
  ${LIBEDIT_EXTRA_SOURCES}
  )

IF(NOT HAVE_STRLCPY)
  LIST(APPEND LIBEDIT_SOURCES strlcpy.c)
ENDIF()
IF(NOT HAVE_STRLCAT)
  LIST(APPEND LIBEDIT_SOURCES strlcat.c)
ENDIF()
IF(NOT HAVE_VIS)
  LIST(APPEND LIBEDIT_SOURCES vis.c)
ENDIF()
IF(NOT HAVE_UNVIS)
  LIST(APPEND LIBEDIT_SOURCES unvis.c)
ENDIF()
IF(FREEBSD AND HAVE_GETLINE)
  ADD_DEFINITIONS(-D_WITH_GETLINE=1)
ENDIF()

ADD_LIBRARY(edit STATIC ${LIBEDIT_SOURCES})
TARGET_LINK_LIBRARIES(edit ${CURSES_LIBRARY})

# clang may complain:
# converts between pointers to integer types with different sign
MY_CHECK_C_COMPILER_FLAG("-Wpointer-sign" HAVE_POINTER_SIGN)
IF(HAVE_POINTER_SIGN)
  TARGET_COMPILE_OPTIONS(edit PRIVATE "-Wno-pointer-sign")
ENDIF()

MY_CHECK_C_COMPILER_FLAG("-Wstringop-overflow" HAVE_STRINGOP_OVERFLOW)
IF(HAVE_STRINGOP_OVERFLOW)
  TARGET_COMPILE_OPTIONS(edit PRIVATE "-Wno-stringop-overflow")
ENDIF()

MY_CHECK_C_COMPILER_FLAG("-Wunused-result" HAVE_UNUSED_RESULT)
IF(HAVE_UNUSED_RESULT)
  TARGET_COMPILE_OPTIONS(edit PRIVATE "-Wno-unused-result")
ENDIF()
