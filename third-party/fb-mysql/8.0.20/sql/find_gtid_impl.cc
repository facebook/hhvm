/*
   Copyright (c) 2000, 2018, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  Finds the position of given gtid by scanning through the given file
  and comparing the given gtid with gtid in each Gtid_log_event.

  This is a templated function needed by rpl_master and mysqlbinlog.

  @param log_name File to look in.
  @param gtid     GTID to search for.
  @param sid_map  Sid_map used when parsing Gtid_log_event.

  @retval 0 if GTID is not found.
  @retval starting offset of the corresponding Gtid_log_event in the file,
          if GTID is found.
*/
template <class BINLOG_FILE_READER>
my_off_t find_gtid_pos_in_log(const char *log_name, const Gtid &gtid,
                              Sid_map *sid_map) {
  DBUG_ENTER("find_gtid_pos_in_log");
  DBUG_PRINT("info", ("Scanning log: %s", log_name));

  Log_event *ev = NULL;
  my_off_t pos = 0;

  BINLOG_FILE_READER binlog_file_reader(false);

  pos = BIN_LOG_HEADER_SIZE;
  if (binlog_file_reader.open(log_name, pos)) {
    goto err;
  }

  while ((ev = binlog_file_reader.read_event_object()) != nullptr) {
    if (ev->get_type_code() == binary_log::GTID_LOG_EVENT) {
      Gtid_log_event *gtid_ev = (Gtid_log_event *)ev;
      if (gtid_ev->get_sidno(sid_map) == gtid.sidno &&
          gtid_ev->get_gno() == gtid.gno)
        break;
    }
    delete ev;
    pos = binlog_file_reader.position();
  }

err:
  /* Event was not found in above while loop.  */
  if (ev == NULL) pos = 0;
  delete ev;
  DBUG_RETURN(pos);
}
