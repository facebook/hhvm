/* Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.

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
  @addtogroup Replication
  @{

  @file load_data_events.h

  @brief LOAD DATA INFILE is not written to the binary log like other
  statements. It is written as one or more events in a packed format,
  not as a cleartext statement in the binary log. The events indicate
  what options are present in the statement and how to process the data file.
*/

#ifndef LOAD_DATA_EVENTS_INCLUDED
#define LOAD_DATA_EVENTS_INCLUDED

#include <sys/types.h>
#include "statement_events.h"
#include "table_id.h"

/*
  These are flags and structs to handle all the LOAD DATA INFILE options (LINES
  TERMINATED etc).
  DUMPFILE_FLAG is probably not used (DUMPFILE is a clause of SELECT,
  not of LOAD DATA).
*/
#define DUMPFILE_FLAG 0x1
#define OPT_ENCLOSED_FLAG 0x2
#define REPLACE_FLAG 0x4
#define IGNORE_FLAG 0x8

#define FIELD_TERM_EMPTY 0x1
#define ENCLOSED_EMPTY 0x2
#define LINE_TERM_EMPTY 0x4
#define LINE_START_EMPTY 0x8
#define ESCAPED_EMPTY 0x10

namespace binary_log {
/**
  Elements of this enum describe how LOAD DATA handles duplicates.
*/
enum enum_load_dup_handling {
  LOAD_DUP_ERROR = 0,
  LOAD_DUP_IGNORE,
  LOAD_DUP_REPLACE
};

/**
  @class Execute_load_query_event

  Event responsible for LOAD DATA execution, it similar to Query_event
  but before executing the query it substitutes original filename in LOAD DATA
  query with name of temporary file.

  The first 13 bytes of the Post-Header for this event are the same as for
  Query_event, as is the initial status variable block in the Body.

  @section Execute_load_query_event_binary_format Binary Format

  The additional members of the events are the following:

   <table>
   <caption>Body for Execute_load_query_event</caption>

   <tr>
     <th>Name</th>
     <th>Format</th>
     <th>Description</th>
   </tr>

   <tr>
     <td>file_id</td>
     <td>4 byte unsigned integer</td>
     <td>ID of the temporary file to load</td>
   </tr>

   <tr>
     <td>fn_pos_start</td>
     <td>4 byte unsigned integer</td>
     <td>The start position within the statement for filename substitution</td>
   </tr>
   <tr>

     <td>fn_pos_end</td>
     <td>4 byte unsigned integer</td>
     <td>The end position within the statement for filename substitution</td>
   </tr>

   <tr>
     <td>dup_handling</td>
     <td>enum_load_dup_handling</td>
     <td>Represents information on how to handle duplicates:
          LOAD_DUP_ERROR= 0, LOAD_DUP_IGNORE= 1, LOAD_DUP_REPLACE= 2</td>
   </tr>
   </table>
*/
class Execute_load_query_event : public virtual Query_event {
 public:
  enum Execute_load_query_event_offset {
    /** ELQ = "Execute Load Query" */
    ELQ_FILE_ID_OFFSET = QUERY_HEADER_LEN,
    ELQ_FN_POS_START_OFFSET = ELQ_FILE_ID_OFFSET + 4,
    ELQ_FN_POS_END_OFFSET = ELQ_FILE_ID_OFFSET + 8,
    ELQ_DUP_HANDLING_OFFSET = ELQ_FILE_ID_OFFSET + 12
  };

  int32_t file_id;       /** file_id of temporary file */
  uint32_t fn_pos_start; /** pointer to the part of the query that should
                            be substituted */
  uint32_t fn_pos_end;   /** pointer to the end of this part of query */

  /**
    We have to store type of duplicate handling explicitly, because
    for LOAD DATA it also depends on LOCAL option. And this part
    of query will be rewritten during replication so this information
    may be lost...
  */
  enum_load_dup_handling dup_handling;

  Execute_load_query_event(uint32_t file_id_arg, uint32_t fn_pos_start,
                           uint32_t fn_pos_end, enum_load_dup_handling dup);

  /**
    The constructor receives a buffer and instantiates a
    Execute_load_query_event filled in with the data from the buffer.

    <pre>
    The fixed event data part buffer layout is as follows:
    +---------------------------------------------------------------------+
    | thread_id | query_exec_time | db_len | error_code | status_vars_len |
    +---------------------------------------------------------------------+
    +----------------------------------------------------+
    | file_id | fn_pos_start | fn_pos_end | dup_handling |
    +----------------------------------------------------+
    </pre>

    <pre>
    The fixed event data part buffer layout is as follows:
    +------------------------------------------------------------------+
    | Zero or more status variables | db |  LOAD DATA INFILE statement |
    +------------------------------------------------------------------+
    </pre>

    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  Execute_load_query_event(const char *buf,
                           const Format_description_event *fde);

  ~Execute_load_query_event() {}
};

/**
  @class Delete_file_event

  DELETE_FILE_EVENT occurs when the LOAD DATA failed on the master.
  This event notifies the slave not to do the load and to delete
  the temporary file.

  @section Delete_file_event_binary_format Binary Format

  The variable data part is empty. The post header contains the following:

  <table>
  <caption>Post header for Delete_file_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>file_id</td>
    <td>32 bit integer</td>
    <td>The ID of the file to be deleted</td>
  </tr>
  </table>
*/
class Delete_file_event : public Binary_log_event {
 protected:
  // Required by Delete_file_log_event(THD* ..)
  Delete_file_event(uint32_t file_id_arg, const char *db_arg)
      : Binary_log_event(DELETE_FILE_EVENT), file_id(file_id_arg), db(db_arg) {}

