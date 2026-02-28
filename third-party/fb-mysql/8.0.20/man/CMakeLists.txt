# Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.
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

# Copy man pages
SET(MAN1
  comp_err.1
  ibd2sdi.1
  innochecksum.1
  lz4_decompress.1
  my_print_defaults.1
  myisam_ftdump.1
  myisamchk.1
  myisamlog.1
  myisampack.1
  mysql.1
  mysql.server.1
  mysql_config.1
  mysql_config_editor.1
  mysql_secure_installation.1
  mysql_ssl_rsa_setup.1
  mysql_tzinfo_to_sql.1
  mysql_upgrade.1
  mysqladmin.1
  mysqlbinlog.1
  mysqlcheck.1
  mysqld_multi.1
  mysqld_safe.1
  mysqldump.1
  mysqldumpslow.1
  mysqlimport.1
  mysqlman.1
  mysqlpump.1
  mysqlshow.1
  mysqlslap.1
  perror.1
  zlib_decompress.1
  )
SET(MAN1_NDB
  ndb-common-options.1
  ndb_blob_tool.1
  ndb_config.1
  ndb_cpcd.1
  ndb_delete_all.1
  ndb_desc.1
  ndb_drop_index.1
  ndb_drop_table.1
  ndb_error_reporter.1
  ndb_import.1
  ndb_index_stat.1
  ndb_mgm.1
  ndb_move_data.1
  ndb_perror.1
  ndb_print_backup_file.1
  ndb_print_file.1
  ndb_print_frag_file.1
  ndb_print_schema_file.1
  ndb_print_sys_file.1
  ndb_redo_log_reader.1
  ndb_restore.1
  ndb_select_all.1
  ndb_select_count.1
  ndb_setup.py.1
  ndb_show_tables.1
  ndb_size.pl.1
  ndb_top.1
  ndb_waiter.1
  ndbinfo_select_all.1
)
SET(MAN1_ROUTER
  mysqlrouter.1
  mysqlrouter_passwd.1
  mysqlrouter_plugin_info.1
)
SET(MAN8
  mysqld.8
  )
SET(MAN8_NDB
  ndb_mgmd.8
  ndbd.8
  ndbmtd.8
)

INSTALL(FILES ${MAN1} DESTINATION ${INSTALL_MANDIR}/man1 COMPONENT ManPages)
INSTALL(FILES ${MAN8} DESTINATION ${INSTALL_MANDIR}/man8 COMPONENT ManPages)

IF(WITH_ROUTER)
  INSTALL(FILES ${MAN1_ROUTER}
    DESTINATION ${INSTALL_MANDIR}/man1 COMPONENT ManPages
    )
ENDIF()

IF(WITH_NDBCLUSTER)
  INSTALL(FILES ${MAN1_NDB}
    DESTINATION ${INSTALL_MANDIR}/man1 COMPONENT ManPages)
  INSTALL(FILES ${MAN8_NDB}
    DESTINATION ${INSTALL_MANDIR}/man8 COMPONENT ManPages)
ENDIF()
