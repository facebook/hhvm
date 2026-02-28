/* Copyright (c) 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__TABLESPACE_ID_OWNER_VISITOR_INCLUDED
#define DD__TABLESPACE_ID_OWNER_VISITOR_INCLUDED

#include "sql/dd/types/index.h"            // dd::Index
#include "sql/dd/types/partition.h"        // dd::Partition
#include "sql/dd/types/partition_index.h"  // dd::Partition_index
#include "sql/dd/types/table.h"            // dd::Table

namespace dd {
/**
   Visitor function which invokes its visitor argument on any subobject
   which holds a tablespace, that is, the table itself, its m_indexes,
   its m_partitions, and m_indexes (Partition_index) of each
   partition. Visitation is terminated when the vistor closure
   returns true.

   @param t table to visit.

   @param visitor generic closure which can be applied to all
   tablespace holding objects.

   @return value of last visitor invocation.
*/
template <class VISITOR_TYPE>
bool visit_tablespace_id_owners(const Table &t, VISITOR_TYPE &&visitor) {
  if (visitor(t)) {
    return true;
  }
  for (const Index *i : t.indexes()) {
    if (visitor(*i)) {
      return true;
    }
  }
  for (const Partition *p : t.partitions()) {
    if (visitor(*p)) {
      return true;
    }
    // Visit indexes for top-level partition
    for (const Partition_index *pi : p->indexes()) {
      if (visitor(*pi)) {
        return true;
      }
    }

    // Visit subpartitions, if any
    for (const Partition *sp : p->subpartitions()) {
      if (visitor(*sp)) {
        return true;
      }
      // Visit indexes for subpartition
      for (const Partition_index *spi : sp->indexes()) {
        if (visitor(*spi)) {
          return true;
        }
      }
    }
  }
  return false;
}
}  // namespace dd

#endif
