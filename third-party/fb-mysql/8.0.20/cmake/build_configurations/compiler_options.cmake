# Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2.0,
# as published by the Free Software Foundation.
#
# This program is also distributed with certain software (including
# but not limited to OpenSSL) that is licensed under separate terms,
# as designated in a particular file or component or in included license
# documentation.  The authors of MySQL hereby grant you an additional
# permission to link the program and your derivative works with the
# separately licensed software that they have included with MySQL.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License, version 2.0, for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(cmake/floating_point.cmake)

IF(SIZEOF_VOIDP EQUAL 4)
  SET(32BIT 1)
ENDIF()
IF(SIZEOF_VOIDP EQUAL 8)
  SET(64BIT 1)
ENDIF()

SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Compiler options
IF(UNIX)  

  IF(MY_COMPILER_IS_GNU_OR_CLANG AND NOT SOLARIS_SPARC)
    SET(SECTIONS_FLAG "-ffunction-sections -fdata-sections")
  ELSE()
    SET(SECTIONS_FLAG)
  ENDIF()

  # Default GCC flags
  IF(MY_COMPILER_IS_GNU)
    SET(COMMON_C_FLAGS               "-fno-omit-frame-pointer")
    # Disable inline optimizations for valgrind testing to avoid false positives
    IF(WITH_VALGRIND)
      STRING_PREPEND(COMMON_C_FLAGS  "-fno-inline ")
    ENDIF()
    # Disable floating point expression contractions to avoid result differences
    IF(HAVE_C_FLOATING_POINT_FUSED_MADD)
      STRING_APPEND(COMMON_C_FLAGS   " -ffp-contract=off")
    ENDIF()

    SET(COMMON_CXX_FLAGS             "-std=c++14 -fno-omit-frame-pointer")
    # Disable inline optimizations for valgrind testing to avoid false positives
    IF(WITH_VALGRIND)
      STRING_PREPEND(COMMON_CXX_FLAGS  "-fno-inline ")
    ENDIF()
    # Disable floating point expression contractions to avoid result differences
    IF(HAVE_CXX_FLOATING_POINT_FUSED_MADD)
      STRING_APPEND(COMMON_CXX_FLAGS " -ffp-contract=off")
    ENDIF()
  ENDIF()

  # Default Clang flags
  IF(MY_COMPILER_IS_CLANG)
    SET(COMMON_C_FLAGS               "-fno-omit-frame-pointer")
    SET(COMMON_CXX_FLAGS             "-std=c++14 -fno-omit-frame-pointer")
  ENDIF()

  # Solaris flags
  IF(SOLARIS)
    # Link mysqld with mtmalloc on Solaris 10 and later
    SET(WITH_MYSQLD_LDFLAGS "-lmtmalloc" CACHE STRING "")

    IF(MY_COMPILER_IS_SUNPRO)
      SET(SUNPRO_FLAGS     "")
      STRING_APPEND(SUNPRO_FLAGS     " -xbuiltin=%all")
      STRING_APPEND(SUNPRO_FLAGS     " -xlibmil")
      STRING_APPEND(SUNPRO_FLAGS     " -xatomic=studio")

      # Show tags for warnings, so that they can be added to suppression list
      SET(SUNPRO_FLAGS     "${SUNPRO_FLAGS} -errtags")

      IF(SOLARIS_INTEL)
        STRING_APPEND(SUNPRO_FLAGS   " -nofstore")
      ENDIF()

      SET(COMMON_C_FLAGS            "${SUNPRO_FLAGS}")

      # Build list of C warning tags to suppress. Comment in/out as needed.

      # warning: useless declaration (E_USELESS_DECLARATION)
      # Count: 8
      # LIST(APPEND SUNPRO_C_WARNING_SUPPRESSION_LIST E_USELESS_DECLARATION)

      # warning: empty translation unit (E_EMPTY_TRANSLATION_UNIT)
      LIST(APPEND SUNPRO_C_WARNING_SUPPRESSION_LIST E_EMPTY_TRANSLATION_UNIT)

      # warning: initialization type mismatch (E_INITIALIZATION_TYPE_MISMATCH)
      # Count: 114
      # LIST(APPEND SUNPRO_C_WARNING_SUPPRESSION_LIST
      # E_INITIALIZATION_TYPE_MISMATCH)

      # warning: statement not reached (E_STATEMENT_NOT_REACHED)
      # Count: 3
      # LIST(APPEND SUNPRO_C_WARNING_SUPPRESSION_LIST E_STATEMENT_NOT_REACHED)

      # warning: initializer will be sign-extended: -1 (E_INIT_SIGN_EXTEND)
      # Count: 1
      # LIST(APPEND SUNPRO_C_WARNING_SUPPRESSION_LIST E_INIT_SIGN_EXTEND)

      # warning: implicit function declaration: ntohl
      # (E_NO_IMPLICIT_DECL_ALLOWED)
      # Count: 2
      # LIST(APPEND SUNPRO_C_WARNING_SUPPRESSION_LIST E_NO_IMPLICIT_DECL_ALLOWED)

      # Convert CMAKE list to comma-separated string, and append to
      # COMMON_C_FLAGS
      STRING(REPLACE ";" "," SUNPRO_C_WARNING_SUPPRESSION_STRING
	${SUNPRO_C_WARNING_SUPPRESSION_LIST})
      SET(SUNPRO_C_WARNING_SUPPRESSION_FLAGS
	"-erroff=${SUNPRO_C_WARNING_SUPPRESSION_STRING}")
      STRING_APPEND(COMMON_C_FLAGS " ${SUNPRO_C_WARNING_SUPPRESSION_FLAGS}")


      SET(COMMON_CXX_FLAGS          "-std=c++14 ${SUNPRO_FLAGS}")

      # Build list of C++ warning tags to suppress. Comment in/out as needed.

      # Warning, anonnotype: Types cannot be declared in anonymous union.
      # Count: 43
      LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST anonnotype)

      # Warning, fieldsemicolonw: extra ";" ignored.
      # Count: 5
      #LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST fieldsemicolonw)

      # Warning, wvarhidemem: key_type hides keyring::Key::key_type.
      # Count: 2917
      LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST wvarhidemem)

      # Warning, anonstruct: Anonymous struct is being declared.
      # Count: 717
      LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST anonstruct)

      # Warning, wlessrestrictedthrow: Function
      # xcl::Connection_impl::~Connection_impl() can throw only the
      # exceptions thrown by the function xcl::XConnection::~XConnection()
      # it overrides.
      # Count: 1221
      LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST wlessrestrictedthrow)

      # Warning, oklambdaretmulti: Deducing non-void lambda return type
      # 'bool' from lambda without a single return statement.
      # Count: 84
      LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST oklambdaretmulti)

      # Warning, nonewline: Last line in file ".../registry_metadata.cc.inc"
      # is not terminated with a newline.
      # Count: 58
      #LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST nonewline)

      # Warning, voidretw: "worker_thread(void*)" is expected to return a
      # value.
      # Count: 1
      #LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST voidretw)

      # Warning, unassigned: The variable ret has not yet been assigned a
      # value.
      # Count: 193
      LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST unassigned)

      # Warning, badinitstr: String literal converted to char* in
      # initialization.
      # Count: 6
      #LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST badinitstr)

      # Warning, hidevf: meb::Process_data_mgr::get_buffer hides the virtual
      # function meb::Data_mgr::get_buffer(unsigned long long).
      # Count: 7
      #LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST hidevf)

      # Warning, attrskipunsup: attribute unused is unsupported and will be
      # skipped..
      # Count: 5
      #LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST attrskipunsup)

      # Warning, nonvarrefww: A reference return value should be an lvalue
      # (if the value of this function is used, the result is unpredictable).
      # Count: 4
      #LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST nonvarrefww)

      # Warning, explctspectypename: "typename" must be used within a template.
      # Count: 1
      #LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST explctspectypename)

      # Warning: Likely out-of-bound read: *(rhs+9[1]) in function decLnOp
      #   (SEC_ARR_OUTSIDE_BOUND_READ)
      # Count: 10
      #LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST SEC_ARR_OUTSIDE_BOUND_READ)

      # Warning (Anachronism), incomtypew: debug_sync_C_callback_ptr, of type
      # void(*)(const char*,unsigned long), was previously declared
      # extern "C" void(*)(const char*,unsigned long).
      # Count: 1
      #LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST, incomtypew)

      # Warning, symdeprecated: 'MD5_Init(MD5state_st*)' is deprecated
      # (declared at /usr/include/openssl/md5.h, line 124).
      # Count: 3
      #LIST(APPEND SUNPRO_CXX_WARNING_SUPPRESSION_LIST symdeprecated)

      # Convert CMAKE list to comma-separated string, and append to
      # COMMON_CXX_FLAGS
      STRING(REPLACE ";" "," SUNPRO_CXX_WARNING_SUPPRESSION_STRING
	"${SUNPRO_CXX_WARNING_SUPPRESSION_LIST}")
      SET(SUNPRO_CXX_WARNING_SUPPRESSION_FLAGS
	"-erroff=${SUNPRO_CXX_WARNING_SUPPRESSION_STRING}")
      STRING_APPEND(COMMON_CXX_FLAGS " ${SUNPRO_CXX_WARNING_SUPPRESSION_FLAGS}")


      # Reduce size of debug binaries, by omitting function declarations.
      # Note that we cannot set "-xdebuginfo=no%decl" during feature tests.
      # We still may get linking errors for merge_large_tests-t with Studio 12.6
      # -g0 is the same as -g, except that inlining is enabled.
      # When building -DWITH_NDBCLUSTER=1 even more of the merge_xxx_tests
      # fail to link, so we keep -g0 for Studio 12.6
      STRING_APPEND(CMAKE_C_FLAGS_DEBUG            " -g0 -xdebuginfo=no%decl")
      STRING_APPEND(CMAKE_CXX_FLAGS_DEBUG          " -g0 -xdebuginfo=no%decl")
      STRING_APPEND(CMAKE_C_FLAGS_RELWITHDEBINFO   " -xdebuginfo=no%decl")
      STRING_APPEND(CMAKE_CXX_FLAGS_RELWITHDEBINFO " -xdebuginfo=no%decl")

      # Bugs in SunPro, compile/link error unless we add some debug info.
      # Errors seem to be related to TLS functions.
      STRING_APPEND(CMAKE_CXX_FLAGS_MINSIZEREL
        " -g0 -xdebuginfo=no%line,no%param,no%decl,no%variable,no%tagtype")
      STRING_APPEND(CMAKE_CXX_FLAGS_RELEASE
        " -g0 -xdebuginfo=no%line,no%param,no%decl,no%variable,no%tagtype")
    ENDIF()
  ENDIF()

  # Use STRING_PREPEND here, so command-line input can override our defaults.
  STRING_PREPEND(CMAKE_C_FLAGS                  "${COMMON_C_FLAGS} ")
  STRING_PREPEND(CMAKE_C_FLAGS_RELWITHDEBINFO   "${SECTIONS_FLAG} ")
  STRING_PREPEND(CMAKE_C_FLAGS_RELEASE          "${SECTIONS_FLAG} ")
  STRING_PREPEND(CMAKE_C_FLAGS_MINSIZEREL       "${SECTIONS_FLAG} ")

  STRING_PREPEND(CMAKE_CXX_FLAGS                "${COMMON_CXX_FLAGS} ")
  STRING_PREPEND(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${SECTIONS_FLAG} ")
  STRING_PREPEND(CMAKE_CXX_FLAGS_RELEASE        "${SECTIONS_FLAG} ")
  STRING_PREPEND(CMAKE_CXX_FLAGS_MINSIZEREL     "${SECTIONS_FLAG} ")

ENDIF()
