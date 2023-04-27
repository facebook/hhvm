/*
   Copyright (c) 2010, 2020, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef ABSTRACT_QUERY_PLAN_H_INCLUDED
#define ABSTRACT_QUERY_PLAN_H_INCLUDED

#include <sys/types.h>

#include "my_dbug.h"
#include "sql/item_cmpfunc.h" // Item_equal_iterator

class Item;
class Item_field;
class JOIN;
class KEY_PART_INFO;
class QEP_TAB;
struct TABLE;

/**
  Abstract query plan (AQP) is an interface for examining certain aspects of 
  query plans without accessing mysqld internal classes (JOIN_TAB, QEP_TAB, 
  etc.) directly.

  AQP maps join execution plans, as represented by mysqld internals, to a set 
  of facade classes. Non-join operations such as sorting and aggregation is
  currently *not* modelled in the AQP.

  The AQP models an n-way join as a sequence of the n table access operations
  that the MySQL server would execute as part of its nested loop join 
  execution. (Each such table access operation is a scan of a table or index,
  or an index lookup.) For each lookup operation, it is possible to examine 
  the expression that represents each field in the key.

  A storage enging will typically use the AQP for finding sections of a join
  execution plan that may be executed in the engine rather than in mysqld. By 
  using the AQP rather than the mysqld internals directly, the coupling between
  the engine and mysqld is reduced.

  The AQP also provides functions which allows the storage engine
  to change the query execution plan for the part of the join which
  it will handle. Thus be aware that although the QEP_TAB*'s are const
  they may be modified.
*/
namespace AQP
{
  class Table_access;

  /**
    This class represents a query plan for an n-way join, in the form a 
    sequence of n table access operations that will execute as a nested loop 
    join.
  */
  class Join_plan
  {
    friend class Equal_set_iterator;
    friend class Table_access;
  public:

    explicit Join_plan(const JOIN* join);

    ~Join_plan();

    Table_access* get_table_access(uint access_no) const;

    uint get_access_count() const;

  private:
    /** 
      Array of the QEP_TABs that are the internal representation of table
      access operations.
    */
    const QEP_TAB* const m_qep_tabs;

    /** Number of table access operations. */
    uint m_access_count;
    Table_access* m_table_accesses;

    const QEP_TAB* get_qep_tab(uint qep_tab_no) const;

    // No copying.
    Join_plan(const Join_plan&);
    Join_plan& operator=(const Join_plan&);
  }; 
  // class Join_plan


  /**
    This class is an iterator for iterating over sets of fields (columns) that
    should have the same value. For example, if the query is
    SELECT * FROM T1, T2, T3 WHERE T1.b = T2.a AND T2.a = T3.a
    then there would be such a set of {T1.b, T2.a, T3.a}.
  */
  class Equal_set_iterator
  {
  public:
    explicit Equal_set_iterator(Item_equal& item_equal)
    : m_iterator(item_equal) {}

    const Item_field* next()
    { return m_iterator++; }

  private:
    /**
      This class is implemented in terms of this mysqld internal class.
     */
    Item_equal_iterator m_iterator;

    // No copying.
    Equal_set_iterator(const Equal_set_iterator&);
    Equal_set_iterator& operator=(const Equal_set_iterator&);
  }; 
  // class Equal_set_iterator

  /** The type of a table access operation. */
  enum enum_access_type
  {
    /** For default initialization.*/
    AT_VOID,
    /** Value has already been fetched / determined by optimizer.*/
    AT_FIXED,
    /** Do a lookup of a single primary key.*/
    AT_PRIMARY_KEY,
    /** Do a lookup of a single unique index key.*/
    AT_UNIQUE_KEY,
    /** Scan an ordered index with a single upper and lower bound pair.*/
    AT_ORDERED_INDEX_SCAN,
    /** Do a multi range read for a set of primary keys.*/
    AT_MULTI_PRIMARY_KEY,
    /** Do a multi range read for a set of unique index keys.*/
    AT_MULTI_UNIQUE_KEY,
    /** 
      Do a multi range read for a mix of ranges (for which there is an
      ordered index), and either primary keys or unique index keys.
    */
    AT_MULTI_MIXED,
    /** Scan a table. (No index is assumed to be used.) */
    AT_TABLE_SCAN,
    /** Access method will not be chosen before the execution phase.*/
    AT_UNDECIDED,
    /**
      The access method has properties that prevents it from being pushed to a 
      storage engine.
     */
    AT_OTHER
  };

