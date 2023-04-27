# Copyright (c) 2010, 2020, Oracle and/or its affiliates. All rights reserved.
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

# The purpose of this file is to set the default installation layout.
#
# The current choices of installation layout are:
#
#  STANDALONE
#    Build with prefix=/usr/local/mysql, create tarball with install prefix="."
#    and relative links.
#
#  RPM
#    Build as per default RPM layout, with prefix=/usr
#
#  DEB
#    Similar to RPM layout.
#
#  SVR4
#    Solaris package layout suitable for pkg* tools, prefix=/opt/mysql/mysql
#
#  TARGZ
#    Build with prefix=/usr/local/mysql, create tarball with install prefix="."
#    and relative links.
#
# To force a directory layout, use -DINSTALL_LAYOUT=<layout>.
#
# The default is STANDALONE.
#
# There is the possibility to further fine-tune installation directories.
# Several variables can be overwritten:
#
# - INSTALL_BINDIR          (directory with client executables and scripts)
# - INSTALL_SBINDIR         (directory with mysqld)
#
# - INSTALL_LIBDIR          (directory with client libraries)
# - INSTALL_PRIV_LIBDIR     (directory with mysql private libraries)
# - INSTALL_PLUGINDIR       (directory for plugins)
#
# - INSTALL_INCLUDEDIR      (directory for MySQL headers)
#
# - INSTALL_DOCDIR          (documentation)
# - INSTALL_DOCREADMEDIR    (readme and similar)
# - INSTALL_MANDIR          (man pages)
# - INSTALL_INFODIR         (info pages)
#
# - INSTALL_SHAREDIR        (location of aclocal/mysql.m4)
# - INSTALL_MYSQLSHAREDIR   (MySQL character sets and localized error messages)
# - INSTALL_MYSQLTESTDIR    (mysql-test)
# - INSTALL_SUPPORTFILESDIR (various extra support files)
#
# - INSTALL_MYSQLDATADIR    (data directory)
# - INSTALL_MYSQLKEYRING    (keyring directory)
# - INSTALL_SECURE_FILE_PRIVDIR (--secure-file-priv directory)
# - INSTALL_PARTIAL_REVOKES (--partial-revokes)
#
# When changing this page,  _please_ do not forget to update public Wiki
# https://dev.mysql.com/doc/refman/8.0/en/source-configuration-options.html#option_cmake_install_layout

IF(NOT INSTALL_LAYOUT)
  SET(DEFAULT_INSTALL_LAYOUT "STANDALONE")
ENDIF()

SET(INSTALL_LAYOUT "${DEFAULT_INSTALL_LAYOUT}"
  CACHE STRING "Installation directory layout. Options are: TARGZ (as in tar.gz installer), STANDALONE, RPM, DEB, SVR4"
  )

IF(UNIX)
  IF(INSTALL_LAYOUT MATCHES "RPM")
    SET(default_prefix "/usr")
  ELSEIF(INSTALL_LAYOUT MATCHES "DEB")
    SET(default_prefix "/usr")
  ELSEIF(INSTALL_LAYOUT MATCHES "SVR4")
    SET(default_prefix "/opt/mysql/mysql")
  ELSE()
    SET(default_prefix "/usr/local/mysql")
  ENDIF()
  IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CMAKE_INSTALL_PREFIX ${default_prefix}
      CACHE PATH "install prefix" FORCE)
  ENDIF()
  SET(VALID_INSTALL_LAYOUTS
    "RPM" "DEB" "SVR4" "TARGZ" "STANDALONE")
  LIST(FIND VALID_INSTALL_LAYOUTS "${INSTALL_LAYOUT}" ind)
  IF(ind EQUAL -1)
    MESSAGE(FATAL_ERROR "Invalid INSTALL_LAYOUT parameter:${INSTALL_LAYOUT}."
    " Choose between ${VALID_INSTALL_LAYOUTS}" )
  ENDIF()

  SET(SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc"
    CACHE PATH "config directory (for my.cnf)")
  MARK_AS_ADVANCED(SYSCONFDIR)
ENDIF()

IF(LINUX AND INSTALL_LAYOUT MATCHES "STANDALONE")
  SET(LINUX_STANDALONE 1)
ENDIF()

IF(LINUX AND INSTALL_LAYOUT MATCHES "RPM")
  SET(LINUX_RPM 1)
ENDIF()