 public:
  enum Delete_file_offset {
    /** DF = "Delete File" */
    DF_FILE_ID_OFFSET = 0
  };

  uint32_t file_id;
  const char *db; /** see comment in Append_block_event */

  /**
    The buffer layout for fixed data part is as follows:
    <pre>
    +---------+
    | file_id |
    +---------+
    </pre>

    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
 */
  Delete_file_event(const char *buf, const Format_description_event *fde);

  ~Delete_file_event() {}

#ifndef HAVE_MYSYS
  // TODO(WL#7684): Implement the method print_event_info and print_long_info
  // for
  //            all the events supported  in  MySQL Binlog
  void print_event_info(std::ostream &) {}
  void print_long_info(std::ostream &) {}
#endif
};

/**
  @class Append_block_event

  This event is created to contain the file data. One LOAD_DATA_INFILE
  can have 0 or more instances of this event written to the binary log
  depending on the size of the file. If the file to be loaded is greater
  than the threshold value, which is roughly 2^17 bytes, the file is
  divided into blocks of size equal to the threshold, and each block
  is sent across as a separate event.

  @section Append_block_event_binary_format Binary Format

  The post header contains the following:

  <table>
  <caption>Post header for Append_block_event</caption>

  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>

  <tr>
    <td>file_id</td>
    <td>32 bit integer</td>
    <td>The ID of the file to append the block to</td>
  </tr>
  </table>

  The body of the event contains the raw data to load. The raw data
  size is the event size minus the size of all the fixed event parts.
*/
class Append_block_event : public Binary_log_event {
 protected:
  /**
    This constructor is used by the MySQL server.
  */
  Append_block_event(const char *db_arg, unsigned char *block_arg,
                     unsigned int block_len_arg, uint32_t file_id_arg)
      : Binary_log_event(APPEND_BLOCK_EVENT),
        block(block_arg),
        block_len(block_len_arg),
        file_id(file_id_arg),
        db(db_arg) {}

  Append_block_event(Log_event_type type_arg = APPEND_BLOCK_EVENT)
      : Binary_log_event(type_arg) {}

 public:
  enum Append_block_offset {
    /** AB = "Append Block" */
    AB_FILE_ID_OFFSET = 0,
    AB_DATA_OFFSET = APPEND_BLOCK_HEADER_LEN
  };

  unsigned char *block;
  unsigned int block_len;
  uint32_t file_id;
  /**
    'db' is filled when the event is created in mysql_load() (the
    event needs to have a 'db' member to be well filtered by
    binlog-*-db rules). 'db' is not written to the binlog (it's not
    used by Append_block_log_event::write()), so it can't be read in
    the Append_block_event(const char* buf, int event_len)
    constructor.  In other words, 'db' is used only for filtering by
    binlog-*-db rules.  Create_file_event is different: it's 'db'
    (which is inherited from Load_event) is written to the binlog
    and can be re-read.
  */
  const char *db;

  /**
    Appends the buffered data, received as a parameter, to the file being loaded
    via LOAD_DATA_FILE.

    The buffer layout for fixed data part is as follows:
    <pre>
    +---------+
    | file_id |
    +---------+
    </pre>

    The buffer layout for variable data part is as follows:
    <pre>
    +-------------------+
    | block | block_len |
    +-------------------+
    </pre>

    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  Append_block_event(const char *buf, const Format_description_event *fde);
  ~Append_block_event() {}

#ifndef HAVE_MYSYS
  // TODO(WL#7684): Implement the method print_event_info and print_long_info
  // for
  //            all the events supported  in  MySQL Binlog
  void print_event_info(std::ostream &) {}
  void print_long_info(std::ostream &) {}
#endif
};

/**
  @class Begin_load_query_event

  Event for the first block of file to be loaded, its only difference from
  Append_block event is that this event creates or truncates existing file
  before writing data.

  @section Begin_load_query_event_binary_format Binary Format

  The Post-Header and Body for this event type are empty; it only has
  the Common-Header.
*/
class Begin_load_query_event : public virtual Append_block_event {
 protected:
  Begin_load_query_event() : Append_block_event(BEGIN_LOAD_QUERY_EVENT) {}

 public:
  /**
    The buffer layout for fixed data part is as follows:
    <pre>
    +---------+
    | file_id |
    +---------+
    </pre>

    The buffer layout for variable data part is as follows:
    <pre>
    +-------------------+
    | block | block_len |
    +-------------------+
    </pre>

    @param buf  Contains the serialized event.
    @param fde  An FDE event (see Rotate_event constructor for more info).
  */
  Begin_load_query_event(const char *buf, const Format_description_event *fde);

  ~Begin_load_query_event() {}

#ifndef HAVE_MYSYS
  // TODO(WL#7684): Implement the method print_event_info and print_long_info
  // for
  //            all the events supported  in  MySQL Binlog
  void print_event_info(std::ostream &) {}
  void print_long_info(std::ostream &) {}
#endif
};
}  // end namespace binary_log
/**
  @} (end of group Replication)
*/
#endif /* LOAD_DATA_EVENTS_INCLUDED */
