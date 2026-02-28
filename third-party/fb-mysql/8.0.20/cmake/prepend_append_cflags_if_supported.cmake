# Copyright (c) 2017, Percona and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

# helper macros to prepend or append c and cxx flags if supported by compiler

INCLUDE (CheckCCompilerFlag)
INCLUDE (CheckCXXCompilerFlag)

MACRO (prepend_cflags_if_supported)
  FOREACH (flag ${ARGN})
    STRING (REGEX REPLACE "-" "_" temp_flag ${flag})
    check_c_compiler_flag (${flag} HAVE_C_${temp_flag})
    IF (HAVE_C_${temp_flag})
      SET (CMAKE_C_FLAGS "${flag} ${CMAKE_C_FLAGS}")
    ENDIF ()
    check_cxx_compiler_flag (${flag} HAVE_CXX_${temp_flag})
    IF (HAVE_CXX_${temp_flag})
      SET (CMAKE_CXX_FLAGS "${flag} ${CMAKE_CXX_FLAGS}")
    ENDIF ()
  ENDFOREACH (flag)
ENDMACRO (prepend_cflags_if_supported)

MACRO (append_cflags_if_supported)
  FOREACH (flag ${ARGN})
    STRING (REGEX REPLACE "-" "_" temp_flag ${flag})
    check_c_compiler_flag (${flag} HAVE_C_${temp_flag})
    IF (HAVE_C_${temp_flag})
      SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
    ENDIF ()
    check_cxx_compiler_flag (${flag} HAVE_CXX_${temp_flag})
    IF (HAVE_CXX_${temp_flag})
      SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    ENDIF ()
  ENDFOREACH (flag)
ENDMACRO (append_cflags_if_supported)

MACRO (remove_compile_flags)
  FOREACH (flag ${ARGN})
    IF(CMAKE_C_FLAGS MATCHES ${flag})
      STRING(REGEX REPLACE "${flag}( |$)" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    ENDIF(CMAKE_C_FLAGS MATCHES ${flag})
    IF(CMAKE_CXX_FLAGS MATCHES ${flag})
      STRING(REGEX REPLACE "${flag}( |$)" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    ENDIF(CMAKE_CXX_FLAGS MATCHES ${flag})
  ENDFOREACH (flag)
ENDMACRO (remove_compile_flags)

## ADD_CXX_COMPILE_FLAGS_TO_FILES(<flags> FILES <source files>)
## Add compile flags to given c++ files for all supported flags
MACRO(ADD_CXX_COMPILE_FLAGS_TO_FILES)
  SET(FILES "")
  SET(FLAGS "")
  SET(FILES_SEEN)
  FOREACH(ARG ${ARGV})
    IF("x${ARG}" STREQUAL "xFILES")
      SET(FILES_SEEN 1)
    ELSEIF(FILES_SEEN)
      LIST(APPEND FILES ${ARG})
    ELSE()
      LIST(APPEND FLAGS ${ARG})
    ENDIF()
  ENDFOREACH()
  SET(CHECKED_FLAGS "")
  FOREACH (flag ${FLAGS})
    STRING (REGEX REPLACE "-" "_" temp_flag ${flag})
    MY_CHECK_CXX_COMPILER_FLAG(${flag} HAVE_CXX_${temp_flag})
    IF (HAVE_CXX_${temp_flag})
      STRING_APPEND(CHECKED_FLAGS "${flag}")
    ENDIF()
  ENDFOREACH(flag)
  ADD_COMPILE_FLAGS(${FILES} COMPILE_FLAGS ${CHECKED_FLAGS})
ENDMACRO(ADD_CXX_COMPILE_FLAGS_TO_FILES)