IF(LINUX AND INSTALL_LAYOUT MATCHES "DEB")
  SET(LINUX_DEB 1)
ENDIF()

IF(WIN32)
  SET(VALID_INSTALL_LAYOUTS "TARGZ" "STANDALONE")
  LIST(FIND VALID_INSTALL_LAYOUTS "${INSTALL_LAYOUT}" ind)
  IF(ind EQUAL -1)
    MESSAGE(FATAL_ERROR "Invalid INSTALL_LAYOUT parameter:${INSTALL_LAYOUT}."
    " Choose between ${VALID_INSTALL_LAYOUTS}" )
  ENDIF()
ENDIF()

#
# DEFAULT_SECURE_FILE_PRIV_DIR
#
IF(INSTALL_LAYOUT MATCHES "STANDALONE")
  SET(secure_file_priv_path "NULL")
ELSEIF(INSTALL_LAYOUT MATCHES "RPM" OR
       INSTALL_LAYOUT MATCHES "SVR4" OR
       INSTALL_LAYOUT MATCHES "DEB")
  SET(secure_file_priv_path "/var/lib/mysql-files")
ELSE()
  SET(secure_file_priv_path "${default_prefix}/mysql-files")
ENDIF()

#
# STANDALONE layout
#
SET(INSTALL_BINDIR_STANDALONE           "bin")
SET(INSTALL_SBINDIR_STANDALONE          "bin")
#
SET(INSTALL_LIBDIR_STANDALONE           "lib")
SET(INSTALL_PRIV_LIBDIR_STANDALONE      "lib/private")
SET(INSTALL_PLUGINDIR_STANDALONE        "lib/plugin")
#
SET(INSTALL_INCLUDEDIR_STANDALONE       "include")
#
SET(INSTALL_DOCDIR_STANDALONE           "docs")
SET(INSTALL_DOCREADMEDIR_STANDALONE     ".")
SET(INSTALL_MANDIR_STANDALONE           "man")
SET(INSTALL_INFODIR_STANDALONE          "docs")
#
SET(INSTALL_SHAREDIR_STANDALONE         "share")
SET(INSTALL_MYSQLSHAREDIR_STANDALONE    "share")
SET(INSTALL_MYSQLTESTDIR_STANDALONE     "mysql-test")
SET(INSTALL_SUPPORTFILESDIR_STANDALONE  "support-files")
#
SET(INSTALL_MYSQLDATADIR_STANDALONE     "data")
SET(INSTALL_MYSQLKEYRINGDIR_STANDALONE  "keyring")
SET(INSTALL_SECURE_FILE_PRIVDIR_STANDALONE ${secure_file_priv_path})

#
# TARGZ layout
#
SET(INSTALL_BINDIR_TARGZ           "bin")
SET(INSTALL_SBINDIR_TARGZ          "bin")
#
SET(INSTALL_LIBDIR_TARGZ           "lib")
SET(INSTALL_PRIV_LIBDIR_TARGZ      "lib/mysql/private")
SET(INSTALL_PLUGINDIR_TARGZ        "lib/plugin")
#
SET(INSTALL_INCLUDEDIR_TARGZ       "include")
#
SET(INSTALL_DOCDIR_TARGZ           "docs")
SET(INSTALL_DOCREADMEDIR_TARGZ     ".")
SET(INSTALL_MANDIR_TARGZ           "man")
SET(INSTALL_INFODIR_TARGZ          "docs")
#
SET(INSTALL_SHAREDIR_TARGZ         "share")
SET(INSTALL_MYSQLSHAREDIR_TARGZ    "share")
SET(INSTALL_MYSQLTESTDIR_TARGZ     "mysql-test")
SET(INSTALL_SUPPORTFILESDIR_TARGZ  "support-files")
#
SET(INSTALL_MYSQLDATADIR_TARGZ     "data")
SET(INSTALL_MYSQLKEYRINGDIR_TARGZ  "keyring")
SET(INSTALL_SECURE_FILE_PRIVDIR_TARGZ ${secure_file_priv_path})

