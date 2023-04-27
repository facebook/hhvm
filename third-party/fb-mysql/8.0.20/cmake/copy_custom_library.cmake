# Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
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

IF(EXISTS "./${library_version}")
  RETURN()
ENDIF()

EXECUTE_PROCESS(
  COMMAND ${CMAKE_COMMAND} -E copy
  "${library_directory}/${library_version}" "./${library_version}"
  )

IF(NOT "${library_version}" STREQUAL "${library_name}")
  EXECUTE_PROCESS(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    "${library_version}" "${library_name}"
    )
ENDIF()

IF(NOT "${library_version}" STREQUAL "${library_soname}")
  EXECUTE_PROCESS(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    "${library_version}" "${library_soname}"
    )
ENDIF()

# Some of the pre-built libraries come without execute bit set.
EXECUTE_PROCESS(
  COMMAND chmod +x "./${library_version}")

# Patch RPATH so that we find NEEDED libraries at load time.
IF(subdir)
  EXECUTE_PROCESS(
    COMMAND ${PATCHELF_EXECUTABLE} --set-rpath "$ORIGIN/.."
    "./${library_version}"
    )
ELSE()
  EXECUTE_PROCESS(
    COMMAND ${PATCHELF_EXECUTABLE} --set-rpath "$ORIGIN" "./${library_version}"
    )
ENDIF()
