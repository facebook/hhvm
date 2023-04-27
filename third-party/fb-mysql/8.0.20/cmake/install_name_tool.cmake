# Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.
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

# Skip execution below if libraries are already in place,
# otherwise all executables will be re-linked on subsequent builds.
IF(EXISTS "./${OPENSSL_VERSION}")
  EXECUTE_PROCESS(
    COMMAND otool -L ${OPENSSL_VERSION}
    OUTPUT_VARIABLE OTOOL_OPENSSL_DEPS
    )

  STRING(REPLACE "\n" ";" DEPS_LIST ${OTOOL_OPENSSL_DEPS})
  FOREACH(LINE ${DEPS_LIST})
    IF(${LINE} MATCHES "@loader_path/${CRYPTO_VERSION}")
      MESSAGE(STATUS "dependency for ${OPENSSL_VERSION} ${LINE}")
      RETURN()
    ENDIF()
  ENDFOREACH()
ENDIF()

EXECUTE_PROCESS(
  COMMAND ${CMAKE_COMMAND} -E copy
  "${CRYPTO_FULL_NAME}" "./${CRYPTO_VERSION}"
  COMMAND ${CMAKE_COMMAND} -E copy
  "${OPENSSL_FULL_NAME}" "./${OPENSSL_VERSION}"
  COMMAND ${CMAKE_COMMAND} -E create_symlink
  "${CRYPTO_VERSION}" "${CRYPTO_NAME}"
  COMMAND ${CMAKE_COMMAND} -E create_symlink
  "${OPENSSL_VERSION}" "${OPENSSL_NAME}"
)

EXECUTE_PROCESS(
  COMMAND chmod +w "${CRYPTO_VERSION}" "${OPENSSL_VERSION}"
)

# install_name_tool -change old new file
EXECUTE_PROCESS(COMMAND install_name_tool -change
  "${OPENSSL_DEPS}" "@loader_path/${CRYPTO_VERSION}" "${OPENSSL_VERSION}"
)