#
# RPM layout
#
SET(INSTALL_BINDIR_RPM                  "bin")
SET(INSTALL_SBINDIR_RPM                 "sbin")
#
IF(CMAKE_SYSTEM_PROCESSOR IN_LIST KNOWN_64BIT_ARCHITECTURES)
  SET(INSTALL_LIBDIR_RPM                "lib64/mysql")
  SET(INSTALL_PRIV_LIBDIR_RPM           "lib64/mysql/private")
  IF(CMAKE_BUILD_TYPE_UPPER STREQUAL "DEBUG")
    SET(INSTALL_PLUGINDIR_RPM           "lib64/mysql/plugin/debug")
  ELSE()
    SET(INSTALL_PLUGINDIR_RPM           "lib64/mysql/plugin")
  ENDIF()
ELSE()
  SET(INSTALL_LIBDIR_RPM                "lib/mysql")
  SET(INSTALL_PRIV_LIBDIR_RPM           "lib/mysql/private")
  IF(CMAKE_BUILD_TYPE_UPPER STREQUAL "DEBUG")
    SET(INSTALL_PLUGINDIR_RPM           "lib/mysql/plugin/debug")
  ELSE()
    SET(INSTALL_PLUGINDIR_RPM           "lib/mysql/plugin")
  ENDIF()
ENDIF()
#
SET(INSTALL_INCLUDEDIR_RPM              "include/mysql")
#
#SET(INSTALL_DOCDIR_RPM                 unset - installed directly by RPM)
#SET(INSTALL_DOCREADMEDIR_RPM           unset - installed directly by RPM)
SET(INSTALL_INFODIR_RPM                 "share/info")
SET(INSTALL_MANDIR_RPM                  "share/man")
#
SET(INSTALL_SHAREDIR_RPM                "share")
SET(INSTALL_MYSQLSHAREDIR_RPM           "share/mysql-${MYSQL_BASE_VERSION}")
SET(INSTALL_MYSQLTESTDIR_RPM            "share/mysql-test")
SET(INSTALL_SUPPORTFILESDIR_RPM         "share/mysql-${MYSQL_BASE_VERSION}")
#
SET(INSTALL_MYSQLDATADIR_RPM            "/var/lib/mysql")
SET(INSTALL_MYSQLKEYRINGDIR_RPM         "/var/lib/mysql-keyring")
SET(INSTALL_SECURE_FILE_PRIVDIR_RPM     ${secure_file_priv_path})

#
# DEB layout
#
SET(INSTALL_BINDIR_DEB                  "bin")
SET(INSTALL_SBINDIR_DEB                 "sbin")
#
SET(INSTALL_LIBDIR_DEB                  "lib")
SET(INSTALL_PRIV_LIBDIR_DEB             "lib/mysql/private")
IF(CMAKE_BUILD_TYPE_UPPER STREQUAL "DEBUG")
  SET(INSTALL_PLUGINDIR_DEB             "lib/mysql/plugin/debug")
ELSE()
  SET(INSTALL_PLUGINDIR_DEB             "lib/mysql/plugin")
ENDIF()
#
SET(INSTALL_INCLUDEDIR_DEB              "include/mysql")
#
SET(INSTALL_DOCDIR_DEB                  "share/mysql-${MYSQL_BASE_VERSION}/docs")
SET(INSTALL_DOCREADMEDIR_DEB            "share/mysql-${MYSQL_BASE_VERSION}")
SET(INSTALL_MANDIR_DEB                  "share/man")
SET(INSTALL_INFODIR_DEB                 "share/mysql/docs")
#
SET(INSTALL_SHAREDIR_DEB                "share")
SET(INSTALL_MYSQLSHAREDIR_DEB           "share/mysql-${MYSQL_BASE_VERSION}")
SET(INSTALL_MYSQLTESTDIR_DEB            "lib/mysql-test")
SET(INSTALL_SUPPORTFILESDIR_DEB         "share/mysql-${MYSQL_BASE_VERSION}")
#
SET(INSTALL_MYSQLDATADIR_DEB            "/var/lib/mysql")
SET(INSTALL_MYSQLKEYRINGDIR_DEB         "/var/lib/mysql-keyring")
SET(INSTALL_SECURE_FILE_PRIVDIR_DEB     ${secure_file_priv_path})