  /**
    This class represents an access operation on a table, such as a table
    scan, or a scan or lookup via an index. A Table_access object is always
    owned by a Join_plan object, such that the life time of the Table_access 
    object ends when the life time of the owning Join_plan object ends.
   */
  class Table_access
  {
    friend class Join_plan;
    friend inline bool equal(const Table_access*, const Table_access*);
  public:

    const Join_plan* get_join_plan() const;

    enum_access_type get_access_type() const;

    const char* get_other_access_reason() const;

    uint get_no_of_key_fields() const;

    const Item* get_key_field(uint field_no) const;

    const KEY_PART_INFO* get_key_part_info(uint field_no) const;

    uint get_access_no() const;

    int get_index_no() const;

    TABLE* get_table() const;

    Item_equal* get_item_equal(const Item_field* field_item) const;

    void dbug_print() const;

    bool uses_join_cache() const;
    
    bool filesort_before_join() const;

    Item* get_condition() const;
    void set_condition(Item* cond);

    uint get_first_inner() const;
    uint get_last_inner() const;
    int get_first_upper() const;

    int get_first_sj_inner() const;
    int get_last_sj_inner() const;

    // Is member of a firstMatch sj_nest?
    bool is_sj_firstmatch() const;
    // Get the upper-table we skip out to upon a firstMatch
    int get_firstmatch_return() const;

    /**
      Getter and setters for an opaque object for each table.
      Used by the handler's to persist 'pushability-flags' to avoid
      overhead by recalculating it for each ::engine_push()
    */
    uint get_table_properties() const;
    void set_table_properties(uint);

  private:

    /** Backref. to the Join_plan which this Table_access is part of */
    const Join_plan* m_join_plan;

    /** This operation corresponds to m_root_tab[m_tab_no].*/
    uint m_tab_no;

    /** The type of this operation.*/
    mutable enum_access_type m_access_type;

    /** 
      The reason for getting m_access_type==AT_OTHER. Used for explain extended.
    */
    mutable const char* m_other_access_reason;

    /** The index to use for this operation (if applicable )*/
    mutable int m_index_no;

    /** May store an opaque property / flag */
    uint m_properties;

    explicit Table_access();

    const QEP_TAB* get_qep_tab() const;

    void compute_type_and_index() const;

    /** No copying*/
    Table_access(const Table_access&);
    Table_access& operator=(const Table_access&);
  }; 
  // class Table_access

  /**
    Get the n'th table access operation.
    @param access_no The index of the table access operation to fetch.
    @return The access_no'th table access operation.
  */
  inline Table_access* Join_plan::get_table_access(uint access_no) const
  {
    DBUG_ASSERT(access_no < m_access_count);
    return m_table_accesses + access_no;
  }

  /**
     @return The number of table access operations in the nested loop join.
  */
  inline uint Join_plan::get_access_count() const
  { 
    return m_access_count;
  }

  /** Get the Join_plan that this Table_access belongs to.*/
  inline const Join_plan* Table_access::get_join_plan() const
  {
    return m_join_plan;
  }

  /** Get the type of this operation.*/
  inline enum_access_type Table_access::get_access_type() const
  {
    if (m_access_type == AT_VOID)
      compute_type_and_index();
    return m_access_type;
  }

  /** 
    Get a description of the reason for getting access_type==AT_OTHER. To be 
    used for informational messages.
    @return A string that should be assumed to have the same life time as the
    Table_access object.
  */
  inline const char* Table_access::get_other_access_reason() const
  {
    if (m_access_type == AT_VOID)
      compute_type_and_index();
    return m_other_access_reason;
  }

  /**
    @return The number of the index to use for this access operation (
    or -1 for non-index operations).
  */
  inline int Table_access::get_index_no() const
  {
    if (m_access_type == AT_VOID)
      compute_type_and_index();
	
    return m_index_no;
  }

  /** 
    Get the number of this Table_access within the enclosing Join_plan. 
    (This number will be in the range 0 to Join_plan::get_access_count() - 1.)
  */
  inline uint Table_access::get_access_no() const
  { 
    return m_tab_no;
  }

  inline uint Table_access::get_table_properties() const
  {
    return m_properties;
  }

  inline void Table_access::set_table_properties(uint val)
  {
    m_properties = val;
  }

}
// namespace AQP

#endif
