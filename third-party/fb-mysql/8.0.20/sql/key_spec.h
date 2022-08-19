/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef KEY_SPEC_INCLUDED
#define KEY_SPEC_INCLUDED

#include <sys/types.h>

#include "lex_string.h"
#include "m_string.h"
#include "my_base.h"
#include "sql/mem_root_array.h"
#include "sql/sql_list.h"

class Create_field;
class Item;
class THD;
struct MEM_ROOT;

enum keytype {
  KEYTYPE_PRIMARY,
  KEYTYPE_UNIQUE,
  KEYTYPE_MULTIPLE,
  KEYTYPE_FULLTEXT,
  KEYTYPE_SPATIAL,
  KEYTYPE_FOREIGN
};

enum fk_option {
  FK_OPTION_UNDEF,
  FK_OPTION_RESTRICT,
  FK_OPTION_CASCADE,
  FK_OPTION_SET_NULL,
  FK_OPTION_NO_ACTION,
  FK_OPTION_DEFAULT
};

enum fk_match_opt {
  FK_MATCH_UNDEF,
  FK_MATCH_FULL,
  FK_MATCH_PARTIAL,
  FK_MATCH_SIMPLE
};

enum enum_order { ORDER_NOT_RELEVANT = 1, ORDER_ASC, ORDER_DESC };

class KEY_CREATE_INFO {
 public:
  enum ha_key_alg algorithm = HA_KEY_ALG_SE_SPECIFIC;
  /**
    A flag which indicates that index algorithm was explicitly specified
    by user.
  */
  bool is_algorithm_explicit = false;
  ulong block_size = 0;
  LEX_CSTRING parser_name = {NullS, 0};
  LEX_CSTRING comment = {NullS, 0};
  bool is_visible = true;

  KEY_CREATE_INFO() = default;

  explicit KEY_CREATE_INFO(bool is_visible_arg) : is_visible(is_visible_arg) {}
};

extern KEY_CREATE_INFO default_key_create_info;

class Key_part_spec {
 public:
  Key_part_spec(Item *expression, enum_order order)
      : m_is_ascending((order == ORDER_DESC) ? false : true),
        m_is_explicit(order != ORDER_NOT_RELEVANT),
        m_field_name(nullptr),
        m_prefix_length(0),
        m_expression(expression),
        m_has_expression(true) {}

  Key_part_spec(const char *column_name, Item *expression, enum_order order)
      : m_is_ascending((order == ORDER_DESC) ? false : true),
        m_is_explicit(order != ORDER_NOT_RELEVANT),
        m_field_name(column_name),
        m_prefix_length(0),
        m_expression(expression),
        m_has_expression(true) {}

  Key_part_spec(LEX_CSTRING column_name, uint prefix_length, enum_order order)
      : m_is_ascending((order == ORDER_DESC) ? false : true),
        m_is_explicit(order != ORDER_NOT_RELEVANT),
        m_field_name(column_name.str),
        m_prefix_length(prefix_length),
        m_expression(nullptr),
        m_has_expression(false) {}

  bool operator==(const Key_part_spec &other) const;
  /**
    Construct a copy of this Key_part_spec. field_name is copied
    by-pointer as it is known to never change. At the same time
    'length' may be reset in mysql_prepare_create_table, and this
    is why we supply it with a copy.

    @return If out of memory, 0 is returned and an error is set in
    THD.
  */
  Key_part_spec *clone(MEM_ROOT *mem_root) const {
    return new (mem_root) Key_part_spec(*this);
  }

  const char *get_field_name() const { return m_field_name; }

  uint get_prefix_length() const { return m_prefix_length; }

  Item *get_expression() const {
    DBUG_ASSERT(has_expression());
    return m_expression;
  }

  /**
    @retval true  if this is an ascending index.
    @retval false if this is a descending index.
  */
  bool is_ascending() const { return m_is_ascending; }

  /**
    @retval true  if the user explicitly specified the index direction when
                  creating the index.
    @retval false if the user didn't specify the index direction.
  */
  bool is_explicit() const { return m_is_explicit; }

  /**
    Resolve the expression that this key part contains. Should only be called
    if has_expression() returns true.

    @param thd thread handler.

    @retval true if an error occurred.
    @retval false on success.
  */
  bool resolve_expression(THD *thd);

  /**
    Set the name and the prefix length of the column this key part references.
    The supplied column name string should have a lifetime equal to or longer
    than this Key_part_spec

    @param name the new column that this key part points to.
    @param prefix_length the prefix length of the index, or 0 if no length is
                         specified.
  */
  void set_name_and_prefix_length(const char *name, uint prefix_length);