#
# SVR4 layout
#
SET(INSTALL_BINDIR_SVR4                 "bin")
SET(INSTALL_SBINDIR_SVR4                "bin")
#
SET(INSTALL_LIBDIR_SVR4                 "lib")
SET(INSTALL_PRIV_LIBDIR_SVR4            "lib/private")
SET(INSTALL_PLUGINDIR_SVR4              "lib/plugin")
#
SET(INSTALL_INCLUDEDIR_SVR4             "include")
#
SET(INSTALL_DOCDIR_SVR4                 "docs")
SET(INSTALL_DOCREADMEDIR_SVR4           ".")
SET(INSTALL_MANDIR_SVR4                 "man")
SET(INSTALL_INFODIR_SVR4                "docs")
#
SET(INSTALL_SHAREDIR_SVR4               "share")
SET(INSTALL_MYSQLSHAREDIR_SVR4          "share")
SET(INSTALL_MYSQLTESTDIR_SVR4           "mysql-test")
SET(INSTALL_SUPPORTFILESDIR_SVR4        "support-files")
#
SET(INSTALL_MYSQLDATADIR_SVR4           "/var/lib/mysql")
SET(INSTALL_MYSQLKEYRINGDIR_SVR4        "/var/lib/mysql-keyring")
SET(INSTALL_SECURE_FILE_PRIVDIR_SVR4    ${secure_file_priv_path})


# Clear cached variables if install layout was changed
IF(OLD_INSTALL_LAYOUT)
  IF(NOT OLD_INSTALL_LAYOUT STREQUAL INSTALL_LAYOUT)
    SET(FORCE FORCE)
  ENDIF()
ENDIF()
SET(OLD_INSTALL_LAYOUT ${INSTALL_LAYOUT} CACHE INTERNAL "")

# Set INSTALL_FOODIR variables for chosen layout (for example, INSTALL_BINDIR
# will be defined  as ${INSTALL_BINDIR_STANDALONE} by default if STANDALONE
# layout is chosen)
FOREACH(var
    BIN
    DOC
    DOCREADME
    INCLUDE
    INFO
    LIB
    MAN
    MYSQLDATA
    MYSQLKEYRING
    MYSQLSHARE
    MYSQLTEST
    PLUGIN
    PLUGINTEST
    PRIV_LIB
    SBIN
    SECURE_FILE_PRIV
    SHARE
    SUPPORTFILES
    )
  SET(INSTALL_${var}DIR
    ${INSTALL_${var}DIR_${INSTALL_LAYOUT}}
    CACHE STRING "${var} installation directory" ${FORCE})
  MARK_AS_ADVANCED(INSTALL_${var}DIR)
ENDFOREACH()

#
# Set DEFAULT_SECURE_FILE_PRIV_DIR
# This is used as default value for --secure-file-priv
#
IF(INSTALL_SECURE_FILE_PRIVDIR)
  SET(DEFAULT_SECURE_FILE_PRIV_DIR "\"${INSTALL_SECURE_FILE_PRIVDIR}\""
      CACHE INTERNAL "default --secure-file-priv directory" FORCE)
ELSE()
  SET(DEFAULT_SECURE_FILE_PRIV_DIR \"\"
      CACHE INTERNAL "default --secure-file-priv directory" FORCE)
ENDIF()

#
# Set DEFAULT_PARTIAL_REVOKES
# This is used as default value for --partial-revokes
#
IF(PARTIAL_REVOKES_DEFAULT)
  SET(DEFAULT_PARTIAL_REVOKES 1
      CACHE INTERNAL "default --partial-revokes" FORCE)
ELSE()
  SET(DEFAULT_PARTIAL_REVOKES 0
      CACHE INTERNAL "default --partial-revokes" FORCE)
ENDIF()


# Install layout for router, follows the same pattern as above.
#

IF("${ROUTER_INSTALL_DOCDIR}" STREQUAL "")
  SET(ROUTER_INSTALL_DOCDIR "${INSTALL_DOCDIR}")
ENDIF()

IF(NOT ROUTER_INSTALL_LAYOUT)
  SET(DEFAULT_ROUTER_INSTALL_LAYOUT "${INSTALL_LAYOUT}")
ENDIF()

SET(ROUTER_INSTALL_LAYOUT "${DEFAULT_ROUTER_INSTALL_LAYOUT}"
  CACHE
  STRING
  "Installation directory layout. Options are: STANDALONE RPM DEB SVR4 TARGZ")

