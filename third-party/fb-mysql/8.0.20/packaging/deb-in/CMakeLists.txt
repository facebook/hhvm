# Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.
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

IF(NOT LINUX)
  RETURN()
ENDIF()

# bionic, buster, etc
IF (NOT DEFINED DEB_CODENAME)
  execute_process(
    COMMAND lsb_release -cs
    OUTPUT_VARIABLE DEB_CODENAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  SET (DEB_CODENAME ${DEB_CODENAME} CACHE STRING "")
ENDIF()

# debian, ubuntu
IF (NOT DEFINED DEB_ID)
  execute_process(
    COMMAND lsb_release -is
    OUTPUT_VARIABLE DEB_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  SET (DEB_ID ${DEB_ID} CACHE STRING "")
ENDIF()

# 18.04, 9.x, etc
IF (NOT DEFINED DEB_RELEASE)
  execute_process(
    COMMAND lsb_release -rs
    OUTPUT_VARIABLE DEB_RELEASE
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  SET (DEB_RELEASE ${DEB_RELEASE} CACHE STRING "")
ENDIF()

# Change uppercase first letter of Ubuntu/Debian
string(TOLOWER "${DEB_ID}" DEB_ID)

IF (WITH_NDBCLUSTER)
  SET (DEB_BASE_PRODUCT "cluster-")
  SET (DEB_BASE_PRODUCTC "Cluster ")
  SET (DEB_BASE_VERSION ${MYSQL_CLUSTER_VERSION})
  SET (DEB_CONTROL_SERVER_CONFLICT_NDB "mysql-community-server, mysql-commercial-server")
  SET (DEB_CONTROL_CLIENT_CONFLICT_NDB "mysql-community-client, mysql-commercial-client")
ELSE()
  SET (DEB_BASE_PRODUCT "")
  SET (DEB_BASE_PRODUCTC "")
  SET (DEB_BASE_VERSION ${VERSION})
  SET (DEB_CONTROL_SERVER_CONFLICT_NDB "mysql-cluster-community-server, mysql-cluster-commercial-server")
  SET (DEB_CONTROL_CLIENT_CONFLICT_NDB "mysql-cluster-community-client, mysql-cluster-commercial-client")
ENDIF()
# Timestamp for use in debian/changelog
IF (NOT DEFINED DEB_CHANGELOG_TIMESTAMP)
  execute_process(
    COMMAND date --rfc-2822
    OUTPUT_VARIABLE DEB_CHANGELOG_TIMESTAMP
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  SET (DEB_CHANGELOG_TIMESTAMP ${DEB_CHANGELOG_TIMESTAMP} CACHE STRING "")
ENDIF()
# Commercial or community
IF (DEB_PRODUCT STREQUAL "commercial")
  SET (DEB_COPYRIGHT_UPSTREAMNAME "MySQL Commercial Server ${MYSQL_BASE_VERSION}")
  SET (DEB_PRODUCTNAME "${DEB_BASE_PRODUCT}commercial")
  SET (DEB_PRODUCTNAMEC "${DEB_BASE_PRODUCTC}Enterprise")
  SET (DEB_NOTPRODUCTNAME "${DEB_BASE_PRODUCT}community")
  SET (DEB_LICENSENAME "Commercial")
  SET (DEB_SERVERVERSION "${DEB_BASE_VERSION}+commercial-1")
  # List of plugins that are only in commercial packages
  # Plugins that are in both community and commercial should NOT be added here
  SET (DEB_INSTALL_SERVER_PLUGINS
"
usr/lib/mysql/plugin/audit_log.so
usr/lib/mysql/plugin/authentication_pam.so
usr/lib/mysql/plugin/authentication_ldap_sasl.so
usr/lib/mysql/plugin/authentication_ldap_simple.so
usr/lib/mysql/plugin/data_masking.so
usr/lib/mysql/plugin/keyring_okv.so
usr/lib/mysql/plugin/keyring_encrypted_file.so
usr/lib/mysql/plugin/keyring_hashicorp.so
usr/lib/mysql/plugin/openssl_udf.so
usr/lib/mysql/plugin/thread_pool.so
usr/lib/mysql/plugin/firewall.so
usr/lib/mysql/plugin/component_test_page_track_component.so
")

  IF (DEB_AWS_SDK)
    SET (DEB_CMAKE_EXTRAS "${DEB_CMAKE_EXTRAS} -DWITH_KEYRING_AWS=ON -DWITH_AWS_SDK=${DEB_AWS_SDK} -DDEB_AWS_SDK=${DEB_AWS_SDK}")
    SET (DEB_INSTALL_SERVER_PLUGINS "${DEB_INSTALL_SERVER_PLUGINS}
usr/lib/mysql/plugin/keyring_aws.so
")
  ENDIF()

  IF (WITH_CURL)
    SET (DEB_CMAKE_EXTRAS "${DEB_CMAKE_EXTRAS} -DWITH_CURL=${WITH_CURL}")
  ENDIF()
  SET (DEB_REMOVEPATTERN "gpl.in")
  INCLUDE (deb_commercial.cmake)
ELSE()
  SET (DEB_PRODUCT "${DEB_BASE_PRODUCT}community")
  SET (DEB_COPYRIGHT_UPSTREAMNAME "MySQL Server ${MYSQL_BASE_VERSION}")
  SET (DEB_PRODUCTNAME "${DEB_BASE_PRODUCT}community")
  SET (DEB_PRODUCTNAMEC "${DEB_BASE_PRODUCTC}Community")
  SET (DEB_NOTPRODUCTNAME "${DEB_BASE_PRODUCT}commercial")
  SET (DEB_LICENSENAME "GPL")
  SET (DEB_SERVERVERSION "${DEB_BASE_VERSION}-1")
  SET (DEB_REMOVEPATTERN "com.in")
ENDIF()

# Build with default ssl. May be overridden if we don't support default ssl version
SET (DEB_CONTROL_BDEPS_SSL ",libssl-dev")
# Generate debug symbol packages (this is done automatically in Debian9+)
SET (DEB_RULES_STRIP_DEFAULT
"
override_dh_strip:
	dh_strip -pmysql-${DEB_PRODUCTNAME}-server-core --dbg-package=mysql-${DEB_PRODUCTNAME}-server-core-dbgsym
	dh_strip -pmysql-${DEB_PRODUCTNAME}-client-core --dbg-package=mysql-${DEB_PRODUCTNAME}-client-core-dbgsym
	dh_strip -pmysql-${DEB_PRODUCTNAME}-server --dbg-package=mysql-${DEB_PRODUCTNAME}-server-dbgsym
	dh_strip -pmysql-${DEB_PRODUCTNAME}-server-debug --dbg-package=mysql-${DEB_PRODUCTNAME}-server-debug-dbgsym
	dh_strip -pmysql-${DEB_PRODUCTNAME}-client --dbg-package=mysql-${DEB_PRODUCTNAME}-client-dbgsym
	dh_strip -pmysql-${DEB_PRODUCTNAME}-test --dbg-package=mysql-${DEB_PRODUCTNAME}-test-dbgsym
	dh_strip -pmysql-${DEB_PRODUCTNAME}-test-debug --dbg-package=mysql-${DEB_PRODUCTNAME}-test-debug-dbgsym
	dh_strip -pmysql-server
	dh_strip -pmysql-client
	dh_strip -pmysql-testsuite
	dh_strip -plibmysqlclient21 --dbg-package=libmysqlclient21-dbgsym
	dh_strip -plibmysqlclient-dev
")
SET (DEB_CONTROL_DBGSYM_DEFAULT
"
Package: libmysqlclient21-dbgsym
Architecture: any
Section: debug
Depends: libmysqlclient21 (=\${binary:Version}), \${misc:Depends}
Description: Debugging symbols for client library

Package: mysql-${DEB_PRODUCTNAME}-test-dbgsym
Architecture: any
Section: debug
Depends: mysql-${DEB_PRODUCTNAME}-test (= \${binary:Version}), \${misc:Depends}
Description: Debugging symbols for test suite

Package: mysql-${DEB_PRODUCTNAME}-test-debug-dbgsym
Architecture: any
Section: debug
Depends: mysql-${DEB_PRODUCTNAME}-test-debug (= \${binary:Version}), \${misc:Depends}
Description: Debugging symbols for test suite debug binaries

Package: mysql-${DEB_PRODUCTNAME}-client-core-dbgsym
Architecture: any
Section: debug
Depends: mysql-${DEB_PRODUCTNAME}-client-core (= \${binary:Version}), \${misc:Depends}
Description: Debugging symbols for client core

Package: mysql-${DEB_PRODUCTNAME}-server-core-dbgsym
Architecture: any
Section: debug
Depends: mysql-${DEB_PRODUCTNAME}-server-core (= \${binary:Version}), \${misc:Depends}
Description: Debugging symbols for server core

Package: mysql-${DEB_PRODUCTNAME}-server-dbgsym
Architecture: any
Section: debug
Depends: mysql-${DEB_PRODUCTNAME}-server (= \${binary:Version}), \${misc:Depends}
Description: Debugging symbols for server

Package: mysql-${DEB_PRODUCTNAME}-server-debug-dbgsym
Architecture: any
Section: debug
Depends: mysql-${DEB_PRODUCTNAME}-server-debug (= \${binary:Version}), \${misc:Depends}
Description: Debugging symbols for server debug binaries

Package: mysql-${DEB_PRODUCTNAME}-client-dbgsym
Architecture: any
Section: debug
Depends: mysql-${DEB_PRODUCTNAME}-client (= \${binary:Version}), \${misc:Depends}
Description: Debugging symbols for client
")

# lsb_release on Debian reports the full version number, e.g. 9.9, and we just want the major version
IF(DEB_ID STREQUAL "debian")
  string(REGEX REPLACE "\\..*" "" DEB_RELEASE "${DEB_RELEASE}")
ENDIF()


# ubuntu18.04, debian9, etc
SET (DEB_PLATFORMRELEASE "${DEB_ID}${DEB_RELEASE}")

# Only used on Ubuntu
SET (DEB_CONTROL_BDEPS "")
SET (DEB_RULES_INSTALL_APPARMOR "")
SET (DEB_RULES_APPARMOR_LOAD "")
SET (DEB_INSTALL_SERVER_APPARMOR "")
SET (DEB_INIT_APPARMOR "")
# Only used on Ubuntu 16.04 for manually created debug symbol packages
SET (DEB_CONTROL_DBGSYM "")
SET (DEB_RULES_STRIP "")

# Used for testing unreleased gcc versions on e.g. Debian unstable
IF (DEFINED DEB_GCC_SNAPSHOT)
  SET (DEB_CMAKE_EXTRAS "${DEB_CMAKE_EXTRAS} -DCMAKE_C_COMPILER=/usr/lib/gcc-snapshot/bin/gcc -DCMAKE_CXX_COMPILER=/usr/lib/gcc-snapshot/bin/g++ -DMYSQL_MAINTAINER_MODE=0 -DCMAKE_CXX_COMPILER_LAUNCHER=ccache")
ENDIF()

# Enable various apparmor systems for Ubuntu
IF(DEB_ID STREQUAL "ubuntu")
  SET (DEB_CONTROL_BDEPS ", dh-apparmor")
  SET (DEB_RULES_INSTALL_APPARMOR
    "
	install -g root -o root -m 0644 -D debian/extra/apparmor-profile debian/tmp/etc/apparmor.d/usr.sbin.mysqld
	install -g root -o root -m 0644 -D debian/extra/apparmor-profile-router debian/tmp/etc/apparmor.d/usr.bin.mysqlrouter
")
  SET (DEB_RULES_APPARMOR_LOAD
  "
	dh_apparmor -pmysql-${DEB_PRODUCTNAME}-server --profile-name=usr.sbin.mysqld
	dh_apparmor -pmysql-router-${DEB_PRODUCTNAME} --profile-name=usr.bin.mysqlrouter
  ")
  SET (DEB_INSTALL_SERVER_APPARMOR "etc/apparmor.d/usr.sbin.mysqld")
  SET (DEB_INSTALL_ROUTER_APPARMOR "etc/apparmor.d/usr.bin.mysqlrouter")
  SET (DEB_INIT_APPARMOR "/lib/apparmor/profile-load usr.sbin.mysqld")
  
  # Ubuntu 16.04 doesn't automatically create debug symbol packages
  IF(DEB_CODENAME STREQUAL "xenial")
    SET (DEB_CONTROL_DBGSYM ${DEB_CONTROL_DBGSYM_DEFAULT})
    SET (DEB_RULES_STRIP ${DEB_RULES_STRIP_DEFAULT})
  ENDIF()
ENDIF()

IF (NOT DEFINED DEB_NOROUTER)
  INCLUDE(deb_router.cmake)
ENDIF()
IF (WITH_NDBCLUSTER)
  INCLUDE(deb_ndb.cmake)
ENDIF()
if (DEFINED DEB_WITH_DEBUG)
  INCLUDE(deb_debug.cmake)
ENDIF()
# All files are configured and copied to the debian/ directory, which is used 
# by debuild to make the packages
SET (DEB_ROOT ${CMAKE_SOURCE_DIR}/packaging/deb-in)
FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/debian)
FILE(GLOB_RECURSE SOURCEFILES RELATIVE "${DEB_ROOT}" "${DEB_ROOT}/*")
# Skip the copyright file from the "other" product type
LIST(REMOVE_ITEM SOURCEFILES "copyright.${DEB_REMOVEPATTERN}")
LIST(REMOVE_ITEM SOURCEFILES "CMakeLists.txt")
# Don't install systemd file on systems without systemd
IF(DEB_INSTALL_SERVER_SYSTEMD STREQUAL "")
	LIST(REMOVE_ITEM SOURCEFILES "mysql-packagesource-server.mysql.service.in")
ENDIF()
FOREACH (SOURCEFILE ${SOURCEFILES})
  STRING(REGEX REPLACE "-packagesource" "-${DEB_PRODUCTNAME}"
    TMPFILE ${SOURCEFILE})
  # Strip away the input file endings from the destination filenames
  STRING(REGEX REPLACE ".in$" "" TMPFILE ${TMPFILE})
  STRING(REGEX REPLACE ".gpl$" "" TMPFILE ${TMPFILE})
  STRING(REGEX REPLACE ".com$" "" DESTFILE ${TMPFILE})
  CONFIGURE_FILE("${DEB_ROOT}/${SOURCEFILE}"
    "${CMAKE_BINARY_DIR}/debian/${DESTFILE}" @ONLY)
ENDFOREACH()
EXECUTE_PROCESS(
  COMMAND chmod +x ${CMAKE_BINARY_DIR}/debian/rules
)

