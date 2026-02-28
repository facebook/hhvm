/* Copyright (c) 2005, 2018, Oracle and/or its affiliates. All rights reserved.

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

#ifndef TABLE_MAPPING_H
#define TABLE_MAPPING_H

#include <sys/types.h>

#include "map_helpers.h"
#include "my_alloc.h"
#include "my_inttypes.h"

/* Forward declarations */
#ifdef MYSQL_SERVER
struct TABLE;

typedef TABLE Mapped_table;
#else
class Table_map_log_event;

typedef Table_map_log_event Mapped_table;
#endif

/**
  Maps table id's (integers) to table pointers.

  In mysqlbinlog, "table pointer" means Table_map_log_event*.

  In the server, "table pointer" means TABLE*.
*/
class table_mapping {
 private:
  MEM_ROOT m_mem_root;

 public:
  enum enum_error {
    ERR_NO_ERROR = 0,
    ERR_LIMIT_EXCEEDED,
    ERR_MEMORY_ALLOCATION
  };

  table_mapping();
  ~table_mapping();

  Mapped_table *get_table(ulonglong table_id);

  int set_table(ulonglong table_id, Mapped_table *table);
  int remove_table(ulonglong table_id);
  void clear_tables();
  ulong count() const { return static_cast<ulong>(m_table_ids.size()); }

 private:
  struct entry {
    ulonglong table_id;
    union {
      Mapped_table *table;
      entry *next;
    };
  };

  int expand();

  /*
    Head of the list of free entries; "free" in the sense that it's an
    allocated entry free for use, NOT in the sense that it's freed
    memory.
  */
  entry *m_free;

  /*
    Map from table ids (numbers) to Mapped_table objects.

    No destructor for entries passed here, as the entries are allocated in a
    MEM_ROOT (freed as a whole in the destructor), they cannot be freed one by
    one.
  */
  malloc_unordered_map<ulonglong, entry *> m_table_ids;
};

#endif