  /**
    @retval true if this index has an expression. In that case, this a
                 functional key part.
    @retval false if this index doesn't have an expression. In that case this
                  key part references a normal column.
  */
  bool has_expression() const { return m_has_expression; }

 private:
  /// true <=> ascending, false <=> descending.
  const bool m_is_ascending;

  /// true <=> ASC/DESC is explicitly specified, false <=> implicit ASC
  const bool m_is_explicit;

  /// The name of the column that this key part points to.
  const char *m_field_name;

  /// The prefix length of this index.
  uint m_prefix_length;

  /**
    The indexed expression if this is a functional key part. If this key part
    points to a "normal" column, m_expression is nullptr.
  */
  Item *m_expression;

  /**
    Whether this key part has an expression or not. If so, this is a functional
    key part.
  */
  bool m_has_expression;
};

class Key_spec {
 public:
  const keytype type;
  const KEY_CREATE_INFO key_create_info;
  Mem_root_array<Key_part_spec *> columns;
  LEX_CSTRING name;
  const bool generated;
  /**
    A flag to determine if we will check for duplicate indexes.
    This typically means that the key information was specified
    directly by the user (set by the parser) or a column
    associated with it was dropped.
  */
  const bool check_for_duplicate_indexes;

  Key_spec(MEM_ROOT *mem_root, keytype type_par, const LEX_CSTRING &name_arg,
           const KEY_CREATE_INFO *key_info_arg, bool generated_arg,
           bool check_for_duplicate_indexes_arg, List<Key_part_spec> &cols)
      : type(type_par),
        key_create_info(*key_info_arg),
        columns(mem_root),
        name(name_arg),
        generated(generated_arg),
        check_for_duplicate_indexes(check_for_duplicate_indexes_arg) {
    columns.reserve(cols.elements);
    List_iterator<Key_part_spec> it(cols);
    Key_part_spec *column;
    while ((column = it++)) columns.push_back(column);
  }

  virtual ~Key_spec() {}
};

class Foreign_key_spec : public Key_spec {
 public:
  const LEX_CSTRING ref_db;
  const LEX_CSTRING orig_ref_db;
  const LEX_CSTRING ref_table;
  const LEX_CSTRING orig_ref_table;
  Mem_root_array<Key_part_spec *> ref_columns;
  const fk_option delete_opt;
  const fk_option update_opt;
  const fk_match_opt match_opt;
  /**
    Indicates whether foreign key name was provided explicitly or
    was generated automatically.

    @todo Get rid of this flag once we implement a better way for
          NDB SE to get generated foreign key name from SQL-layer.
    @sa   prepare_foreign_key().
  */
  const bool has_explicit_name;

  Foreign_key_spec(MEM_ROOT *mem_root, const LEX_CSTRING &name_arg,
                   List<Key_part_spec> cols, const LEX_CSTRING &ref_db_arg,
                   const LEX_CSTRING &orig_ref_db_arg,
                   const LEX_CSTRING &ref_table_arg,
                   const LEX_CSTRING &orig_ref_table_arg,
                   List<Key_part_spec> *ref_cols, fk_option delete_opt_arg,
                   fk_option update_opt_arg, fk_match_opt match_opt_arg)
      : Key_spec(mem_root, KEYTYPE_FOREIGN, name_arg, &default_key_create_info,
                 false,
                 false,  // We don't check for duplicate FKs.
                 cols),
        ref_db(ref_db_arg),
        orig_ref_db(orig_ref_db_arg),
        ref_table(ref_table_arg),
        orig_ref_table(orig_ref_table_arg),
        ref_columns(mem_root),
        delete_opt(delete_opt_arg),
        update_opt(update_opt_arg),
        match_opt(match_opt_arg),
        has_explicit_name(name_arg.str != nullptr) {
    if (ref_cols) {
      ref_columns.reserve(ref_cols->elements);
      List_iterator<Key_part_spec> it(*ref_cols);
      Key_part_spec *ref_column;
      while ((ref_column = it++)) ref_columns.push_back(ref_column);
    }
  }

  /**
    Check if the foreign key name has valid length and its options
    are compatible with columns on which the FK is created.

    @param thd                  Thread handle
    @param table_name           Table name (for error reporting)
    @param table_fields         List of columns

    @retval false   Key valid
    @retval true    Key invalid
 */
  bool validate(THD *thd, const char *table_name,
                List<Create_field> &table_fields) const;
};

/**
  Test if a foreign key (= generated key) is a prefix of the given key
  (ignoring key name, key type and order of columns)

  @note This is only used to test if an index for a FOREIGN KEY exists.
  We only compare field names.

  @retval false   Generated key is a prefix of other key
  @retval true    Not equal
*/
bool foreign_key_prefix(const Key_spec *a, const Key_spec *b);

#endif  // KEY_SPEC_INCLUDED