# If we are shared STANDALONE with the the server, we shouldn't write
# into the server's data/ as that would create a "schemadir" in
# mysql-servers sense
#
# STANDALONE layout
#
SET(ROUTER_INSTALL_CONFIGDIR_STANDALONE  ".")
SET(ROUTER_INSTALL_DATADIR_STANDALONE    "var/lib/mysqlrouter")
SET(ROUTER_INSTALL_LOGDIR_STANDALONE     ".")
SET(ROUTER_INSTALL_RUNTIMEDIR_STANDALONE "run")

SET(ROUTER_INSTALL_BINDIR_STANDALONE      "bin")
IF(LINUX)
  SET(ROUTER_INSTALL_LIBDIR_STANDALONE    "lib/mysqlrouter/private")
ELSE()
  SET(ROUTER_INSTALL_LIBDIR_STANDALONE    "lib")
ENDIF()
IF(WIN32)
  SET(ROUTER_INSTALL_PLUGINDIR_STANDALONE "lib")
ELSE()
  SET(ROUTER_INSTALL_PLUGINDIR_STANDALONE "lib/mysqlrouter")
ENDIF()

#
# TARGZ layout
#
FOREACH(var
    CONFIG
    DATA
    LOG
    RUNTIME
    BIN
    LIB
    PLUGIN
    )
  SET(ROUTER_INSTALL_${var}DIR_TARGZ ${ROUTER_INSTALL_${var}DIR_STANDALONE})
ENDFOREACH()

#
# RPM layout
#
SET(ROUTER_INSTALL_CONFIGDIR_RPM    "/etc/mysqlrouter")
SET(ROUTER_INSTALL_DATADIR_RPM      "/var/lib/mysqlrouter")
SET(ROUTER_INSTALL_LOGDIR_RPM       "/var/log/mysqlrouter")
IF (LINUX_FEDORA)
  SET(ROUTER_INSTALL_RUNTIMEDIR_RPM "/run/mysqlrouter")
ELSE()
  SET(ROUTER_INSTALL_RUNTIMEDIR_RPM "/var/run/mysqlrouter")
ENDIF()

SET(ROUTER_INSTALL_BINDIR_RPM     "bin")
IF(CMAKE_SYSTEM_PROCESSOR IN_LIST KNOWN_64BIT_ARCHITECTURES)
  SET(ROUTER_INSTALL_LIBDIR_RPM     "lib64/mysqlrouter/private")
  SET(ROUTER_INSTALL_PLUGINDIR_RPM  "lib64/mysqlrouter")
ELSE()
  SET(ROUTER_INSTALL_LIBDIR_RPM     "lib/mysqlrouter/private")
  SET(ROUTER_INSTALL_PLUGINDIR_RPM  "lib/mysqlrouter")
ENDIF()

#
# DEB layout
#
SET(ROUTER_INSTALL_CONFIGDIR_DEB  "/etc/mysqlrouter")
SET(ROUTER_INSTALL_DATADIR_DEB    "/var/lib/mysqlrouter")
SET(ROUTER_INSTALL_LOGDIR_DEB     "/var/log/mysqlrouter")
SET(ROUTER_INSTALL_RUNTIMEDIR_DEB "/var/run/mysqlrouter")

SET(ROUTER_INSTALL_BINDIR_DEB     "bin")
SET(ROUTER_INSTALL_LIBDIR_DEB     "lib/mysqlrouter/private")
SET(ROUTER_INSTALL_PLUGINDIR_DEB  "lib/mysqlrouter/plugin")

#
# SVR4 layout
#
FOREACH(var
    CONFIG
    DATA
    LOG
    RUNTIME
    BIN
    LIB
    PLUGIN
    )
  SET(ROUTER_INSTALL_${var}DIR_SVR4 ${ROUTER_INSTALL_${var}DIR_STANDALONE})
ENDFOREACH()

# Set ROUTER_INSTALL_FOODIR variables for chosen layout for example,
# ROUTER_INSTALL_CONFIGDIR will be defined as
# ${ROUTER_INSTALL_CONFIGDIR_STANDALONE} by default if STANDALONE
# layout is chosen.
FOREACH(directory
    CONFIG
    DATA
    LOG
    RUNTIME
    BIN
    LIB
    PLUGIN
    )
  SET(ROUTER_INSTALL_${directory}DIR
    ${ROUTER_INSTALL_${directory}DIR_${ROUTER_INSTALL_LAYOUT}}
    CACHE STRING "Router ${directory} installation directory")
  MARK_AS_ADVANCED(ROUTER_INSTALL_${directory}DIR)
ENDFOREACH()
