# Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.
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

MACRO(MY_ADD_C_WARNING_FLAG WARNING_FLAG)
  MY_CHECK_C_COMPILER_FLAG("-${WARNING_FLAG}" HAVE_${WARNING_FLAG})
  IF(HAVE_${WARNING_FLAG})
    STRING_APPEND(MY_C_WARNING_FLAGS " -${WARNING_FLAG}")
  ENDIF()
ENDMACRO()

MACRO(MY_ADD_CXX_WARNING_FLAG WARNING_FLAG)
  MY_CHECK_CXX_COMPILER_WARNING("-${WARNING_FLAG}" HAS_FLAG)
  IF(HAS_FLAG)
    STRING_APPEND(MY_CXX_WARNING_FLAGS " -${WARNING_FLAG}")
  ENDIF()
ENDMACRO()

#
# Common flags for all versions/compilers
#

# Common warning flags for GCC, G++, Clang and Clang++
SET(MY_WARNING_FLAGS
    "-Wall -Wextra -Wformat-security -Wvla")

# Gives spurious warnings on 32-bit; see GCC bug 81890.
IF(SIZEOF_VOIDP EQUAL 8)
  STRING_APPEND(MY_WARNING_FLAGS " -Wmissing-format-attribute")
ENDIF()

# Clang 6.0 and newer on Windows treat -Wall as -Weverything; use /W4 instead
IF(WIN32_CLANG)
  STRING(REPLACE "-Wall -Wextra" "/W4" MY_WARNING_FLAGS "${MY_WARNING_FLAGS}")
ENDIF()

# Common warning flags for GCC and Clang
SET(MY_C_WARNING_FLAGS "${MY_WARNING_FLAGS} -Wwrite-strings")

# Common warning flags for G++ and Clang++
SET(MY_CXX_WARNING_FLAGS "${MY_WARNING_FLAGS} -Woverloaded-virtual -Wcast-qual")

IF(MY_COMPILER_IS_GNU)
  # The default =3 given by -Wextra is a bit too strict for our code.
  MY_ADD_CXX_WARNING_FLAG("Wimplicit-fallthrough=2")
  MY_ADD_C_WARNING_FLAG("Wjump-misses-init")
  # This is included in -Wall on some platforms, enable it explicitly.
  MY_ADD_C_WARNING_FLAG("Wstringop-truncation")
  MY_ADD_CXX_WARNING_FLAG("Wstringop-truncation")
ENDIF()

#
# Extra flags not supported on all versions/compilers
#

# Only for C++ as C code has some macro usage that is difficult to avoid
IF(MY_COMPILER_IS_GNU)
  MY_ADD_CXX_WARNING_FLAG("Wlogical-op")
ENDIF()

# Extra warning flags for Clang
IF(MY_COMPILER_IS_CLANG)
  STRING_APPEND(MY_C_WARNING_FLAGS " -Wconditional-uninitialized")
  STRING_APPEND(MY_C_WARNING_FLAGS " -Wextra-semi")
  STRING_APPEND(MY_C_WARNING_FLAGS " -Wmissing-noreturn")
  STRING_APPEND(MY_C_WARNING_FLAGS " -Wno-unused-command-line-argument")

  MY_ADD_C_WARNING_FLAG("Wunreachable-code-break")
  MY_ADD_C_WARNING_FLAG("Wunreachable-code-return")
ENDIF()
  
