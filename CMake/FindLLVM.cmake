#  Find LLVM libraries and headers
#
#  LIBLLVM_INCLUDE_DIR - LLVM header files location
#  LIBLLVM_LIBRARY     - LLVM library to use
#  LIBLLVM_VERSION     - LLVM version
#  LIBLLVM_FOUND       - true if LLVM library was found


# llvm-config can have different names depending on the system
FIND_PROGRAM(LLVMCONFIG
  NAMES "llvm-config-3.5" "llvm-config-3.4" "llvm-config" "llvm-config-64"
  DOC "LLVM config utility"
)

IF (LLVMCONFIG)
  IF (LIBLLVM_INCLUDE_DIR)
    SET(LIBLLVM_FIND_QUIETLY TRUE)
  ENDIF ()

  MACRO (GET_LLVM_CONFIG cl_arg var)
    EXECUTE_PROCESS (
      COMMAND ${LLVMCONFIG} "--${cl_arg}"
      OUTPUT_VARIABLE ${var}
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  ENDMACRO(GET_LLVM_CONFIG)

  GET_LLVM_CONFIG(version LIBLLVM_VERSION)

  IF (${LIBLLVM_VERSION} VERSION_LESS "3.4")
    MESSAGE(STATUS "LLVM version 3.4 or later is required. Will not use LLVM.")
  ELSE (${LIBLLVM_VERSION} VERSION_LESS "3.4")

    GET_LLVM_CONFIG(includedir LIBLLVM_INCLUDE_DIR)
    GET_LLVM_CONFIG(libdir LIBLLVM_LIBDIR)

    FIND_LIBRARY(LIBLLVM_LIBRARY NAMES LLVM-${LIBLLVM_VERSION} LLVM PATHS ${LIBLLVM_LIBDIR} NO_DEFAULT_PATH)
    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBLLVM DEFAULT_MSG LIBLLVM_LIBRARY LIBLLVM_INCLUDE_DIR)

    MARK_AS_ADVANCED(LIBLLVM_LIBRARY LIBLLVM_INCLUDE_DIR)

  ENDIF ()

ELSE()
  MESSAGE(STATUS "Could not find llvm-config. Will not use LLVM for the build.")
ENDIF()
