# Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.
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

SET(DEB_CONTROL_ROUTER
"
Package: mysql-router-${DEB_PRODUCTNAME}
Architecture: any
Depends: \${shlibs:Depends}, \${misc:Depends}
Breaks: mysql-router-${DEB_PRODUCTNAME}
Replaces: mysql-router-${DEB_PRODUCTNAME}
Conflicts: mysql-router-${DEB_NOTPRODUCTNAME}, mysql-router (<< 8.0.3)
Description: MySQL Router
 The MySQL(TM) Router software delivers a fast, multi-threaded way of
 routing connections from MySQL Clients to MySQL Servers. MySQL is a
 trademark of Oracle.


Package: mysql-router
Architecture: any
Depends: mysql-router-${DEB_PRODUCTNAME}
Description: MySQL Router Metapackage
 The MySQL(TM) Router software delivers a fast, multi-threaded way of
 routing connections from MySQL Clients to MySQL Servers. MySQL is a
 trademark of Oracle. This is the shared router metapackage, used for
 dependency handling.

")