# Extra warning flags for Clang++
IF(MY_COMPILER_IS_CLANG)
  # Disable a few default Clang++ warnings
  STRING_APPEND(MY_CXX_WARNING_FLAGS " -Wno-null-conversion")
  STRING_APPEND(MY_CXX_WARNING_FLAGS " -Wno-unused-private-field")
  STRING_APPEND(MY_CXX_WARNING_FLAGS " -Wno-unused-command-line-argument")

  STRING_APPEND(MY_CXX_WARNING_FLAGS " -Wconditional-uninitialized")
  STRING_APPEND(MY_CXX_WARNING_FLAGS " -Wdeprecated")
  STRING_APPEND(MY_CXX_WARNING_FLAGS " -Wextra-semi")
  STRING_APPEND(MY_CXX_WARNING_FLAGS " -Wheader-hygiene")
  STRING_APPEND(MY_CXX_WARNING_FLAGS " -Wnon-virtual-dtor")
  STRING_APPEND(MY_CXX_WARNING_FLAGS " -Wundefined-reinterpret-cast")

  # clang-5 or older: disable "suggest braces around initialization of subobject" warnings
  IF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6)
    STRING_APPEND(MY_CXX_WARNING_FLAGS " -Wno-missing-braces")
  ENDIF()

  MY_ADD_CXX_WARNING_FLAG("Winconsistent-missing-destructor-override")
  MY_ADD_CXX_WARNING_FLAG("Wshadow-field")

  # Other possible options that give warnings (Clang 6.0):
  # -Wabstract-vbase-init
  # -Wc++2a-compat
  # -Wc++98-compat-pedantic
  # -Wcast-align
  # -Wclass-varargs
  # -Wcomma
  # -Wconversion
  # -Wcovered-switch-default
  # -Wdeprecated-dynamic-exception-spec
  # -Wdisabled-macro-expansion
  # -Wdocumentation
  # -Wdocumentation-pedantic
  # -Wdocumentation-unknown-command
  # -Wdouble-promotion
  # -Wexit-time-destructors
  # -Wfloat-equal
  # -Wformat-nonliteral
  # -Wformat-pedantic
  # -Wglobal-constructors
  # -Wgnu-anonymous-struct
  # -Wgnu-zero-variadic-macro-arguments
  # -Wimplicit-fallthrough
  # -Wkeyword-macro
  # -Wmissing-noreturn
  # -Wmissing-prototypes
  # -Wmissing-variable-declarations
  # -Wnested-anon-types
  # -Wnewline-eof
  # -Wold-style-cast
  # -Wpadded
  # -Wpedantic
  # -Wrange-loop-analysis
  # -Wredundant-parens
  # -Wreserved-id-macro
  # -Wshadow
  # -Wshift-sign-overflow
  # -Wsign-conversion
  # -Wswitch-enum
  # -Wtautological-type-limit-compare
  # -Wtautological-unsigned-enum-zero-compare
  # -Wundefined-func-template
  # -Wunreachable-code
  # -Wunreachable-code-break
  # -Wunreachable-code-return
  # -Wunused-exception-parameter
  # -Wunused-macros
  # -Wunused-member-function
  # -Wunused-template
  # -Wused-but-marked-unused
  # -Wweak-template-vtables
  # -Wweak-vtables
  # -Wzero-as-null-pointer-constant
ENDIF()

# Turn on Werror (warning => error) when using maintainer mode.
IF(MYSQL_MAINTAINER_MODE)
  STRING_APPEND(MY_C_WARNING_FLAGS   " -Werror")
  STRING_APPEND(MY_CXX_WARNING_FLAGS " -Werror")
ENDIF()

# Set warning flags for gcc/g++/clang/clang++
IF(MY_COMPILER_IS_GNU_OR_CLANG)
  STRING_APPEND(CMAKE_C_FLAGS   " ${MY_C_WARNING_FLAGS}")
  STRING_APPEND(CMAKE_CXX_FLAGS " ${MY_CXX_WARNING_FLAGS}")
ENDIF()

MACRO(ADD_WSHADOW_WARNING)
  IF(MY_COMPILER_IS_GNU AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
    ADD_COMPILE_OPTIONS("-Wshadow=local")
  ELSEIF(MY_COMPILER_IS_CLANG)
    ADD_COMPILE_OPTIONS("-Wshadow-uncaptured-local")
  ENDIF()
ENDMACRO()

# When builing with PGO, GCC 9 will report -Wmissing-profile when compiling
# files for which it cannot find profile data. It is valid to disable
# this warning for files we are not currently interested in profiling.
MACRO(DISABLE_MISSING_PROFILE_WARNING)
  IF(FPROFILE_USE)
    MY_CHECK_CXX_COMPILER_WARNING("-Wmissing-profile" HAS_WARN_FLAG)
    IF(HAS_WARN_FLAG)
      STRING_APPEND(CMAKE_CXX_FLAGS " ${HAS_WARN_FLAG}")
    ENDIF()
  ENDIF()
ENDMACRO()
