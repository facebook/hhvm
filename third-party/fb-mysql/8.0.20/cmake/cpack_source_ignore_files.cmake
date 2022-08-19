# Copyright (c) 2009, 2014, Oracle and/or its affiliates. All rights reserved.
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

SET(CPACK_SOURCE_IGNORE_FILES
CMakeCache\\\\.txt
cmake_dist\\\\.cmake
CPackSourceConfig\\\\.cmake
CPackConfig.cmake
/cmake_install\\\\.cmake
/CTestTestfile\\\\.cmake
/CMakeFiles/
/version_resources/
/_CPack_Packages/
$\\\\.gz
$\\\\.zip
$\\\\.bz2
/CMakeFiles/
/version_resources/
/_CPack_Packages/
scripts/make_binary_distribution$
scripts/mysql_config$
scripts/mysql_secure_installation$
scripts/mysql_server_config$
scripts/mysqld_multi$
scripts/mysqld_safe$
scripts/mysqldumpslow$
Makefile$
include/config\\\\.h$
include/my_config\\\\.h$
/autom4te\\\\.cache/
errmsg\\\\.sys$
#
)
