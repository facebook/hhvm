# Copyright (c) 2017, 2019 Oracle and/or its affiliates. All rights reserved.
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

SET (DEB_NDB_CONTROL_TEST_DEPS "mysql-${DEB_PRODUCTNAME}-data-node (= \${binary:Version}), mysql-${DEB_PRODUCTNAME}-management-server (= \${binary:Version})")

SET (DEB_NDB_CONTROL_DBGSYM
"
Package: mysql-${DEB_PRODUCTNAME}-data-node-dbgsym
Architecture: any
Section: debug
Depends: mysql-${DEB_PRODUCTNAME}-data-node (=\${binary:Version}), \${misc:Depends}
Description: Debugging symbols for data node

Package: mysql-${DEB_PRODUCTNAME}-management-server-dbgsym
Architecture: any
Section: debug
Depends: mysql-${DEB_PRODUCTNAME}-management-server (=\${binary:Version}), \${misc:Depends}
Description: Debugging symbols for management server

Package: mysql-${DEB_PRODUCTNAME}-memcached-dbgsym
Architecture: any
Section: debug
Depends: mysql-${DEB_PRODUCTNAME}-memcached (=\${binary:Version}), \${misc:Depends}
Description: Debugging symbols for memcached

Package: mysql-${DEB_PRODUCTNAME}-nodejs-dbgsym
Architecture: any
Section: debug
Depends: mysql-${DEB_PRODUCTNAME}-nodejs (=\${binary:Version}), \${misc:Depends}
Description: Debugging symbols for nodejs

Package: ndbclient-dbgsym
Architecture: any
Section: debug
Depends: ndbclient (=\${binary:Version}), \${misc:Depends}
Description: Debugging symbols for ndb client library
")
SET (DEB_NDB_RULES_STRIP
"
	dh_strip -pmysql-${DEB_PRODUCTNAME}-data-node --dbg-package=mysql-${DEB_PRODUCTNAME}-data-node-dbgsym
	dh_strip -pmysql-${DEB_PRODUCTNAME}-management-server --dbg-package=mysql-${DEB_PRODUCTNAME}-management-server-dbgsym
	dh_strip -pmysql-${DEB_PRODUCTNAME}-memcached --dbg-package=mysql-${DEB_PRODUCTNAME}-memcached-dbgsym
	dh_strip -pmysql-${DEB_PRODUCTNAME}-nodejs --dbg-package=mysql-${DEB_PRODUCTNAME}-nodejs-dbgsym
	dh_strip -pndbclient --dbg-package=ndbclient-dbgsym
")
SET(DEB_NDB_CONTROL_EXTRAS
"
Package: mysql-${DEB_PRODUCTNAME}-management-server
Architecture: any
Depends: \${shlibs:Depends}, \${misc:Depends}
Description: Management server
 This package contains the MySQL Cluster Management Server Daemon,
 which reads the cluster configuration file and distributes this
 information to all nodes in the cluster

Package: mysql-${DEB_PRODUCTNAME}-data-node
Architecture: any
Depends: \${shlibs:Depends}, \${misc:Depends},
 libclass-methodmaker-perl
Description: Data node
 This package contains MySQL Cluster Data Node Daemon, it's the process
 that is used to handle all the data in tables using the NDB Cluster
 storage engine. It comes in two variants: ndbd and ndbmtd, the former
 is single threaded while the latter is multi-threaded.

Package: mysql-${DEB_PRODUCTNAME}-auto-installer
Architecture: any
Depends: \${shlibs:Depends}, \${misc:Depends},
 python-paramiko
Description: Data node
 This package contains MySQL Cluster Data Node Daemon, it's the process
 that is used to handle all the data in tables using the NDB Cluster
 storage engine. It comes in two variants: ndbd and ndbmtd, the former
 is single threaded while the latter is multi-threaded.

Package: ndbclient
Architecture: any
Depends: \${shlibs:Depends}, \${misc:Depends}
Description: Ndb client
 This package contains the shared libraries for MySQL MySQL NDB storage
 engine client applications.

Package: ndbclient-dev
Architecture: any
Depends: \${shlibs:Depends}, \${misc:Depends}, ndbclient
Description: ndbclient dev package
 This package contains the development header files and libraries
 necessary to develop client applications for MySQL NDB storage engine.

Package: mysql-${DEB_PRODUCTNAME}-java
Architecture: any
Depends: \${shlibs:Depends}, \${misc:Depends}
Description: Java connector
 This package contains MySQL Cluster Connector for Java, which includes
 ClusterJ and ClusterJPA, a plugin for use with OpenJPA.
 .
 ClusterJ is a high level database API that is similar in style and
 concept to object-relational mapping persistence frameworks such as
 Hibernate and JPA.
 .
 ClusterJPA is an OpenJPA implementation for MySQL Cluster that
 attempts to offer the best possible performance by leveraging the
 strengths of both ClusterJ and JDBC

Package: mysql-${DEB_PRODUCTNAME}-memcached
Architecture: any
Depends: \${shlibs:Depends}, \${misc:Depends}, mysql-${DEB_PRODUCTNAME}-server
Description: memcached
 This package contains the standard memcached server and a loadable
 storage engine for memcached using the Memcache API for MySQL Cluster
 to provide a persistent MySQL Cluster data store.

Package: mysql-${DEB_PRODUCTNAME}-nodejs
Architecture: any
Depends: \${shlibs:Depends}, \${misc:Depends}
Description: nodejs
 This package contains MySQL NoSQL Connector for JavaScript, a set of
 Node.js adapters for MySQL Cluster and MySQL Server, which make it
 possible to write JavaScript applications for Node.js using MySQL
 data.
")

  SET (DEB_NDB_CLIENT_EXTRA
"
/usr/bin/ndb_blob_tool
/usr/bin/ndb_config
/usr/bin/ndb_delete_all
/usr/bin/ndb_desc
/usr/bin/ndb_drop_index
/usr/bin/ndb_drop_table
/usr/bin/ndb_error_reporter
/usr/bin/ndb_index_stat
/usr/bin/ndb_import
/usr/bin/ndb_mgm
/usr/bin/ndb_move_data
/usr/bin/ndb_perror
/usr/bin/ndb_print_backup_file
/usr/bin/ndb_print_file
/usr/bin/ndb_print_frag_file
/usr/bin/ndb_print_schema_file
/usr/bin/ndb_print_sys_file
/usr/bin/ndb_redo_log_reader
/usr/bin/ndb_restore
/usr/bin/ndb_select_all
/usr/bin/ndb_select_count
/usr/bin/ndb_setup.py
/usr/bin/ndb_show_tables
/usr/bin/ndb_size.pl
/usr/bin/ndb_top
/usr/bin/ndb_waiter
/usr/bin/ndbinfo_select_all

/usr/share/man/man1/ndb-common-options.1*
/usr/share/man/man1/ndb_blob_tool.1*
/usr/share/man/man1/ndb_config.1*
/usr/share/man/man1/ndb_cpcd.1*
/usr/share/man/man1/ndb_delete_all.1*
/usr/share/man/man1/ndb_desc.1*
/usr/share/man/man1/ndb_drop_index.1*
/usr/share/man/man1/ndb_drop_table.1*
/usr/share/man/man1/ndb_error_reporter.1*
/usr/share/man/man1/ndb_import.1*
/usr/share/man/man1/ndb_index_stat.1*
/usr/share/man/man1/ndb_mgm.1*
/usr/share/man/man1/ndb_move_data.1*
/usr/share/man/man1/ndb_perror.1*
/usr/share/man/man1/ndb_print_backup_file.1*
/usr/share/man/man1/ndb_print_file.1*
/usr/share/man/man1/ndb_print_frag_file.1*
/usr/share/man/man1/ndb_print_schema_file.1*
/usr/share/man/man1/ndb_print_sys_file.1*
/usr/share/man/man1/ndb_restore.1*
/usr/share/man/man1/ndb_select_all.1*
/usr/share/man/man1/ndb_select_count.1*
/usr/share/man/man1/ndb_setup.py.1*
/usr/share/man/man1/ndb_show_tables.1*
/usr/share/man/man1/ndb_size.pl.1*
/usr/share/man/man1/ndb_top.1*
/usr/share/man/man1/ndb_waiter.1*
/usr/share/man/man1/ndb_redo_log_reader.1*
/usr/share/man/man1/ndbinfo_select_all.1*
")
