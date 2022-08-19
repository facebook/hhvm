/* Copyright (c) 2013, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef PARSE_TREE_NODES_INCLUDED
#define PARSE_TREE_NODES_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <cctype>  // std::isspace
#include <memory>

#include "lex_string.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_bit.h"  // is_single_bit
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "my_time.h"
#include "mysqld_error.h"
#include "sql/enum_query_type.h"
#include "sql/handler.h"
#include "sql/key_spec.h"
#include "sql/mem_root_array.h"
#include "sql/opt_explain.h"         // Sql_cmd_explain_other_thread
#include "sql/parse_tree_helpers.h"  // PT_item_list
#include "sql/parse_tree_node_base.h"
#include "sql/parser_yystype.h"
#include "sql/partition_info.h"
#include "sql/resourcegroups/resource_group_basic_types.h"
#include "sql/resourcegroups/resource_group_sql_cmd.h"
#include "sql/set_var.h"
#include "sql/sql_admin.h"  // Sql_cmd_shutdown etc.
#include "sql/sql_alter.h"
#include "sql/sql_check_constraint.h"  // Sql_check_constraint_spec
#include "sql/sql_cmd_srs.h"
#include "sql/sql_exchange.h"
#include "sql/sql_lex.h"  // LEX
#include "sql/sql_list.h"
#include "sql/sql_load.h"  // Sql_cmd_load_table
#include "sql/sql_partition_admin.h"
#include "sql/sql_restart_server.h"  // Sql_cmd_restart_server
#include "sql/sql_show.h"
#include "sql/sql_tablespace.h"  // Tablespace_options
#include "sql/sql_truncate.h"    // Sql_cmd_truncate_table
#include "sql/table.h"           // Common_table_expr
#include "sql/window.h"          // Window
#include "sql/window_lex.h"
#include "thr_lock.h"

class Item;
class Item_cache;
class Item_string;
class Json_table_column;
class PT_field_def_base;
class PT_hint_list;
class PT_part_definition;
class PT_partition;
class PT_subquery;
class PT_type;
class sp_head;
class sp_name;
class Sql_cmd;
class String;
class THD;
enum class enum_jt_column;
enum class enum_jtc_on : uint16;
struct CHARSET_INFO;

/**
  @defgroup ptn  Parse tree nodes
  @ingroup  Parser
*/
/**
  @defgroup ptn_stmt  Nodes representing SQL statements
  @ingroup  ptn
*/
/**
  @defgroup ptn_create_table  CREATE TABLE statement
  @ingroup  ptn_stmt
*/
/**
  @defgroup ptn_alter_table  ALTER TABLE statement
  @ingroup  ptn_stmt
*/
/**
  @defgroup ptn_create_table_stuff  Clauses of CREATE TABLE statement
  @ingroup  ptn_create_table
*/
/**
  @defgroup ptn_partitioning CREATE/ALTER TABLE partitioning-related stuff
  @ingroup  ptn_create_table ptn_alter_table
*/
/**
  @defgroup ptn_part_options Partition options in CREATE/ALTER TABLE
  @ingroup  ptn_partitioning
*/
/**
  @defgroup ptn_create_or_alter_table_options  Table options of CREATE/ALTER
  TABLE
  @anchor   ptn_create_or_alter_table_options
  @ingroup  ptn_create_table ptn_alter_table
*/
/**
  @defgroup ptn_col_types  Column types in CREATE/ALTER TABLE
  @ingroup  ptn_create_table ptn_alter_table
*/
/**
  @defgroup ptn_col_attrs  Column attributes in CREATE/ALTER TABLE
  @ingroup  ptn_create_table ptn_alter_table
*/
/**
  @defgroup ptn_not_gcol_attr Non-generated column attributes in CREATE/ALTER
  TABLE
  @ingroup ptn_col_attrs ptn_alter_table
*/

/**
  Calls contextualize() on every node in the array.
*/
template <class Node_type, class Parse_context_type>
bool contextualize_nodes(Mem_root_array_YY<Node_type *> nodes,
                         Parse_context_type *pc) {
  for (Node_type *i : nodes)
    if (i->contextualize(pc)) return true;
  return false;
}

/**
  Base class for all top-level nodes of SQL statements

  @ingroup ptn_stmt
*/
class Parse_tree_root {
  Parse_tree_root(const Parse_tree_root &) = delete;
  void operator=(const Parse_tree_root &) = delete;

 protected:
  virtual ~Parse_tree_root() {}
  Parse_tree_root() {}

 public:
  virtual Sql_cmd *make_cmd(THD *thd) = 0;
};

class PT_table_ddl_stmt_base : public Parse_tree_root {
 public:
  explicit PT_table_ddl_stmt_base(MEM_ROOT *mem_root)
      : m_alter_info(mem_root) {}

  virtual ~PT_table_ddl_stmt_base() = 0;  // force abstract class

 protected:
  Alter_info m_alter_info;
};

inline PT_table_ddl_stmt_base::~PT_table_ddl_stmt_base() {}

/**
  Parse context for the table DDL (ALTER TABLE and CREATE TABLE) nodes.

  For internal use in the contextualization code.
*/
struct Table_ddl_parse_context final : public Parse_context {
  Table_ddl_parse_context(THD *thd_arg, SELECT_LEX *select_arg,
                          Alter_info *alter_info);
  HA_CREATE_INFO *const create_info;
  Alter_info *const alter_info;
  KEY_CREATE_INFO *const key_create_info;
};

/**
  Base class for all table DDL (ALTER TABLE and CREATE TABLE) nodes.
*/
typedef Parse_tree_node_tmpl<Table_ddl_parse_context> Table_ddl_node;

class PT_order_expr : public Parse_tree_node, public ORDER {
  typedef Parse_tree_node super;

 public:
  PT_order_expr(Item *item_arg, enum_order dir) {
    item_ptr = item_arg;
    direction = (dir == ORDER_DESC) ? ORDER_DESC : ORDER_ASC;
  }

  bool contextualize(Parse_context *pc) override;
};

class PT_order_list : public Parse_tree_node {
  typedef Parse_tree_node super;

 public:
  SQL_I_List<ORDER> value;

 public:
  bool contextualize(Parse_context *pc) override {
    if (super::contextualize(pc)) return true;
    for (ORDER *o = value.first; o != nullptr; o = o->next) {
      if (static_cast<PT_order_expr *>(o)->contextualize(pc)) return true;
    }
    return false;
  }

  void push_back(PT_order_expr *order) {
    order->item = &order->item_ptr;
    order->used_alias = false;
    order->used = 0;
    order->is_position = false;
    value.link_in_list(order, &order->next);
  }
};

class PT_gorder_list : public PT_order_list {
  typedef PT_order_list super;

 public:
  bool contextualize(Parse_context *pc) override {
    return super::contextualize(pc);
  }
};

/**
  Represents an element of the WITH list:
  WITH [...], [...] SELECT ...,
         ^  or  ^
  i.e. a Common Table Expression (CTE, or Query Name in SQL99 terms).
*/
class PT_common_table_expr : public Parse_tree_node {
  typedef Parse_tree_node super;

 public:
  explicit PT_common_table_expr(const LEX_STRING &name,
                                const LEX_STRING &subq_text,
                                uint subq_text_offset, PT_subquery *sn,
                                const Create_col_name_list *column_names,
                                MEM_ROOT *mem_root);

  /// The name after AS
  const LEX_STRING &name() const { return m_name; }
  /**
    @param      thd  Thread handler
    @param[out] node PT_subquery
    @returns a PT_subquery to attach to a table reference for this CTE
  */
  bool make_subquery_node(THD *thd, PT_subquery **node);
  /**
    @param tl  Table reference to match
    @param in_self  If this is a recursive reference
    @param[out]  found Is set to true/false if matches or not
    @returns true if error
  */
  bool match_table_ref(TABLE_LIST *tl, bool in_self, bool *found);
  /**
    @returns true if 'other' is the same instance as 'this'
  */
  bool is(const Common_table_expr *other) const {
    return other == &m_postparse;
  }
  void print(const THD *thd, String *str, enum_query_type query_type);

 private:
  LEX_STRING m_name;
  /// Raw text of query expression (including parentheses)
  const LEX_STRING m_subq_text;
  /**
    Offset in bytes of m_subq_text in original statement which had the WITH
    clause.
  */
  uint m_subq_text_offset;
  /// Parsed version of subq_text
  PT_subquery *const m_subq_node;
  /// List of explicitely specified column names; if empty, no list.
  const Create_col_name_list m_column_names;
  /**
    A TABLE_LIST representing a CTE needs access to the WITH list
    element it derives from. However, in order to:
    - limit the members which TABLE_LIST can access
    - avoid including this header file everywhere TABLE_LIST needs to access
    these members,
    these members are relocated into a separate inferior object whose
    declaration is in table.h, like that of TABLE_LIST. It's the "postparse"
    part. TABLE_LIST accesses this inferior object only.
  */
  Common_table_expr m_postparse;

  friend bool SELECT_LEX_UNIT::clear_correlated_query_blocks();
};

/**
   Represents the WITH list.
   WITH [...], [...] SELECT ...,
        ^^^^^^^^^^^^
*/
class PT_with_list : public Parse_tree_node {
  typedef Parse_tree_node super;

 public:
  /// @param mem_root where interior objects are allocated
  explicit PT_with_list(MEM_ROOT *mem_root) : m_elements(mem_root) {}
  bool push_back(PT_common_table_expr *el);
  const Mem_root_array<PT_common_table_expr *> &elements() const {
    return m_elements;
  }

 private:
  Mem_root_array<PT_common_table_expr *> m_elements;
};

/**
  Represents the WITH clause:
  WITH [...], [...] SELECT ...,
  ^^^^^^^^^^^^^^^^^
*/
class PT_with_clause : public Parse_tree_node {
  typedef Parse_tree_node super;

 public:
  PT_with_clause(const PT_with_list *l, bool r)
      : m_list(l), m_recursive(r), m_most_inner_in_parsing(nullptr) {}

  bool contextualize(Parse_context *pc) override;

  /**
    Looks up a table reference into the list of CTEs.
    @param      tl    Table reference to look up
    @param[out] found Is set to true/false if found or not
    @returns true if error
  */
  bool lookup(TABLE_LIST *tl, PT_common_table_expr **found);
  /**
    Call this to record in the WITH clause that we are contextualizing the
    CTE definition inserted in table reference 'tl'.
    @returns information which the caller must provide to
    leave_parsing_definition().
  */
  const TABLE_LIST *enter_parsing_definition(TABLE_LIST *tl) {
    auto old = m_most_inner_in_parsing;
    m_most_inner_in_parsing = tl;
    return old;
  }
  void leave_parsing_definition(const TABLE_LIST *old) {
    m_most_inner_in_parsing = old;
  }
  void print(const THD *thd, String *str, enum_query_type query_type);

 private:
  /// All CTEs of this clause
  const PT_with_list *const m_list;
  /// True if the user has specified the RECURSIVE keyword.
  const bool m_recursive;
  /**
    The innermost CTE reference which we're parsing at the
    moment. Used to detect forward references, loops and recursiveness.
  */
  const TABLE_LIST *m_most_inner_in_parsing;

  friend bool SELECT_LEX_UNIT::clear_correlated_query_blocks();
};

class PT_select_item_list : public PT_item_list {
  typedef PT_item_list super;

 public:
  bool contextualize(Parse_context *pc) override;
};

class PT_limit_clause : public Parse_tree_node {
  typedef Parse_tree_node super;

  Limit_options limit_options;

 public:
  PT_limit_clause(const Limit_options &limit_options_arg)
      : limit_options(limit_options_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_cross_join;
class PT_joined_table;

class PT_table_reference : public Parse_tree_node {
 public:
  TABLE_LIST *value;

  /**
    Lets us build a parse tree top-down, which is necessary due to the
    context-dependent nature of the join syntax. This function adds
    the `<table_ref>` cross join as the left-most leaf in this join tree
    rooted at this node.

    @todo: comment on non-join PT_table_reference objects

    @param cj This `<table ref>` will be added if it represents a cross join.

    @return The new top-level join.
  */
  virtual PT_joined_table *add_cross_join(PT_cross_join *cj);
};

class PT_table_factor_table_ident : public PT_table_reference {
  typedef PT_table_reference super;

  Table_ident *table_ident;
  List<String> *opt_use_partition;
  const char *const opt_table_alias;
  List<Index_hint> *opt_key_definition;

 public:
  PT_table_factor_table_ident(Table_ident *table_ident_arg,
                              List<String> *opt_use_partition_arg,
                              const LEX_CSTRING &opt_table_alias_arg,
                              List<Index_hint> *opt_key_definition_arg)
      : table_ident(table_ident_arg),
        opt_use_partition(opt_use_partition_arg),
        opt_table_alias(opt_table_alias_arg.str),
        opt_key_definition(opt_key_definition_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_json_table_column : public Parse_tree_node {
 public:
  virtual Json_table_column *get_column() = 0;
};

class PT_table_factor_function : public PT_table_reference {
  typedef PT_table_reference super;

 public:
  PT_table_factor_function(Item *expr, Item *path,
                           Mem_root_array<PT_json_table_column *> *nested_cols,
                           const LEX_STRING &table_alias)
      : m_expr(expr),
        m_path(path),
        m_nested_columns(nested_cols),
        m_table_alias(table_alias) {}

  bool contextualize(Parse_context *pc) override;

 private:
  Item *m_expr;
  Item *m_path;
  Mem_root_array<PT_json_table_column *> *m_nested_columns;
  const LEX_STRING m_table_alias;
};

class PT_table_reference_list_parens : public PT_table_reference {
  typedef PT_table_reference super;

  Mem_root_array_YY<PT_table_reference *> table_list;

 public:
  explicit PT_table_reference_list_parens(
      const Mem_root_array_YY<PT_table_reference *> table_list)
      : table_list(table_list) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_derived_table : public PT_table_reference {
  typedef PT_table_reference super;

 public:
  PT_derived_table(bool lateral, PT_subquery *subquery,
                   const LEX_CSTRING &table_alias,
                   Create_col_name_list *column_names);

  bool contextualize(Parse_context *pc) override;

  // Mark this table as system.
  void mark_system() { m_system = true; }

 private:
  bool m_lateral;
  PT_subquery *m_subquery;
  const char *const m_table_alias;
  /// List of explicitely specified column names; if empty, no list.
  const Create_col_name_list column_names;

  // Tmp table should be marked as SYSTEM_TMP_TABLE.
  bool m_system{false};
};

class PT_table_factor_joined_table : public PT_table_reference {
  typedef PT_table_reference super;

 public:
  PT_table_factor_joined_table(PT_joined_table *joined_table)
      : m_joined_table(joined_table) {}

  bool contextualize(Parse_context *pc) override;

 private:
  PT_joined_table *m_joined_table;
};

class PT_joined_table : public PT_table_reference {
  typedef PT_table_reference super;

 protected:
  PT_table_reference *tab1_node;
  POS join_pos;
  PT_joined_table_type m_type;
  PT_table_reference *tab2_node;

  TABLE_LIST *tr1;
  TABLE_LIST *tr2;

 public:
  PT_joined_table(PT_table_reference *tab1_node_arg, const POS &join_pos_arg,
                  PT_joined_table_type type, PT_table_reference *tab2_node_arg)
      : tab1_node(tab1_node_arg),
        join_pos(join_pos_arg),
        m_type(type),
        tab2_node(tab2_node_arg),
        tr1(nullptr),
        tr2(nullptr) {
    static_assert(is_single_bit(JTT_INNER), "not a single bit");
    static_assert(is_single_bit(JTT_STRAIGHT), "not a single bit");
    static_assert(is_single_bit(JTT_NATURAL), "not a single bit");
    static_assert(is_single_bit(JTT_LEFT), "not a single bit");
    static_assert(is_single_bit(JTT_RIGHT), "not a single bit");

    DBUG_ASSERT(type == JTT_INNER || type == JTT_STRAIGHT_INNER ||
                type == JTT_NATURAL_INNER || type == JTT_NATURAL_LEFT ||
                type == JTT_NATURAL_RIGHT || type == JTT_LEFT ||
                type == JTT_RIGHT);
  }

  /**
    Adds the cross join to this join operation. The cross join is nested as
    the table reference on the left-hand side.
  */
  PT_joined_table *add_cross_join(PT_cross_join *cj) override {
    tab1_node = tab1_node->add_cross_join(cj);
    return this;
  }

  /// Adds the table reference as the right-hand side of this join.
  void add_rhs(PT_table_reference *table) {
    DBUG_ASSERT(tab2_node == nullptr);
    tab2_node = table;
  }

  bool contextualize(Parse_context *pc) override;

  /// This class is being inherited, it should thus be abstract.
  ~PT_joined_table() override = 0;

 protected:
  bool contextualize_tabs(Parse_context *pc) {
    if (tr1 != nullptr) return false;  // already done

    if (tab1_node->contextualize(pc) || tab2_node->contextualize(pc))
      return true;

    tr1 = tab1_node->value;
    tr2 = tab2_node->value;

    if (tr1 == nullptr || tr2 == nullptr) {
      error(pc, join_pos);
      return true;
    }
    return false;
  }
};

inline PT_joined_table::~PT_joined_table() {}

class PT_cross_join : public PT_joined_table {
  typedef PT_joined_table super;

 public:
  PT_cross_join(PT_table_reference *tab1_node_arg, const POS &join_pos_arg,
                PT_joined_table_type Type_arg,
                PT_table_reference *tab2_node_arg)
      : PT_joined_table(tab1_node_arg, join_pos_arg, Type_arg, tab2_node_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_joined_table_on : public PT_joined_table {
  typedef PT_joined_table super;
  Item *on;

 public:
  PT_joined_table_on(PT_table_reference *tab1_node_arg, const POS &join_pos_arg,
                     PT_joined_table_type type,
                     PT_table_reference *tab2_node_arg, Item *on_arg)
      : super(tab1_node_arg, join_pos_arg, type, tab2_node_arg), on(on_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_joined_table_using : public PT_joined_table {
  typedef PT_joined_table super;
  List<String> *using_fields;

 public:
  PT_joined_table_using(PT_table_reference *tab1_node_arg,
                        const POS &join_pos_arg, PT_joined_table_type type,
                        PT_table_reference *tab2_node_arg,
                        List<String> *using_fields_arg)
      : super(tab1_node_arg, join_pos_arg, type, tab2_node_arg),
        using_fields(using_fields_arg) {}

  /// A PT_joined_table_using without a list of columns denotes a natural join.
  PT_joined_table_using(PT_table_reference *tab1_node_arg,
                        const POS &join_pos_arg, PT_joined_table_type type,
                        PT_table_reference *tab2_node_arg)
      : PT_joined_table_using(tab1_node_arg, join_pos_arg, type, tab2_node_arg,
                              nullptr) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_group : public Parse_tree_node {
  typedef Parse_tree_node super;

  PT_order_list *group_list;
  olap_type olap;

 public:
  PT_group(PT_order_list *group_list_arg, olap_type olap_arg)
      : group_list(group_list_arg), olap(olap_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_order : public Parse_tree_node {
  typedef Parse_tree_node super;

  PT_order_list *order_list;

 public:
  explicit PT_order(PT_order_list *order_list_arg)
      : order_list(order_list_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_locking_clause : public Parse_tree_node {
 public:
  PT_locking_clause(Lock_strength strength, Locked_row_action action)
      : m_lock_strength(strength), m_locked_row_action(action) {}

  bool contextualize(Parse_context *pc) override final;

  virtual bool set_lock_for_tables(Parse_context *pc) = 0;

  Locked_row_action action() const { return m_locked_row_action; }

 protected:
  Lock_descriptor get_lock_descriptor() const {
    thr_lock_type lock_type = TL_IGNORE;
    switch (m_lock_strength) {
      case Lock_strength::UPDATE:
        lock_type = TL_WRITE;
        break;
      case Lock_strength::SHARE:
        lock_type = TL_READ_WITH_SHARED_LOCKS;
        break;
    }

    return {lock_type, static_cast<thr_locked_row_action>(action())};
  }

 private:
  Lock_strength m_lock_strength;
  Locked_row_action m_locked_row_action;
};

class PT_query_block_locking_clause : public PT_locking_clause {
 public:
  explicit PT_query_block_locking_clause(
      Lock_strength strength,
      Locked_row_action action = Locked_row_action::WAIT)
      : PT_locking_clause(strength, action) {}

  bool set_lock_for_tables(Parse_context *pc) override;
};

class PT_table_locking_clause : public PT_locking_clause {
 public:
  typedef Mem_root_array_YY<Table_ident *> Table_ident_list;

  PT_table_locking_clause(Lock_strength strength,
                          Mem_root_array_YY<Table_ident *> tables,
                          Locked_row_action action)
      : PT_locking_clause(strength, action), m_tables(tables) {}

  bool set_lock_for_tables(Parse_context *pc) override;

 private:
  /// @todo Move this function to Table_ident?
  void print_table_ident(const THD *thd, const Table_ident *ident, String *s);

  bool raise_error(THD *thd, const Table_ident *name, int error);

  bool raise_error(int error);

  Table_ident_list m_tables;
};

class PT_locking_clause_list : public Parse_tree_node {
 public:
  PT_locking_clause_list(MEM_ROOT *mem_root) {
    m_locking_clauses.init(mem_root);
  }

  bool push_back(PT_locking_clause *locking_clause) {
    return m_locking_clauses.push_back(locking_clause);
  }

  bool contextualize(Parse_context *pc) override {
    for (auto locking_clause : m_locking_clauses)
      if (locking_clause->contextualize(pc)) return true;
    return false;
  }

 private:
  Mem_root_array_YY<PT_locking_clause *> m_locking_clauses;
};

class PT_query_expression_body : public Parse_tree_node {
 public:
  virtual bool is_union() const = 0;

  /**
    True if this query expression can absorb an extraneous order by/limit
    clause. The `ORDER BY`/`LIMIT` syntax is mostly consistestent, i.e. a
    trailing clause may not refer to the tables in the `<query primary>`, with
    one glaring exception:

        (...( SELECT ... )...) ORDER BY ...

    If the nested query expression doesn't contain `ORDER BY`, the statement
    is interpreted as if the `ORDER BY` was absorbed by the innermost query
    expression, i.e.:

        (...( SELECT ... ORDER BY ... )...)

    There is no rewriting of the parse tree nor AST happening here, the
    transformation is done by the contextualizer (see
    PT_query_expression::contextualize_order_and_limit), which interprets the
    parse tree, and builds the AST according to this interpretation. This
    interpretation is governed by the following rule: An `ORDER BY` can be
    absorbed if none the nested query expressions contains an `ORDER BY` *or*
    `LIMIT`. The rule is complex, so here are some examples for illustration:

    In these cases the `ORDER BY` *is* absorbed:

        ( SELECT * FROM t1 ) ORDER BY t1.a;
        (( SELECT * FROM t1 )) ORDER BY t1.a;

    In these cases the ORDER BY is *not* absorbed:

        ( SELECT * FROM t1 ORDER BY 1 ) ORDER BY t1.a;
        (( SELECT * FROM t1 ) ORDER BY 1 ) ORDER BY t1.a;
        ( SELECT * FROM t1 LIMIT 1 ) ORDER BY t1.a;
        (( SELECT * FROM t1 ) LIMIT 1 ) ORDER BY t1.a;

    The same happens with `LIMIT`, obviously, but the optimizer is freeer to
    choose when to apply the limit, and there are name no resolution issues
    involved.

    @param order  True if the outer query block has the ORDER BY clause.
    @param limit  True if the outer query block has the LIMIT clause.
  */
  virtual bool can_absorb_order_and_limit(bool order, bool limit) const = 0;
  virtual bool has_into_clause() const = 0;
  virtual bool has_trailing_into_clause() const = 0;

  virtual bool is_table_value_constructor() const = 0;
  virtual PT_insert_values_list *get_row_value_list() const = 0;
};

class PT_internal_variable_name : public Parse_tree_node {
 public:
  sys_var_with_base value;
};

class PT_internal_variable_name_1d : public PT_internal_variable_name {
  typedef PT_internal_variable_name super;

  LEX_CSTRING ident;

 public:
  PT_internal_variable_name_1d(LEX_CSTRING ident_arg) : ident(ident_arg) {}

  bool contextualize(Parse_context *pc) override;
};

/**
  Parse tree node class for 2-dimentional variable names (example: \@global.x)
*/
class PT_internal_variable_name_2d : public PT_internal_variable_name {
  typedef PT_internal_variable_name super;

 public:
  const POS pos;

 private:
  LEX_CSTRING ident1;
  LEX_CSTRING ident2;

 public:
  PT_internal_variable_name_2d(const POS &pos, LEX_CSTRING ident1_arg,
                               LEX_CSTRING ident2_arg)
      : pos(pos), ident1(ident1_arg), ident2(ident2_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_internal_variable_name_default : public PT_internal_variable_name {
  typedef PT_internal_variable_name super;

  LEX_STRING ident;

 public:
  PT_internal_variable_name_default(const LEX_STRING &ident_arg)
      : ident(ident_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_option_value_following_option_type : public Parse_tree_node {
  typedef Parse_tree_node super;

  POS pos;
  PT_internal_variable_name *name;
  Item *opt_expr;

 public:
  PT_option_value_following_option_type(const POS &pos,
                                        PT_internal_variable_name *name_arg,
                                        Item *opt_expr_arg)
      : pos(pos), name(name_arg), opt_expr(opt_expr_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_option_value_no_option_type : public Parse_tree_node {};

class PT_option_value_no_option_type_internal
    : public PT_option_value_no_option_type {
  typedef PT_option_value_no_option_type super;

  PT_internal_variable_name *name;
  Item *opt_expr;
  POS expr_pos;

 public:
  PT_option_value_no_option_type_internal(PT_internal_variable_name *name_arg,
                                          Item *opt_expr_arg,
                                          const POS &expr_pos_arg)
      : name(name_arg), opt_expr(opt_expr_arg), expr_pos(expr_pos_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_option_value_no_option_type_user_var
    : public PT_option_value_no_option_type {
  typedef PT_option_value_no_option_type super;

  LEX_STRING name;
  Item *expr;

 public:
  PT_option_value_no_option_type_user_var(const LEX_STRING &name_arg,
                                          Item *expr_arg)
      : name(name_arg), expr(expr_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_option_value_no_option_type_sys_var
    : public PT_option_value_no_option_type {
  typedef PT_option_value_no_option_type super;

  enum_var_type type;
  PT_internal_variable_name *name;
  Item *opt_expr;

 public:
  PT_option_value_no_option_type_sys_var(enum_var_type type_arg,
                                         PT_internal_variable_name *name_arg,
                                         Item *opt_expr_arg)
      : type(type_arg), name(name_arg), opt_expr(opt_expr_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_option_value_no_option_type_charset
    : public PT_option_value_no_option_type {
  typedef PT_option_value_no_option_type super;

  const CHARSET_INFO *opt_charset;

 public:
  PT_option_value_no_option_type_charset(const CHARSET_INFO *opt_charset_arg)
      : opt_charset(opt_charset_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_option_value_no_option_type_names
    : public PT_option_value_no_option_type {
  typedef PT_option_value_no_option_type super;

  POS pos;

 public:
  explicit PT_option_value_no_option_type_names(const POS &pos) : pos(pos) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_set_names : public PT_option_value_no_option_type {
  typedef PT_option_value_no_option_type super;

  const CHARSET_INFO *opt_charset;
  const CHARSET_INFO *opt_collation;

 public:
  PT_set_names(const CHARSET_INFO *opt_charset_arg,
               const CHARSET_INFO *opt_collation_arg)
      : opt_charset(opt_charset_arg), opt_collation(opt_collation_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_start_option_value_list : public Parse_tree_node {};

class PT_option_value_no_option_type_password
    : public PT_start_option_value_list {
  typedef PT_start_option_value_list super;

  const char *password;
  const char *current_password;
  bool retain_current_password;
  bool random_password_generator;
  POS expr_pos;

 public:
  PT_option_value_no_option_type_password(const char *password_arg,
                                          const char *current_password_arg,
                                          bool retain_current,
                                          bool random_password,
                                          const POS &expr_pos_arg)
      : password(password_arg),
        current_password(current_password_arg),
        retain_current_password(retain_current),
        random_password_generator(random_password),
        expr_pos(expr_pos_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_option_value_no_option_type_password_for
    : public PT_start_option_value_list {
  typedef PT_start_option_value_list super;

  LEX_USER *user;
  const char *password;
  const char *current_password;
  bool retain_current_password;
  bool random_password_generator;
  POS expr_pos;

 public:
  PT_option_value_no_option_type_password_for(LEX_USER *user_arg,
                                              const char *password_arg,
                                              const char *current_password_arg,
                                              bool retain_current,
                                              bool random_pass,
                                              const POS &expr_pos_arg)
      : user(user_arg),
        password(password_arg),
        current_password(current_password_arg),
        retain_current_password(retain_current),
        random_password_generator(random_pass),
        expr_pos(expr_pos_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_option_value_type : public Parse_tree_node {
  enum_var_type option_type;
  ulong thread_id;
  PT_option_value_following_option_type *value;

 public:
  PT_option_value_type(enum_var_type option_type_arg, ulong thread_id_arg,
                       PT_option_value_following_option_type *value_arg)
      : option_type(option_type_arg),
        thread_id(thread_id_arg),
        value(value_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_option_value_list_head : public Parse_tree_node {
  typedef Parse_tree_node super;

  POS delimiter_pos;
  Parse_tree_node *value;
  POS value_pos;

 public:
  PT_option_value_list_head(const POS &delimiter_pos_arg,
                            Parse_tree_node *value_arg,
                            const POS &value_pos_arg)
      : delimiter_pos(delimiter_pos_arg),
        value(value_arg),
        value_pos(value_pos_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_option_value_list : public PT_option_value_list_head {
  typedef PT_option_value_list_head super;

  PT_option_value_list_head *head;

 public:
  PT_option_value_list(PT_option_value_list_head *head_arg,
                       const POS &delimiter_pos_arg, Parse_tree_node *tail,
                       const POS &tail_pos)
      : super(delimiter_pos_arg, tail, tail_pos), head(head_arg) {}

  bool contextualize(Parse_context *pc) override {
    return head->contextualize(pc) || super::contextualize(pc);
  }
};

class PT_start_option_value_list_no_type : public PT_start_option_value_list {
  typedef PT_start_option_value_list super;

  PT_option_value_no_option_type *head;
  POS head_pos;
  PT_option_value_list_head *tail;

 public:
  PT_start_option_value_list_no_type(PT_option_value_no_option_type *head_arg,
                                     const POS &head_pos_arg,
                                     PT_option_value_list_head *tail_arg)
      : head(head_arg), head_pos(head_pos_arg), tail(tail_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_transaction_characteristic : public Parse_tree_node {
  typedef Parse_tree_node super;

  const char *name;
  int32 value;

 public:
  PT_transaction_characteristic(const char *name_arg, int32 value_arg)
      : name(name_arg), value(value_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_transaction_access_mode : public PT_transaction_characteristic {
  typedef PT_transaction_characteristic super;

 public:
  explicit PT_transaction_access_mode(bool is_read_only)
      : super("transaction_read_only", (int32)is_read_only) {}
};

class PT_isolation_level : public PT_transaction_characteristic {
  typedef PT_transaction_characteristic super;

 public:
  explicit PT_isolation_level(enum_tx_isolation level)
      : super("transaction_isolation", (int32)level) {}
};

class PT_transaction_characteristics : public Parse_tree_node {
  typedef Parse_tree_node super;

  PT_transaction_characteristic *head;
  PT_transaction_characteristic *opt_tail;

 public:
  PT_transaction_characteristics(PT_transaction_characteristic *head_arg,
                                 PT_transaction_characteristic *opt_tail_arg)
      : head(head_arg), opt_tail(opt_tail_arg) {}

  bool contextualize(Parse_context *pc) override {
    return (super::contextualize(pc) || head->contextualize(pc) ||
            (opt_tail != nullptr && opt_tail->contextualize(pc)));
  }
};

class PT_start_option_value_list_transaction
    : public PT_start_option_value_list {
  typedef PT_start_option_value_list super;

  PT_transaction_characteristics *characteristics;
  POS end_pos;

 public:
  PT_start_option_value_list_transaction(
      PT_transaction_characteristics *characteristics_arg,
      const POS &end_pos_arg)
      : characteristics(characteristics_arg), end_pos(end_pos_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_start_option_value_list_following_option_type
    : public Parse_tree_node {};

class PT_start_option_value_list_following_option_type_eq
    : public PT_start_option_value_list_following_option_type {
  typedef PT_start_option_value_list_following_option_type super;

  enum_var_type option_type;
  ulong thread_id;
  PT_option_value_following_option_type *head;
  POS head_pos;
  PT_option_value_list_head *opt_tail;

 public:
  PT_start_option_value_list_following_option_type_eq(
      enum_var_type option_type_arg, ulong thread_id_arg,
      PT_option_value_following_option_type *head_arg, const POS &head_pos_arg,
      PT_option_value_list_head *opt_tail_arg)
      : option_type(option_type_arg),
        thread_id(thread_id_arg),
        head(head_arg),
        head_pos(head_pos_arg),
        opt_tail(opt_tail_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_start_option_value_list_following_option_type_transaction
    : public PT_start_option_value_list_following_option_type {
  typedef PT_start_option_value_list_following_option_type super;

  enum_var_type type;
  PT_transaction_characteristics *characteristics;
  POS characteristics_pos;

 public:
  PT_start_option_value_list_following_option_type_transaction(
      enum_var_type type_arg,
      PT_transaction_characteristics *characteristics_arg,
      const POS &characteristics_pos_arg)
      : type(type_arg),
        characteristics(characteristics_arg),
        characteristics_pos(characteristics_pos_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_start_option_value_list_type : public PT_start_option_value_list {
  typedef PT_start_option_value_list super;

  PT_start_option_value_list_following_option_type *list;

 public:
  PT_start_option_value_list_type(
      PT_start_option_value_list_following_option_type *list_arg)
      : list(list_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_set : public Parse_tree_node {
  typedef Parse_tree_node super;

  POS set_pos;
  PT_start_option_value_list *list;

 public:
  PT_set(const POS &set_pos_arg, PT_start_option_value_list *list_arg)
      : set_pos(set_pos_arg), list(list_arg) {}

  bool contextualize(Parse_context *pc) override;
};

class PT_into_destination : public Parse_tree_node {
  typedef Parse_tree_node super;
  POS m_pos;

 protected:
  PT_into_destination(const POS &pos) : m_pos(pos) {}

 public:
  bool contextualize(Parse_context *pc) override;
};

class PT_into_destination_outfile final : public PT_into_destination {
  typedef PT_into_destination super;

 public:
  PT_into_destination_outfile(const POS &pos, const LEX_STRING &file_name_arg,
                              Item *chunk_expr, const CHARSET_INFO *charset_arg,
                              const Field_separators &field_term_arg,
                              const Line_separators &line_term_arg)
      : PT_into_destination(pos), m_exchange(file_name_arg.str, false) {
    m_exchange.cs = charset_arg;
    m_exchange.field.merge_field_separators(field_term_arg);
    m_exchange.line.merge_line_separators(line_term_arg);
    m_exchange.compressed_chunk_expr = chunk_expr;
  }

  bool contextualize(Parse_context *pc) override;

 private:
  sql_exchange m_exchange;
};

class PT_into_destination_dumpfile final : public PT_into_destination {
  typedef PT_into_destination super;

 public:
  PT_into_destination_dumpfile(const POS &pos, const LEX_STRING &file_name_arg)
      : PT_into_destination(pos), m_exchange(file_name_arg.str, true) {}

  bool contextualize(Parse_context *pc) override;

 private:
  sql_exchange m_exchange;
};

class PT_select_var : public Parse_tree_node {
 public:
  const LEX_STRING name;

  explicit PT_select_var(const LEX_STRING &name_arg) : name(name_arg) {}

  virtual bool is_local() const { return false; }
  virtual uint get_offset() const {
    DBUG_ASSERT(0);
    return 0;
  }
};

class PT_select_sp_var : public PT_select_var {
  typedef PT_select_var super;

  uint offset;

#ifndef DBUG_OFF
  /*
    Routine to which this Item_splocal belongs. Used for checking if correct
    runtime context is used for variable handling.
  */
  sp_head *sp;
#endif

 public:
  PT_select_sp_var(const LEX_STRING &name_arg) : super(name_arg) {}

  bool is_local() const override { return true; }
  uint get_offset() const override { return offset; }

  bool contextualize(Parse_context *pc) override;
};

class PT_select_var_list : public PT_into_destination {
  typedef PT_into_destination super;

 public:
  explicit PT_select_var_list(const POS &pos) : PT_into_destination(pos) {}

  List<PT_select_var> value;

  bool contextualize(Parse_context *pc) override;

  bool push_back(PT_select_var *var) { return value.push_back(var); }
};

/**
  Parse tree node for a single of a window extent's borders,
  cf. \<window frame extent\> in SQL 2003.
*/
class PT_border : public Parse_tree_node {
  friend class Window;
  Item *m_value{nullptr};  ///< only relevant iff m_border_type == WBT_VALUE_*
 public:
  enum_window_border_type m_border_type;
  const bool m_date_time;
  interval_type m_int_type;

  ///< For unbounded border
  PT_border(enum_window_border_type type)
      : m_border_type(type), m_date_time(false) {
    DBUG_ASSERT(type != WBT_VALUE_PRECEDING && type != WBT_VALUE_FOLLOWING);
  }

  ///< For bounded non-temporal border, e.g. 2 PRECEDING: 'value' is 2.
  PT_border(enum_window_border_type type, Item *value)
      : m_value(value), m_border_type(type), m_date_time(false) {}

  ///< For bounded INTERVAL 2 DAYS, 'value' is 2, int_type is DAYS.
  PT_border(enum_window_border_type type, Item *value, interval_type int_type)
      : m_value(value),
        m_border_type(type),
        m_date_time(true),
        m_int_type(int_type) {}

  ///< @returns the '2' in '2 PRECEDING' or 'INTERVAL 2 DAYS PRECEDING'
  Item *border() const { return m_value; }
  /// Need such low-level access so that fix_fields updates the right pointer
  Item **border_ptr() { return &m_value; }

  /**
    @returns Addition operator for computation of frames, nullptr if error.
    @param  order_expr  Expression to add/substract to
    @param  prec    true if PRECEDING
    @param  asc     true if ASC
    @param  window  only used for error generation
  */
  Item *build_addop(Item_cache *order_expr, bool prec, bool asc,
                    const Window *window);
};

/**
  Parse tree node for one or both of a window extent's borders, cf.
  \<window frame extent\> in SQL 2003.
*/
class PT_borders : public Parse_tree_node {
  PT_border *m_borders[2];
  friend class PT_frame;

 public:
  /**
    Constructor.

    Frames of the form "frame_start no_frame_end" are translated during
    parsing to "BETWEEN frame_start AND CURRENT ROW". So both 'start' and
    'end' are non-nullptr.
  */
  PT_borders(PT_border *start, PT_border *end) {
    m_borders[0] = start;
    m_borders[1] = end;
  }
};

/**
  Parse tree node for a window frame's exclusions, cf. the
  \<window frame exclusion\> clause in SQL 2003.
*/
class PT_exclusion : public Parse_tree_node {
  enum_window_frame_exclusion m_exclusion;

 public:
  PT_exclusion(enum_window_frame_exclusion e) : m_exclusion(e) {}
  // enum_window_frame_exclusion exclusion() { return m_exclusion; }
};

/**
  Parse tree node for a window's frame, cf. the \<window frame clause\>
  in SQL 2003.
*/
class PT_frame : public Parse_tree_node {
 public:
  enum_window_frame_unit m_unit;

  PT_border *m_from;
  PT_border *m_to;

  PT_exclusion *m_exclusion;

  /// If true, this is an artificial frame, not specified by the user
  bool m_originally_absent = false;

  PT_frame(enum_window_frame_unit unit, PT_borders *from_to,
           PT_exclusion *exclusion)
      : m_unit(unit),
        m_from(from_to->m_borders[0]),
        m_to(from_to->m_borders[1]),
        m_exclusion(exclusion) {}
};

/**
  Parse tree node for a window; just a shallow wrapper for
  class Window, q.v.
*/
class PT_window : public Parse_tree_node, public Window {
  typedef Parse_tree_node super;

 public:
  PT_window(PT_order_list *partition_by, PT_order_list *order_by,
            PT_frame *frame)
      : Window(partition_by, order_by, frame) {}

  PT_window(PT_order_list *partition_by, PT_order_list *order_by,
            PT_frame *frame, Item_string *inherit)
      : Window(partition_by, order_by, frame, inherit) {}

  PT_window(Item_string *name) : Window(name) {}

  bool contextualize(Parse_context *pc) override;
};

/**
  Parse tree node for a list of window definitions corresponding
  to a \<window clause\> in SQL 2003.
*/
class PT_window_list : public Parse_tree_node {
  typedef Parse_tree_node super;
  List<Window> m_windows;

 public:
  PT_window_list() {}

  bool contextualize(Parse_context *pc) override;

  bool push_back(PT_window *w) { return m_windows.push_back(w); }
};

class PT_query_primary : public PT_query_expression_body {};

class PT_query_specification : public PT_query_primary {
  typedef PT_query_primary super;

  PT_hint_list *opt_hints;
  Query_options options;
  PT_item_list *item_list;
  PT_into_destination *opt_into1;
  const bool m_is_from_clause_implicit;
  Mem_root_array_YY<PT_table_reference *> from_clause;  // empty list for DUAL
  Item *opt_where_clause;
  PT_group *opt_group_clause;
  Item *opt_having_clause;
  PT_window_list *opt_window_clause;

 public:
  PT_query_specification(
      PT_hint_list *opt_hints_arg, const Query_options &options_arg,
      PT_item_list *item_list_arg, PT_into_destination *opt_into1_arg,
      const Mem_root_array_YY<PT_table_reference *> &from_clause_arg,
      Item *opt_where_clause_arg, PT_group *opt_group_clause_arg,
      Item *opt_having_clause_arg, PT_window_list *opt_window_clause_arg,
      bool implicit_from_clause)
      : opt_hints(opt_hints_arg),
        options(options_arg),
        item_list(item_list_arg),
        opt_into1(opt_into1_arg),
        m_is_from_clause_implicit{implicit_from_clause},
        from_clause(from_clause_arg),
        opt_where_clause(opt_where_clause_arg),
        opt_group_clause(opt_group_clause_arg),
        opt_having_clause(opt_having_clause_arg),
        opt_window_clause(opt_window_clause_arg) {
    DBUG_ASSERT(implicit_from_clause ? from_clause.empty() : true);
  }

  PT_query_specification(
      const Query_options &options_arg, PT_item_list *item_list_arg,
      const Mem_root_array_YY<PT_table_reference *> &from_clause_arg,
      Item *opt_where_clause_arg)
      : opt_hints(nullptr),
        options(options_arg),
        item_list(item_list_arg),
        opt_into1(nullptr),
        m_is_from_clause_implicit{true},
        from_clause(from_clause_arg),
        opt_where_clause(opt_where_clause_arg),
        opt_group_clause(nullptr),
        opt_having_clause(nullptr),
        opt_window_clause(nullptr) {}

  PT_query_specification(const Query_options &options_arg,
                         PT_item_list *item_list_arg)
      : opt_hints(nullptr),
        options(options_arg),
        item_list(item_list_arg),
        opt_into1(nullptr),
        m_is_from_clause_implicit{false},
        from_clause{},
        opt_where_clause(nullptr),
        opt_group_clause(nullptr),
        opt_having_clause(nullptr),
        opt_window_clause(nullptr) {}

  bool contextualize(Parse_context *pc) override;

  bool has_into_clause() const override { return opt_into1 != nullptr; }
  bool has_trailing_into_clause() const override {
    return (has_into_clause() && is_implicit_from_clause() &&
            opt_where_clause == nullptr && opt_group_clause == nullptr &&
            opt_having_clause == nullptr && opt_window_clause == nullptr);
  }

  bool is_union() const override { return false; }

  bool can_absorb_order_and_limit(bool, bool) const override { return true; }

  bool is_table_value_constructor() const override { return false; }
  PT_insert_values_list *get_row_value_list() const override { return nullptr; }

 private:
  bool is_implicit_from_clause() const { return m_is_from_clause_implicit; }
};

class PT_table_value_constructor : public PT_query_primary {
  typedef PT_query_primary super;

  PT_insert_values_list *const row_value_list;

 public:
  explicit PT_table_value_constructor(PT_insert_values_list *row_value_list_arg)
      : row_value_list(row_value_list_arg) {}

  bool contextualize(Parse_context *pc) override;

  bool has_into_clause() const override { return false; }
  bool has_trailing_into_clause() const override { return false; }

  bool is_union() const override { return false; }

  bool can_absorb_order_and_limit(bool, bool) const override { return false; }

  bool is_table_value_constructor() const override { return true; }

  PT_insert_values_list *get_row_value_list() const override {
    return row_value_list;
  }
};

class PT_explicit_table : public PT_query_specification {
  using super = PT_query_specification;

 public:
  PT_explicit_table(
      const Query_options &options_arg, PT_item_list *item_list_arg,
      const Mem_root_array_YY<PT_table_reference *> &from_clause_arg)
      : super(options_arg, item_list_arg, from_clause_arg, nullptr) {}
};

class PT_query_expression final : public PT_query_primary {
 public:
  PT_query_expression(PT_with_clause *with_clause,
                      PT_query_expression_body *body, PT_order *order,
                      PT_limit_clause *limit)
      : m_body(body),
        m_order(order),
        m_limit(limit),
        m_with_clause(with_clause) {}

  PT_query_expression(PT_query_expression_body *body, PT_order *order,
                      PT_limit_clause *limit)
      : PT_query_expression(nullptr, body, order, limit) {}

  explicit PT_query_expression(PT_query_expression_body *body)
      : PT_query_expression(body, nullptr, nullptr) {}

  bool contextualize(Parse_context *pc) override;

  bool is_union() const override { return m_body->is_union(); }

  bool has_into_clause() const override { return m_body->has_into_clause(); }
  bool has_trailing_into_clause() const override {
    return (m_body->has_trailing_into_clause() && m_order == nullptr &&
            m_limit == nullptr);
  }

  bool can_absorb_order_and_limit(bool order, bool limit) const override {
    if (m_body->is_union()) {
      return false;
    }
    if (m_order == nullptr && m_limit == nullptr) {
      /*
        It is safe to push ORDER and/or LIMIT down in:

          (SELECT ...<no order or limit clauses>) ORDER BY ... LIMIT ...;
          (SELECT ...<no order or limit clauses>) ORDER BY ...;
          (SELECT ...<no order or limit clauses>)              LIMIT ...;
      */
      return true;
    }
    if (m_limit != nullptr && !order && limit) {
      /*
        In MySQL, it is ok(*) to push LIMIT down in:

          (SELECT ... [ORDER BY ...] LIMIT a) LIMIT b;

        *) MySQL doesn't follow the standard when overwriting `LIMIT a` with
           `LIMIT b` if a < b.  Moreover, the result of:

             (SELECT ... ORDER BY order1 LIMIT a) ORDER BY order1 LIMIT b; (1)

           can diverge from:

             (SELECT ... ORDER BY order1 LIMIT a) LIMIT b;                  (2)

           since the example (1) never overwrites `LIMIT a` with `LIMIT b`,
           while the example (2) does overwrite.

           TODO: add a warning, deprecate and replace this behavior with the
                 standard one.
      */
      return true;
    }
    return false;
  }

  bool is_table_value_constructor() const override {
    return m_body->is_table_value_constructor();
  }

  PT_insert_values_list *get_row_value_list() const override {
    return m_body->get_row_value_list();
  }

 private:
  /**
    Contextualizes the order and limit clauses, re-interpreting them according
    to the rules. If the `<query expression body>` can absorb the clauses,
    they are simply contextualized into the current SELECT_LEX. If not, we
    have to create the "fake" SELECT_LEX unless there is one already
    (SELECT_LEX_UNIT::new_union_query() is known to do this.)

    @see PT_query_expression::can_absorb_order_and_limit()
  */
  bool contextualize_order_and_limit(Parse_context *pc);

  PT_query_expression_body *m_body;
  PT_order *m_order;
  PT_limit_clause *m_limit;
  PT_with_clause *m_with_clause;
};

/*
  After the removal of the `... <locking_clause> <into_clause>` syntax
  PT_locking will disappear.
*/
class PT_locking final : public PT_query_primary {
  using super = PT_query_primary;

 public:
  PT_locking(PT_query_expression_body *qe,
             PT_locking_clause_list *locking_clauses)
      : m_query_expression{qe}, m_locking_clauses{locking_clauses} {}

  bool contextualize(Parse_context *pc) override {
    return (super::contextualize(pc) || m_query_expression->contextualize(pc) ||
            m_locking_clauses->contextualize(pc));
  }

  bool is_union() const override { return m_query_expression->is_union(); }

  bool has_into_clause() const override {
    return m_query_expression->has_into_clause();
  }
  bool has_trailing_into_clause() const override { return false; }

  bool can_absorb_order_and_limit(bool order, bool limit) const override {
    return m_query_expression->can_absorb_order_and_limit(order, limit);
  }

  bool is_table_value_constructor() const override {
    return m_query_expression->is_table_value_constructor();
  }

  PT_insert_values_list *get_row_value_list() const override {
    return m_query_expression->get_row_value_list();
  }

 private:
  PT_query_expression_body *const m_query_expression;
  PT_locking_clause_list *const m_locking_clauses;
};

class PT_subquery : public Parse_tree_node {
  typedef Parse_tree_node super;

  PT_query_primary *qe;
  POS pos;
  SELECT_LEX *select_lex;

 public:
  bool m_is_derived_table;

  PT_subquery(POS p, PT_query_primary *query_expression)
      : qe(query_expression),
        pos(p),
        select_lex(nullptr),
        m_is_derived_table(false) {}

  bool contextualize(Parse_context *pc) override;

  SELECT_LEX *value() { return select_lex; }
};

class PT_union : public PT_query_expression_body {
 public:
  PT_union(PT_query_expression_body *lhs, const POS &lhs_pos, bool is_distinct,
           PT_query_primary *rhs, bool is_rhs_in_parentheses = false)
      : m_lhs(lhs),
        m_lhs_pos(lhs_pos),
        m_is_distinct(is_distinct),
        m_rhs(rhs),
        m_is_rhs_in_parentheses{is_rhs_in_parentheses} {}

  bool contextualize(Parse_context *pc) override;

  bool is_union() const override { return true; }

  bool has_into_clause() const override {
    return m_lhs->has_into_clause() || m_rhs->has_into_clause();
  }
  bool has_trailing_into_clause() const override {
    return !m_is_rhs_in_parentheses && m_rhs->has_trailing_into_clause();
  }

  bool can_absorb_order_and_limit(bool, bool) const override { return false; }

  bool is_table_value_constructor() const override { return false; }
  PT_insert_values_list *get_row_value_list() const override { return nullptr; }

 private:
  PT_query_expression_body *m_lhs;
  POS m_lhs_pos;
  bool m_is_distinct;
  PT_query_primary *m_rhs;
  PT_into_destination *m_into;
  const bool m_is_rhs_in_parentheses;
};

class PT_select_stmt : public Parse_tree_root {
  typedef Parse_tree_root super;

 public:
  /**
    @param qe The query expression.
    @param sql_command The type of SQL command.
  */
  PT_select_stmt(enum_sql_command sql_command, PT_query_expression_body *qe)
      : m_sql_command(sql_command),
        m_qe(qe),
        m_into(nullptr),
        m_has_trailing_locking_clauses{false} {}

  /**
    Creates a SELECT command. Only SELECT commands can have into.

    @param qe                           The query expression.
    @param into                         The own INTO destination.
    @param has_trailing_locking_clauses True if there are locking clauses (like
                                        `FOR UPDATE`) at the end of the
                                        statement.
  */
  explicit PT_select_stmt(PT_query_expression_body *qe,
                          PT_into_destination *into = nullptr,
                          bool has_trailing_locking_clauses = false)
      : m_sql_command{SQLCOM_SELECT},
        m_qe{qe},
        m_into{into},
        m_has_trailing_locking_clauses{has_trailing_locking_clauses} {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  enum_sql_command m_sql_command;
  PT_query_expression_body *m_qe;
  PT_into_destination *m_into;
  const bool m_has_trailing_locking_clauses;
};

/**
  Top-level node for the DELETE statement

  @ingroup ptn_stmt
*/
class PT_delete final : public Parse_tree_root {
 private:
  PT_with_clause *m_with_clause;
  PT_hint_list *opt_hints;
  const int opt_delete_options;
  Table_ident *table_ident;
  const char *const opt_table_alias;
  Mem_root_array_YY<Table_ident *> table_list;
  List<String> *opt_use_partition;
  Mem_root_array_YY<PT_table_reference *> join_table_list;
  Item *opt_where_clause;
  PT_order *opt_order_clause;
  Item *opt_delete_limit_clause;
  SQL_I_List<TABLE_LIST> delete_tables;

 public:
  // single-table DELETE node constructor:
  PT_delete(PT_with_clause *with_clause_arg, PT_hint_list *opt_hints_arg,
            int opt_delete_options_arg, Table_ident *table_ident_arg,
            const LEX_CSTRING &opt_table_alias_arg,
            List<String> *opt_use_partition_arg, Item *opt_where_clause_arg,
            PT_order *opt_order_clause_arg, Item *opt_delete_limit_clause_arg)
      : m_with_clause(with_clause_arg),
        opt_hints(opt_hints_arg),
        opt_delete_options(opt_delete_options_arg),
        table_ident(table_ident_arg),
        opt_table_alias(opt_table_alias_arg.str),
        opt_use_partition(opt_use_partition_arg),
        opt_where_clause(opt_where_clause_arg),
        opt_order_clause(opt_order_clause_arg),
        opt_delete_limit_clause(opt_delete_limit_clause_arg) {
    table_list.init_empty_const();
    join_table_list.init_empty_const();
  }

  // multi-table DELETE node constructor:
  PT_delete(PT_with_clause *with_clause_arg, PT_hint_list *opt_hints_arg,
            int opt_delete_options_arg,
            const Mem_root_array_YY<Table_ident *> &table_list_arg,
            const Mem_root_array_YY<PT_table_reference *> &join_table_list_arg,
            Item *opt_where_clause_arg)
      : m_with_clause(with_clause_arg),
        opt_hints(opt_hints_arg),
        opt_delete_options(opt_delete_options_arg),
        table_ident(nullptr),
        opt_table_alias(nullptr),
        table_list(table_list_arg),
        opt_use_partition(nullptr),
        join_table_list(join_table_list_arg),
        opt_where_clause(opt_where_clause_arg),
        opt_order_clause(nullptr),
        opt_delete_limit_clause(nullptr) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  bool is_multitable() const {
    DBUG_ASSERT((table_ident != nullptr) ^ (table_list.size() > 0));
    return table_ident == nullptr;
  }

  bool add_table(Parse_context *pc, Table_ident *table);
};

/**
  Top-level node for the UPDATE statement

  @ingroup ptn_stmt
*/
class PT_update : public Parse_tree_root {
  PT_with_clause *m_with_clause;
  PT_hint_list *opt_hints;
  thr_lock_type opt_low_priority;
  bool opt_ignore;
  Mem_root_array_YY<PT_table_reference *> join_table_list;
  PT_item_list *column_list;
  PT_item_list *value_list;
  Item *opt_where_clause;
  PT_order *opt_order_clause;
  Item *opt_limit_clause;

 public:
  PT_update(PT_with_clause *with_clause_arg, PT_hint_list *opt_hints_arg,
            thr_lock_type opt_low_priority_arg, bool opt_ignore_arg,
            const Mem_root_array_YY<PT_table_reference *> &join_table_list_arg,
            PT_item_list *column_list_arg, PT_item_list *value_list_arg,
            Item *opt_where_clause_arg, PT_order *opt_order_clause_arg,
            Item *opt_limit_clause_arg)
      : m_with_clause(with_clause_arg),
        opt_hints(opt_hints_arg),
        opt_low_priority(opt_low_priority_arg),
        opt_ignore(opt_ignore_arg),
        join_table_list(join_table_list_arg),
        column_list(column_list_arg),
        value_list(value_list_arg),
        opt_where_clause(opt_where_clause_arg),
        opt_order_clause(opt_order_clause_arg),
        opt_limit_clause(opt_limit_clause_arg) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

class PT_insert_values_list : public Parse_tree_node {
  typedef Parse_tree_node super;

  List<List_item> many_values;

 public:
  bool contextualize(Parse_context *pc) override;

  bool push_back(List<Item> *x) { return many_values.push_back(x); }

  virtual List<List_item> &get_many_values() {
    DBUG_ASSERT(is_contextualized());
    return many_values;
  }
};

/**
  Top-level node for the INSERT statement

  @ingroup ptn_stmt
*/
class PT_insert final : public Parse_tree_root {
  const bool is_replace;
  PT_hint_list *opt_hints;
  const thr_lock_type lock_option;
  const bool ignore;
  Table_ident *const table_ident;
  List<String> *const opt_use_partition;
  PT_item_list *const column_list;
  PT_insert_values_list *row_value_list;
  PT_query_primary *insert_query_expression;
  const char *const opt_values_table_alias;
  Create_col_name_list *const opt_values_column_list;
  PT_item_list *const opt_on_duplicate_column_list;
  PT_item_list *const opt_on_duplicate_value_list;

 public:
  PT_insert(bool is_replace_arg, PT_hint_list *opt_hints_arg,
            thr_lock_type lock_option_arg, bool ignore_arg,
            Table_ident *table_ident_arg, List<String> *opt_use_partition_arg,
            PT_item_list *column_list_arg,
            PT_insert_values_list *row_value_list_arg,
            PT_query_primary *insert_query_expression_arg,
            const LEX_CSTRING &opt_values_table_alias_arg,
            Create_col_name_list *opt_values_column_list_arg,
            PT_item_list *opt_on_duplicate_column_list_arg,
            PT_item_list *opt_on_duplicate_value_list_arg)
      : is_replace(is_replace_arg),
        opt_hints(opt_hints_arg),
        lock_option(lock_option_arg),
        ignore(ignore_arg),
        table_ident(table_ident_arg),
        opt_use_partition(opt_use_partition_arg),
        column_list(column_list_arg),
        row_value_list(row_value_list_arg),
        insert_query_expression(insert_query_expression_arg),
        opt_values_table_alias(opt_values_table_alias_arg.str),
        opt_values_column_list(opt_values_column_list_arg),
        opt_on_duplicate_column_list(opt_on_duplicate_column_list_arg),
        opt_on_duplicate_value_list(opt_on_duplicate_value_list_arg) {
    // REPLACE statement can't have IGNORE flag:
    DBUG_ASSERT(!is_replace || !ignore);
    // REPLACE statement can't have ON DUPLICATE KEY UPDATE clause:
    DBUG_ASSERT(!is_replace || opt_on_duplicate_column_list == nullptr);
    // INSERT/REPLACE ... SELECT can't have VALUES clause:
    DBUG_ASSERT((row_value_list != nullptr) ^
                (insert_query_expression != nullptr));
    // ON DUPLICATE KEY UPDATE: column and value arrays must have same sizes:
    DBUG_ASSERT((opt_on_duplicate_column_list == nullptr &&
                 opt_on_duplicate_value_list == nullptr) ||
                (opt_on_duplicate_column_list->elements() ==
                 opt_on_duplicate_value_list->elements()));
  }

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  bool has_select() const { return insert_query_expression != nullptr; }
};

class PT_call final : public Parse_tree_root {
  sp_name *proc_name;
  PT_item_list *opt_expr_list;

 public:
  PT_call(sp_name *proc_name_arg, PT_item_list *opt_expr_list_arg)
      : proc_name(proc_name_arg), opt_expr_list(opt_expr_list_arg) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

/**
  Top-level node for the SHUTDOWN statement

  @ingroup ptn_stmt
*/
class PT_shutdown final : public Parse_tree_root {
  Sql_cmd_shutdown sql_cmd;

 public:
  Sql_cmd *make_cmd(THD *) override { return &sql_cmd; }
};

/**
  Top-level node for the CREATE [OR REPLACE] SPATIAL REFERENCE SYSTEM statement.

  @ingroup ptn_stmt
*/
class PT_create_srs final : public Parse_tree_root {
  /// The SQL command object.
  Sql_cmd_create_srs sql_cmd;
  /// Whether OR REPLACE is specified.
  bool m_or_replace;
  /// Whether IF NOT EXISTS is specified.
  bool m_if_not_exists;
  /// SRID of the SRS to create.
  ///
  /// The range is larger than that of gis::srid_t, so it must be
  /// verified to be less than the uint32 maximum value.
  unsigned long long m_srid;
  /// All attributes except SRID.
  const Sql_cmd_srs_attributes m_attributes;

  /// Check if a UTF-8 string contains control characters.
  ///
  /// @note This function only checks single byte control characters (U+0000 to
  /// U+001F, and U+007F). There are some control characters at U+0080 to U+00A0
  /// that are not detected by this function.
  ///
  /// @param str The string.
  /// @param length Length of the string.
  ///
  /// @retval false The string contains no control characters.
  /// @retval true The string contains at least one control character.
  bool contains_control_char(char *str, size_t length) {
    for (size_t pos = 0; pos < length; pos++) {
      if (std::iscntrl(str[pos])) return true;
    }
    return false;
  }

 public:
  PT_create_srs(unsigned long long srid,
                const Sql_cmd_srs_attributes &attributes, bool or_replace,
                bool if_not_exists)
      : m_or_replace(or_replace),
        m_if_not_exists(if_not_exists),
        m_srid(srid),
        m_attributes(attributes) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

/**
  Top-level node for the DROP SPATIAL REFERENCE SYSTEM statement.

  @ingroup ptn_stmt
*/
class PT_drop_srs final : public Parse_tree_root {
  /// The SQL command object.
  Sql_cmd_drop_srs sql_cmd;
  /// SRID of the SRS to drop.
  ///
  /// The range is larger than that of gis::srid_t, so it must be
  /// verified to be less than the uint32 maximum value.
  unsigned long long m_srid;

 public:
  PT_drop_srs(unsigned long long srid, bool if_exists)
      : sql_cmd(srid, if_exists), m_srid(srid) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

/**
  Top-level node for the ALTER INSTANCE statement

  @ingroup ptn_stmt
*/
class PT_alter_instance final : public Parse_tree_root {
  Sql_cmd_alter_instance sql_cmd;

 public:
  explicit PT_alter_instance(
      enum alter_instance_action_enum alter_instance_action)
      : sql_cmd(alter_instance_action) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

/**
  A template-free base class for index options that we can predeclare in
  sql_lex.h
*/
class PT_base_index_option : public Table_ddl_node {};

/**
  A key part specification.

  This can either be a "normal" key part (a key part that points to a column),
  or this can be a functional key part (a key part that points to an
  expression).
*/
class PT_key_part_specification : public Parse_tree_node {
  typedef Parse_tree_node super;

 public:
  /**
    Constructor for a functional key part.

    @param expression The expression to index.
    @param order The direction of the index.
  */
  PT_key_part_specification(Item *expression, enum_order order);

  /**
    Constructor for a "normal" key part. That is a key part that points to a
    column and not an expression.

    @param column_name The column name that this key part points to.
    @param order The direction of the index.
    @param prefix_length How many bytes or characters this key part should
           index, or zero if it should index the entire column.
  */
  PT_key_part_specification(const LEX_CSTRING &column_name, enum_order order,
                            int prefix_length);

  /**
    Contextualize this key part specification. This will also call itemize on
    the indexed expression if this is a functional key part.

    @param pc The parse context

    @retval true on error
    @retval false on success
  */
  bool contextualize(Parse_context *pc) override;

  /**
    Get the indexed expression. The caller must ensure that has_expression()
    returns true before calling this.

    @returns The indexed expression
  */
  Item *get_expression() const {
    DBUG_ASSERT(has_expression());
    return m_expression;
  }

  /**
    @returns The direction of the index: ORDER_ASC, ORDER_DESC or
             ORDER_NOT_RELEVANT in case the user didn't explicitly specify a
             direction.
  */
  enum_order get_order() const { return m_order; }

  /**
    @retval true if the user explicitly specified a direction (asc/desc).
    @retval false if the user didn't explicitly specify a direction.
  */
  bool is_explicit() const { return get_order() != ORDER_NOT_RELEVANT; }

  /**
    @retval true if the key part contains an expression (and thus is a
            functional key part).
    @retval false if the key part doesn't contain an expression.
  */
  bool has_expression() const { return m_expression != nullptr; }

  /**
    Get the column that this key part points to. This is only valid if this
    key part isn't a functional index. The caller must thus check the return
    value of has_expression() before calling this function.

    @returns The column that this key part points to.
  */
  LEX_CSTRING get_column_name() const {
    DBUG_ASSERT(!has_expression());
    return m_column_name;
  }

  /**
    @returns The number of bytes that this key part should index. If the column
             this key part points to is a non-binary column, this is the number
             of characters. Returns zero if the entire column should be indexed.
  */
  int get_prefix_length() const { return m_prefix_length; }

 private:
  /**
    The indexed expression in case this is a functional key part. Only valid if
    has_expression() returns true.
  */
  Item *m_expression;

  /// The direction of the index.
  enum_order m_order;

  /// The name of the column that this key part indexes.
  LEX_CSTRING m_column_name;

  /**
    If this is greater than zero, it represents how many bytes of the column
    that is indexed. Note that for non-binary columns (VARCHAR, TEXT etc), this
    is the number of characters.
  */
  int m_prefix_length;
};

/**
  A template for options that set a single `<alter option>` value in
  thd->lex->key_create_info.

  @tparam Option_type The data type of the option.
  @tparam Property Pointer-to-member for the option of KEY_CREATE_INFO.
*/
template <typename Option_type, Option_type KEY_CREATE_INFO::*Property>
class PT_index_option : public PT_base_index_option {
 public:
  /// @param option_value The value of the option.
  PT_index_option(Option_type option_value) : m_option_value(option_value) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    pc->key_create_info->*Property = m_option_value;
    return false;
  }

 private:
  Option_type m_option_value;
};

/**
  A template for options that set a single property in a KEY_CREATE_INFO, and
  also records if the option was explicitly set.
*/
template <typename Option_type, Option_type KEY_CREATE_INFO::*Property,
          bool KEY_CREATE_INFO::*Property_is_explicit>
class PT_traceable_index_option : public PT_base_index_option {
 public:
  PT_traceable_index_option(Option_type option_value)
      : m_option_value(option_value) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    pc->key_create_info->*Property = m_option_value;
    pc->key_create_info->*Property_is_explicit = true;
    return false;
  }

 private:
  Option_type m_option_value;
};

typedef Mem_root_array_YY<PT_base_index_option *> Index_options;
typedef PT_index_option<ulong, &KEY_CREATE_INFO::block_size> PT_block_size;
typedef PT_index_option<LEX_CSTRING, &KEY_CREATE_INFO::comment>
    PT_index_comment;
typedef PT_index_option<LEX_CSTRING, &KEY_CREATE_INFO::parser_name>
    PT_fulltext_index_parser_name;
typedef PT_index_option<bool, &KEY_CREATE_INFO::is_visible> PT_index_visibility;

/**
  The data structure (B-tree, Hash, etc) used for an index is called
  'index_type' in the manual. Internally, this is stored in
  KEY_CREATE_INFO::algorithm, while what the manual calls 'algorithm' is
  stored in partition_info::key_algorithm. In an `<create_index_stmt>`
  it's ignored. The terminology is somewhat confusing, but we stick to the
  manual in the parser.
*/
typedef PT_traceable_index_option<ha_key_alg, &KEY_CREATE_INFO::algorithm,
                                  &KEY_CREATE_INFO::is_algorithm_explicit>
    PT_index_type;

class PT_create_index_stmt final : public PT_table_ddl_stmt_base {
 public:
  PT_create_index_stmt(MEM_ROOT *mem_root, keytype type_par,
                       const LEX_STRING &name_arg, PT_base_index_option *type,
                       Table_ident *table_ident,
                       List<PT_key_part_specification> *cols,
                       Index_options options,
                       Alter_info::enum_alter_table_algorithm algo,
                       Alter_info::enum_alter_table_lock lock)
      : PT_table_ddl_stmt_base(mem_root),
        m_keytype(type_par),
        m_name(name_arg),
        m_type(type),
        m_table_ident(table_ident),
        m_columns(cols),
        m_options(options),
        m_algo(algo),
        m_lock(lock) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  keytype m_keytype;
  LEX_STRING m_name;
  PT_base_index_option *m_type;
  Table_ident *m_table_ident;
  List<PT_key_part_specification> *m_columns;
  Index_options m_options;
  const Alter_info::enum_alter_table_algorithm m_algo;
  const Alter_info::enum_alter_table_lock m_lock;
};

/**
  Base class for column/constraint definitions in CREATE %TABLE

  @ingroup ptn_create_table_stuff
*/
class PT_table_element : public Table_ddl_node {};

class PT_table_constraint_def : public PT_table_element {};

class PT_inline_index_definition : public PT_table_constraint_def {
  typedef PT_table_constraint_def super;

 public:
  PT_inline_index_definition(keytype type_par, const LEX_STRING &name_arg,
                             PT_base_index_option *type,
                             List<PT_key_part_specification> *cols,
                             Index_options options)
      : m_keytype(type_par),
        m_name(name_arg),
        m_type(type),
        m_columns(cols),
        m_options(options) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

 private:
  keytype m_keytype;
  const LEX_STRING m_name;
  PT_base_index_option *m_type;
  List<PT_key_part_specification> *m_columns;
  Index_options m_options;
};

class PT_foreign_key_definition : public PT_table_constraint_def {
  typedef PT_table_constraint_def super;

 public:
  PT_foreign_key_definition(const LEX_STRING &constraint_name,
                            const LEX_STRING &key_name,
                            List<PT_key_part_specification> *columns,
                            Table_ident *referenced_table,
                            List<Key_part_spec> *ref_list,
                            fk_match_opt fk_match_option,
                            fk_option fk_update_opt, fk_option fk_delete_opt)
      : m_constraint_name(constraint_name),
        m_key_name(key_name),
        m_columns(columns),
        m_referenced_table(referenced_table),
        m_ref_list(ref_list),
        m_fk_match_option(fk_match_option),
        m_fk_update_opt(fk_update_opt),
        m_fk_delete_opt(fk_delete_opt) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

 private:
  const LEX_STRING m_constraint_name;
  const LEX_STRING m_key_name;
  List<PT_key_part_specification> *m_columns;
  Table_ident *m_referenced_table;
  List<Key_part_spec> *m_ref_list;
  fk_match_opt m_fk_match_option;
  fk_option m_fk_update_opt;
  fk_option m_fk_delete_opt;
};

/**
  Common base class for CREATE TABLE and ALTER TABLE option nodes

  @ingroup ptn_create_or_alter_table_options
*/
class PT_ddl_table_option : public Table_ddl_node {
 public:
  ~PT_ddl_table_option() override = 0;  // Force abstract class declaration

  virtual bool is_rename_table() const { return false; }
};

inline PT_ddl_table_option::~PT_ddl_table_option() {}

/**
  Base class for CREATE TABLE option nodes

  @ingroup ptn_create_or_alter_table_options
*/
class PT_create_table_option : public PT_ddl_table_option {
  typedef PT_ddl_table_option super;

 public:
  ~PT_create_table_option() override = 0;  // Force abstract class declaration

  bool contextualize(Table_ddl_parse_context *pc) override {
    if (super::contextualize(pc)) return true;
    pc->alter_info->flags |= Alter_info::ALTER_OPTIONS;
    return false;
  }
};

inline PT_create_table_option::~PT_create_table_option() {}

/**
  A template for options that set a single property in HA_CREATE_INFO, and
  also records if the option was explicitly set.
*/
template <typename Option_type, Option_type HA_CREATE_INFO::*Property,
          ulong Property_flag>
class PT_traceable_create_table_option : public PT_create_table_option {
  typedef PT_create_table_option super;

  const Option_type value;

 public:
  explicit PT_traceable_create_table_option(Option_type value) : value(value) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    if (super::contextualize(pc)) return true;
    pc->create_info->*Property = value;
    pc->create_info->used_fields |= Property_flag;
    return false;
  }
};

#define TYPE_AND_REF(x) decltype(x), &x

/**
  Node for the @SQL{MAX_ROWS [=] @B{@<integer@>}} table option

  @ingroup ptn_create_or_alter_table_options
*/
typedef PT_traceable_create_table_option<TYPE_AND_REF(HA_CREATE_INFO::max_rows),
                                         HA_CREATE_USED_MAX_ROWS>
    PT_create_max_rows_option;

/**
  Node for the @SQL{MIN_ROWS [=] @B{@<integer@>}} table option

  @ingroup ptn_create_or_alter_table_options
*/
typedef PT_traceable_create_table_option<TYPE_AND_REF(HA_CREATE_INFO::min_rows),
                                         HA_CREATE_USED_MIN_ROWS>
    PT_create_min_rows_option;

/**
  Node for the @SQL{AVG_ROW_LENGTH_ROWS [=] @B{@<integer@>}} table option

  @ingroup ptn_create_or_alter_table_options
*/
typedef PT_traceable_create_table_option<
    TYPE_AND_REF(HA_CREATE_INFO::avg_row_length), HA_CREATE_USED_AVG_ROW_LENGTH>
    PT_create_avg_row_length_option;

/**
  Node for the @SQL{PASSWORD [=] @B{@<string@>}} table option

  @ingroup ptn_create_or_alter_table_options
*/
typedef PT_traceable_create_table_option<TYPE_AND_REF(HA_CREATE_INFO::password),
                                         HA_CREATE_USED_PASSWORD>
    PT_create_password_option;

/**
  Node for the @SQL{COMMENT [=] @B{@<string@>}} table option

  @ingroup ptn_create_or_alter_table_options
*/
typedef PT_traceable_create_table_option<TYPE_AND_REF(HA_CREATE_INFO::comment),
                                         HA_CREATE_USED_COMMENT>
    PT_create_commen_option;

/**
  Node for the @SQL{COMPRESSION [=] @B{@<string@>}} table option

  @ingroup ptn_create_or_alter_table_options
*/
typedef PT_traceable_create_table_option<TYPE_AND_REF(HA_CREATE_INFO::compress),
                                         HA_CREATE_USED_COMPRESS>
    PT_create_compress_option;

/**
  Node for the @SQL{ENGRYPTION [=] @B{@<string@>}} table option

  @ingroup ptn_create_or_alter_table_options
*/
typedef PT_traceable_create_table_option<
    TYPE_AND_REF(HA_CREATE_INFO::encrypt_type), HA_CREATE_USED_ENCRYPT>
    PT_create_encryption_option;

/**
  Node for the @SQL{AUTO_INCREMENT [=] @B{@<integer@>}} table option

  @ingroup ptn_create_or_alter_table_options
*/
typedef PT_traceable_create_table_option<
    TYPE_AND_REF(HA_CREATE_INFO::auto_increment_value), HA_CREATE_USED_AUTO>
    PT_create_auto_increment_option;

typedef PT_traceable_create_table_option<TYPE_AND_REF(HA_CREATE_INFO::row_type),
                                         HA_CREATE_USED_ROW_FORMAT>
    PT_create_row_format_option;

typedef PT_traceable_create_table_option<
    TYPE_AND_REF(HA_CREATE_INFO::merge_insert_method),
    HA_CREATE_USED_INSERT_METHOD>
    PT_create_insert_method_option;

typedef PT_traceable_create_table_option<
    TYPE_AND_REF(HA_CREATE_INFO::data_file_name), HA_CREATE_USED_DATADIR>
    PT_create_data_directory_option;

typedef PT_traceable_create_table_option<
    TYPE_AND_REF(HA_CREATE_INFO::index_file_name), HA_CREATE_USED_INDEXDIR>
    PT_create_index_directory_option;

typedef PT_traceable_create_table_option<
    TYPE_AND_REF(HA_CREATE_INFO::tablespace), HA_CREATE_USED_TABLESPACE>
    PT_create_tablespace_option;

typedef PT_traceable_create_table_option<
    TYPE_AND_REF(HA_CREATE_INFO::connect_string), HA_CREATE_USED_CONNECTION>
    PT_create_connection_option;

typedef PT_traceable_create_table_option<
    TYPE_AND_REF(HA_CREATE_INFO::key_block_size), HA_CREATE_USED_KEY_BLOCK_SIZE>
    PT_create_key_block_size_option;

typedef decltype(HA_CREATE_INFO::table_options) table_options_t;

/**
  A template for options that set HA_CREATE_INFO::table_options and
  also records if the option was explicitly set.
*/
template <ulong Property_flag, table_options_t Default, table_options_t Yes,
          table_options_t No>
class PT_ternary_create_table_option : public PT_create_table_option {
  typedef PT_create_table_option super;

  const Ternary_option value;

 public:
  explicit PT_ternary_create_table_option(Ternary_option value)
      : value(value) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    if (super::contextualize(pc)) return true;
    pc->create_info->table_options &= ~(Yes | No);
    switch (value) {
      case Ternary_option::ON:
        pc->create_info->table_options |= Yes;
        break;
      case Ternary_option::OFF:
        pc->create_info->table_options |= No;
        break;
      case Ternary_option::DEFAULT:
        break;
      default:
        DBUG_ASSERT(false);
    }
    pc->create_info->used_fields |= Property_flag;
    return false;
  }
};

/**
  Node for the @SQL{PACK_KEYS [=] @B{1|0|DEFAULT}} table option

  @ingroup ptn_create_or_alter_table_options

  PACK_KEYS | Constructor parameter
  ----------|----------------------
  1         | Ternary_option::ON
  0         | Ternary_option::OFF
  DEFAULT   | Ternary_option::DEFAULT
*/
typedef PT_ternary_create_table_option<HA_CREATE_USED_PACK_KEYS,  // flag
                                       0,                         // DEFAULT
                                       HA_OPTION_PACK_KEYS,       // ON
                                       HA_OPTION_NO_PACK_KEYS>    // OFF
    PT_create_pack_keys_option;

/**
  Node for the @SQL{STATS_PERSISTENT [=] @B{1|0|DEFAULT}} table option

  @ingroup ptn_create_or_alter_table_options

  STATS_PERSISTENT | Constructor parameter
  -----------------|----------------------
  1                | Ternary_option::ON
  0                | Ternary_option::OFF
  DEFAULT          | Ternary_option::DEFAULT
*/
typedef PT_ternary_create_table_option<HA_CREATE_USED_STATS_PERSISTENT,  // flag
                                       0,                           // DEFAULT
                                       HA_OPTION_STATS_PERSISTENT,  // ON
                                       HA_OPTION_NO_STATS_PERSISTENT>  // OFF
    PT_create_stats_persistent_option;

/**
  A template for options that set HA_CREATE_INFO::table_options and
  also records if the option was explicitly set.
*/
template <ulong Property_flag, table_options_t Yes, table_options_t No>
class PT_bool_create_table_option : public PT_create_table_option {
  typedef PT_create_table_option super;

  const bool value;

 public:
  explicit PT_bool_create_table_option(bool value) : value(value) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    if (super::contextualize(pc)) return true;
    pc->create_info->table_options &= ~(Yes | No);
    pc->create_info->table_options |= value ? Yes : No;
    pc->create_info->used_fields |= Property_flag;
    return false;
  }
};

/**
  Node for the @SQL{CHECKSUM|TABLE_CHECKSUM [=] @B{0|@<not 0@>}} table option

  @ingroup ptn_create_or_alter_table_options

  TABLE_CHECKSUM | Constructor parameter
  ---------------|----------------------
  0              | false
  not 0          | true
*/
typedef PT_bool_create_table_option<HA_CREATE_USED_CHECKSUM,  // flag
                                    HA_OPTION_CHECKSUM,       // ON
                                    HA_OPTION_NO_CHECKSUM     // OFF
                                    >
    PT_create_checksum_option;

/**
  Node for the @SQL{DELAY_KEY_WRITE [=] @B{0|@<not 0@>}} table option

  @ingroup ptn_create_or_alter_table_options

  TABLE_CHECKSUM | Constructor parameter
  ---------------|----------------------
  0              | false
  not 0          | true
*/
typedef PT_bool_create_table_option<HA_CREATE_USED_DELAY_KEY_WRITE,  // flag
                                    HA_OPTION_DELAY_KEY_WRITE,       // ON
                                    HA_OPTION_NO_DELAY_KEY_WRITE>    // OFF
    PT_create_delay_key_write_option;

/**
  Node for the @SQL{ENGINE [=] @B{@<identifier@>|@<string@>}} table option

  @ingroup ptn_create_or_alter_table_options
*/
class PT_create_table_engine_option : public PT_create_table_option {
  typedef PT_create_table_option super;

  const LEX_CSTRING engine;

 public:
  /**
    @param engine       Storage engine name.
  */
  explicit PT_create_table_engine_option(const LEX_CSTRING &engine)
      : engine(engine) {}

  bool contextualize(Table_ddl_parse_context *pc) override;
};

/**
  Node for the @SQL{SECONDARY_ENGINE [=] @B{@<identifier@>|@<string@>|NULL}}
  table option.

  @ingroup ptn_create_or_alter_table_options
*/
class PT_create_table_secondary_engine_option : public PT_create_table_option {
  using super = PT_create_table_option;

 public:
  explicit PT_create_table_secondary_engine_option() {}
  explicit PT_create_table_secondary_engine_option(
      const LEX_CSTRING &secondary_engine)
      : m_secondary_engine(secondary_engine) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

 private:
  const LEX_CSTRING m_secondary_engine{nullptr, 0};
};

/**
  Node for the @SQL{STATS_AUTO_RECALC [=] @B{@<0|1|DEFAULT@>})} table option

  @ingroup ptn_create_or_alter_table_options
*/
class PT_create_stats_auto_recalc_option : public PT_create_table_option {
  typedef PT_create_table_option super;

  const Ternary_option value;

 public:
  /**
    @param value
      STATS_AUTO_RECALC | value
      ------------------|----------------------
      1                 | Ternary_option::ON
      0                 | Ternary_option::OFF
      DEFAULT           | Ternary_option::DEFAULT
  */
  PT_create_stats_auto_recalc_option(Ternary_option value) : value(value) {}

  bool contextualize(Table_ddl_parse_context *pc) override;
};

/**
  Node for the @SQL{STATS_SAMPLE_PAGES [=] @B{@<integer@>|DEFAULT}} table option

  @ingroup ptn_create_or_alter_table_options
*/
class PT_create_stats_stable_pages : public PT_create_table_option {
  typedef PT_create_table_option super;
  typedef decltype(HA_CREATE_INFO::stats_sample_pages) value_t;

  const value_t value;

 public:
  /**
    Constructor for implicit number of pages

    @param value       Nunber of pages, 1@<=N@<=65535.
  */
  explicit PT_create_stats_stable_pages(value_t value) : value(value) {
    DBUG_ASSERT(value != 0 && value <= 0xFFFF);
  }
  /**
    Constructor for the DEFAULT number of pages
  */
  PT_create_stats_stable_pages() : value(0) {}  // DEFAULT

  bool contextualize(Table_ddl_parse_context *pc) override;
};

class PT_create_union_option : public PT_create_table_option {
  typedef PT_create_table_option super;

  const Mem_root_array<Table_ident *> *tables;

 public:
  explicit PT_create_union_option(const Mem_root_array<Table_ident *> *tables)
      : tables(tables) {}

  bool contextualize(Table_ddl_parse_context *pc) override;
};

class PT_create_storage_option : public PT_create_table_option {
  typedef PT_create_table_option super;

  const ha_storage_media value;

 public:
  explicit PT_create_storage_option(ha_storage_media value) : value(value) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    if (super::contextualize(pc)) return true;
    pc->create_info->storage_media = value;
    return false;
  }
};

class PT_create_table_default_charset : public PT_create_table_option {
  typedef PT_create_table_option super;

  const CHARSET_INFO *value;

 public:
  explicit PT_create_table_default_charset(const CHARSET_INFO *value)
      : value(value) {
    DBUG_ASSERT(value != nullptr);
  }

  bool contextualize(Table_ddl_parse_context *pc) override;
};

class PT_create_table_default_collation : public PT_create_table_option {
  typedef PT_create_table_option super;

  const CHARSET_INFO *value;

 public:
  explicit PT_create_table_default_collation(const CHARSET_INFO *value)
      : value(value) {
    DBUG_ASSERT(value != nullptr);
  }

  bool contextualize(Table_ddl_parse_context *pc) override;
};

class PT_check_constraint final : public PT_table_constraint_def {
  typedef PT_table_constraint_def super;
  Sql_check_constraint_spec cc_spec;

 public:
  explicit PT_check_constraint(LEX_STRING &name, Item *expr, bool is_enforced) {
    cc_spec.name = name;
    cc_spec.check_expr = expr;
    cc_spec.is_enforced = is_enforced;
  }
  void set_column_name(const LEX_STRING &name) { cc_spec.column_name = name; }

  bool contextualize(Table_ddl_parse_context *pc) override;
};

class PT_column_def : public PT_table_element {
  typedef PT_table_element super;

  const LEX_STRING field_ident;
  PT_field_def_base *field_def;
  // Currently we ignore that constraint in the executor.
  PT_table_constraint_def *opt_column_constraint;

  const char *opt_place;

 public:
  PT_column_def(const LEX_STRING &field_ident, PT_field_def_base *field_def,
                PT_table_constraint_def *opt_column_constraint,
                const char *opt_place = nullptr)
      : field_ident(field_ident),
        field_def(field_def),
        opt_column_constraint(opt_column_constraint),
        opt_place(opt_place) {}

  bool contextualize(Table_ddl_parse_context *pc) override;
};

/**
  Top-level node for the CREATE %TABLE statement

  @ingroup ptn_create_table
*/
class PT_create_table_stmt final : public PT_table_ddl_stmt_base {
  bool is_temporary;
  bool only_if_not_exists;
  Table_ident *table_name;
  const Mem_root_array<PT_table_element *> *opt_table_element_list;
  const Mem_root_array<PT_create_table_option *> *opt_create_table_options;
  PT_partition *opt_partitioning;
  On_duplicate on_duplicate;
  PT_query_primary *opt_query_expression;
  Table_ident *opt_like_clause;

  HA_CREATE_INFO m_create_info;

 public:
  /**
    @param mem_root                   MEM_ROOT to use for allocation
    @param is_temporary               True if @SQL{CREATE @B{TEMPORARY} %TABLE}
    @param only_if_not_exists  True if @SQL{CREATE %TABLE ... @B{IF NOT EXISTS}}
    @param table_name                 @SQL{CREATE %TABLE ... @B{@<table name@>}}
    @param opt_table_element_list     NULL or a list of table column and
                                      constraint definitions.
    @param opt_create_table_options   NULL or a list of
                                      @ref ptn_create_or_alter_table_options
                                      "table options".
    @param opt_partitioning           NULL or the @SQL{PARTITION BY} clause.
    @param on_duplicate               DUPLICATE, IGNORE or fail with an error
                                      on data duplication errors (relevant
                                      for @SQL{CREATE TABLE ... SELECT}
                                      statements).
    @param opt_query_expression       NULL or the @SQL{@B{SELECT}} clause.
  */
  PT_create_table_stmt(
      MEM_ROOT *mem_root, bool is_temporary, bool only_if_not_exists,
      Table_ident *table_name,
      const Mem_root_array<PT_table_element *> *opt_table_element_list,
      const Mem_root_array<PT_create_table_option *> *opt_create_table_options,
      PT_partition *opt_partitioning, On_duplicate on_duplicate,
      PT_query_primary *opt_query_expression)
      : PT_table_ddl_stmt_base(mem_root),
        is_temporary(is_temporary),
        only_if_not_exists(only_if_not_exists),
        table_name(table_name),
        opt_table_element_list(opt_table_element_list),
        opt_create_table_options(opt_create_table_options),
        opt_partitioning(opt_partitioning),
        on_duplicate(on_duplicate),
        opt_query_expression(opt_query_expression),
        opt_like_clause(nullptr) {}
  /**
    @param mem_root           MEM_ROOT to use for allocation
    @param is_temporary       True if @SQL{CREATE @B{TEMPORARY} %TABLE}.
    @param only_if_not_exists True if @SQL{CREATE %TABLE ... @B{IF NOT EXISTS}}.
    @param table_name         @SQL{CREATE %TABLE ... @B{@<table name@>}}.
    @param opt_like_clause    NULL or the @SQL{@B{LIKE @<table name@>}} clause.
  */
  PT_create_table_stmt(MEM_ROOT *mem_root, bool is_temporary,
                       bool only_if_not_exists, Table_ident *table_name,
                       Table_ident *opt_like_clause)
      : PT_table_ddl_stmt_base(mem_root),
        is_temporary(is_temporary),
        only_if_not_exists(only_if_not_exists),
        table_name(table_name),
        opt_table_element_list(nullptr),
        opt_create_table_options(nullptr),
        opt_partitioning(nullptr),
        on_duplicate(On_duplicate::ERROR),
        opt_query_expression(nullptr),
        opt_like_clause(opt_like_clause) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

class PT_create_role final : public Parse_tree_root {
  Sql_cmd_create_role sql_cmd;

 public:
  PT_create_role(bool if_not_exists, const List<LEX_USER> *roles)
      : sql_cmd(if_not_exists, roles) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

class PT_drop_role final : public Parse_tree_root {
  Sql_cmd_drop_role sql_cmd;

 public:
  explicit PT_drop_role(bool ignore_errors, const List<LEX_USER> *roles)
      : sql_cmd(ignore_errors, roles) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

class PT_set_role : public Parse_tree_root {
  Sql_cmd_set_role sql_cmd;

 public:
  explicit PT_set_role(role_enum role_type,
                       const List<LEX_USER> *opt_except_roles = nullptr)
      : sql_cmd(role_type, opt_except_roles) {
    DBUG_ASSERT(role_type == role_enum::ROLE_ALL ||
                opt_except_roles == nullptr);
  }
  explicit PT_set_role(const List<LEX_USER> *roles) : sql_cmd(roles) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

/**
  This class is used for representing both static and dynamic privileges on
  global as well as table and column level.
*/
struct Privilege {
  enum privilege_type { STATIC, DYNAMIC };

  privilege_type type;
  const Mem_root_array<LEX_CSTRING> *columns;

  explicit Privilege(privilege_type type,
                     const Mem_root_array<LEX_CSTRING> *columns)
      : type(type), columns(columns) {}
};

struct Static_privilege : public Privilege {
  const uint grant;

  Static_privilege(uint grant, const Mem_root_array<LEX_CSTRING> *columns_arg)
      : Privilege(STATIC, columns_arg), grant(grant) {}
};

struct Dynamic_privilege : public Privilege {
  const LEX_STRING ident;

  Dynamic_privilege(const LEX_STRING &ident,
                    const Mem_root_array<LEX_CSTRING> *columns_arg)
      : Privilege(DYNAMIC, columns_arg), ident(ident) {}
};

class PT_role_or_privilege : public Parse_tree_node {
 private:
  POS pos;

 public:
  explicit PT_role_or_privilege(const POS &pos) : pos(pos) {}
  virtual LEX_USER *get_user(THD *thd);
  virtual Privilege *get_privilege(THD *thd);
};

class PT_role_at_host final : public PT_role_or_privilege {
  LEX_STRING role;
  LEX_STRING host;

 public:
  PT_role_at_host(const POS &pos, const LEX_STRING &role,
                  const LEX_STRING &host)
      : PT_role_or_privilege(pos), role(role), host(host) {}

  LEX_USER *get_user(THD *thd) override;
};

class PT_role_or_dynamic_privilege final : public PT_role_or_privilege {
  LEX_STRING ident;

 public:
  PT_role_or_dynamic_privilege(const POS &pos, const LEX_STRING &ident)
      : PT_role_or_privilege(pos), ident(ident) {}

  LEX_USER *get_user(THD *thd) override;
  Privilege *get_privilege(THD *thd) override;
};

class PT_static_privilege final : public PT_role_or_privilege {
  const uint grant;
  const Mem_root_array<LEX_CSTRING> *columns;

 public:
  PT_static_privilege(const POS &pos, uint grant,
                      const Mem_root_array<LEX_CSTRING> *columns = nullptr)
      : PT_role_or_privilege(pos), grant(grant), columns(columns) {}

  Privilege *get_privilege(THD *thd) override;
};

class PT_dynamic_privilege final : public PT_role_or_privilege {
  LEX_STRING ident;

 public:
  PT_dynamic_privilege(const POS &pos, const LEX_STRING &ident)
      : PT_role_or_privilege(pos), ident(ident) {}

  Privilege *get_privilege(THD *thd) override;
};

class PT_grant_roles final : public Parse_tree_root {
  const Mem_root_array<PT_role_or_privilege *> *roles;
  const List<LEX_USER> *users;
  const bool with_admin_option;

 public:
  PT_grant_roles(const Mem_root_array<PT_role_or_privilege *> *roles,
                 const List<LEX_USER> *users, bool with_admin_option)
      : roles(roles), users(users), with_admin_option(with_admin_option) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

class PT_revoke_roles final : public Parse_tree_root {
  const Mem_root_array<PT_role_or_privilege *> *roles;
  const List<LEX_USER> *users;

 public:
  PT_revoke_roles(Mem_root_array<PT_role_or_privilege *> *roles,
                  const List<LEX_USER> *users)
      : roles(roles), users(users) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

class PT_alter_user_default_role final : public Parse_tree_root {
  Sql_cmd_alter_user_default_role sql_cmd;

 public:
  PT_alter_user_default_role(bool if_exists, const List<LEX_USER> *users,
                             const List<LEX_USER> *roles,
                             const role_enum role_type)
      : sql_cmd(if_exists, users, roles, role_type) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

class PT_show_grants final : public Parse_tree_root {
  Sql_cmd_show_grants sql_cmd;

 public:
  PT_show_grants(const LEX_USER *opt_for_user,
                 const List<LEX_USER> *opt_using_users)
      : sql_cmd(opt_for_user, opt_using_users) {
    DBUG_ASSERT(opt_using_users == nullptr || opt_for_user != nullptr);
  }

  Sql_cmd *make_cmd(THD *thd) override;
};

/**
  Base class for Parse tree nodes of SHOW FIELDS/SHOW INDEX statements.
*/
class PT_show_fields_and_keys : public Parse_tree_root {
 protected:
  enum Type { SHOW_FIELDS = SQLCOM_SHOW_FIELDS, SHOW_KEYS = SQLCOM_SHOW_KEYS };

  PT_show_fields_and_keys(const POS &pos, Type type, Table_ident *table_ident,
                          const LEX_STRING &wild, Item *where_condition)
      : m_sql_cmd(static_cast<enum_sql_command>(type)),
        m_pos(pos),
        m_type(type),
        m_table_ident(table_ident),
        m_wild(wild),
        m_where_condition(where_condition) {
    DBUG_ASSERT(wild.str == nullptr || where_condition == nullptr);
  }

 public:
  Sql_cmd *make_cmd(THD *thd) override;

 private:
  // Sql_cmd for SHOW COLUMNS/SHOW INDEX statements.
  Sql_cmd_show m_sql_cmd;

  // Textual location of a token just parsed.
  POS m_pos;

  // SHOW_FIELDS or SHOW_KEYS
  Type m_type;

  // Table used in the statement.
  Table_ident *m_table_ident;

  // Wild or where clause used in the statement.
  LEX_STRING m_wild;
  Item *m_where_condition;
};

/**
  Parse tree node for SHOW FIELDS statement.
*/
class PT_show_fields final : public PT_show_fields_and_keys {
  typedef PT_show_fields_and_keys super;

 public:
  PT_show_fields(const POS &pos, Show_cmd_type show_cmd_type,
                 Table_ident *table, const LEX_STRING &wild)
      : PT_show_fields_and_keys(pos, SHOW_FIELDS, table, wild, nullptr),
        m_show_cmd_type(show_cmd_type) {}

  PT_show_fields(const POS &pos, Show_cmd_type show_cmd_type,
                 Table_ident *table_ident, Item *where_condition = nullptr);

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  Show_cmd_type m_show_cmd_type;
};

/**
  Parse tree node for SHOW INDEX statement.
*/
class PT_show_keys final : public PT_show_fields_and_keys {
 public:
  PT_show_keys(const POS &pos, bool extended_show, Table_ident *table,
               Item *where_condition);

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  typedef PT_show_fields_and_keys super;

  // Flag to indicate EXTENDED keyword usage in the statement.
  bool m_extended_show;
};

class PT_alter_table_action : public PT_ddl_table_option {
  typedef PT_ddl_table_option super;

 protected:
  explicit PT_alter_table_action(Alter_info::Alter_info_flag flag)
      : flag(flag) {}

 public:
  bool contextualize(Table_ddl_parse_context *pc) override;

 protected:
  /**
    A routine used by the parser to decide whether we are specifying a full
    partitioning or if only partitions to add or to reorganize.

    @retval  true    ALTER TABLE ADD/REORGANIZE PARTITION.
    @retval  false   Something else.
  */
  bool is_add_or_reorganize_partition() const {
    return (flag == Alter_info::ALTER_ADD_PARTITION ||
            flag == Alter_info::ALTER_REORGANIZE_PARTITION);
  }

 public:
  const Alter_info::Alter_info_flag flag;
};

class PT_alter_table_add_column final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  PT_alter_table_add_column(const LEX_STRING &field_ident,
                            PT_field_def_base *field_def,
                            PT_table_constraint_def *opt_column_constraint,
                            const char *opt_place)
      : super(Alter_info::ALTER_ADD_COLUMN),
        m_column_def(field_ident, field_def, opt_column_constraint, opt_place) {
  }

  bool contextualize(Table_ddl_parse_context *pc) override {
    return super::contextualize(pc) || m_column_def.contextualize(pc);
  }

 private:
  PT_column_def m_column_def;
};

class PT_alter_table_add_columns final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  explicit PT_alter_table_add_columns(
      const Mem_root_array<PT_table_element *> *columns)
      : super(Alter_info::ALTER_ADD_COLUMN), m_columns(columns) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    if (super::contextualize(pc)) return true;

    for (auto *column : *m_columns)
      if (column->contextualize(pc)) return true;

    return false;
  }

 private:
  const Mem_root_array<PT_table_element *> *m_columns;
};

class PT_alter_table_add_constraint final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  explicit PT_alter_table_add_constraint(PT_table_constraint_def *constraint)
      : super(Alter_info::ALTER_ADD_INDEX), m_constraint(constraint) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    return super::contextualize(pc) || m_constraint->contextualize(pc);
  }

 private:
  PT_table_constraint_def *m_constraint;
};

class PT_alter_table_change_column final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  PT_alter_table_change_column(const LEX_STRING &old_name,
                               const LEX_STRING &new_name,
                               PT_field_def_base *field_def,
                               const char *opt_place)
      : super(Alter_info::ALTER_CHANGE_COLUMN),
        m_old_name(old_name),
        m_new_name(new_name),
        m_field_def(field_def),
        m_opt_place(opt_place) {}

  PT_alter_table_change_column(const LEX_STRING &name,
                               PT_field_def_base *field_def,
                               const char *opt_place)
      : PT_alter_table_change_column(name, name, field_def, opt_place) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

 private:
  const LEX_STRING m_old_name;
  const LEX_STRING m_new_name;
  PT_field_def_base *m_field_def;
  const char *m_opt_place;
};

class PT_alter_table_drop : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 protected:
  PT_alter_table_drop(Alter_drop::drop_type drop_type,
                      Alter_info::Alter_info_flag alter_info_flag,
                      const char *name)
      : super(alter_info_flag), m_alter_drop(drop_type, name) {}

 public:
  bool contextualize(Table_ddl_parse_context *pc) override {
    return (super::contextualize(pc) ||
            pc->alter_info->drop_list.push_back(&m_alter_drop));
  }

 private:
  Alter_drop m_alter_drop;
};

class PT_alter_table_drop_column final : public PT_alter_table_drop {
 public:
  explicit PT_alter_table_drop_column(const char *name)
      : PT_alter_table_drop(Alter_drop::COLUMN, Alter_info::ALTER_DROP_COLUMN,
                            name) {}
};

class PT_alter_table_drop_foreign_key final : public PT_alter_table_drop {
 public:
  explicit PT_alter_table_drop_foreign_key(const char *name)
      : PT_alter_table_drop(Alter_drop::FOREIGN_KEY,
                            Alter_info::DROP_FOREIGN_KEY, name) {}
};

class PT_alter_table_drop_key final : public PT_alter_table_drop {
 public:
  explicit PT_alter_table_drop_key(const char *name)
      : PT_alter_table_drop(Alter_drop::KEY, Alter_info::ALTER_DROP_INDEX,
                            name) {}
};

class PT_alter_table_drop_check_constraint final : public PT_alter_table_drop {
 public:
  explicit PT_alter_table_drop_check_constraint(const char *name)
      : PT_alter_table_drop(Alter_drop::CHECK_CONSTRAINT,
                            Alter_info::DROP_CHECK_CONSTRAINT, name) {}
};

class PT_alter_table_drop_constraint final : public PT_alter_table_drop {
 public:
  explicit PT_alter_table_drop_constraint(const char *name)
      : PT_alter_table_drop(Alter_drop::ANY_CONSTRAINT,
                            Alter_info::DROP_ANY_CONSTRAINT, name) {}
};

class PT_alter_table_enforce_constraint : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 protected:
  PT_alter_table_enforce_constraint(
      Alter_constraint_enforcement::Type alter_type,
      Alter_info::Alter_info_flag alter_info_flag, const char *name,
      bool is_enforced)
      : super(alter_info_flag),
        m_constraint_enforcement(alter_type, name, is_enforced) {}

 public:
  explicit PT_alter_table_enforce_constraint(const char *name, bool is_enforced)
      : super(is_enforced ? Alter_info::ENFORCE_ANY_CONSTRAINT
                          : Alter_info::SUSPEND_ANY_CONSTRAINT),
        m_constraint_enforcement(
            Alter_constraint_enforcement::Type::ANY_CONSTRAINT, name,
            is_enforced) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    return (super::contextualize(pc) ||
            pc->alter_info->alter_constraint_enforcement_list.push_back(
                &m_constraint_enforcement));
  }

 private:
  Alter_constraint_enforcement m_constraint_enforcement;
};

class PT_alter_table_enforce_check_constraint final
    : public PT_alter_table_enforce_constraint {
 public:
  explicit PT_alter_table_enforce_check_constraint(const char *name,
                                                   bool is_enforced)
      : PT_alter_table_enforce_constraint(
            Alter_constraint_enforcement::Type::CHECK_CONSTRAINT,
            is_enforced ? Alter_info::ENFORCE_CHECK_CONSTRAINT
                        : Alter_info::SUSPEND_CHECK_CONSTRAINT,
            name, is_enforced) {}
};

class PT_alter_table_enable_keys final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  explicit PT_alter_table_enable_keys(bool enable)
      : super(Alter_info::ALTER_KEYS_ONOFF), m_enable(enable) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    pc->alter_info->keys_onoff =
        m_enable ? Alter_info::ENABLE : Alter_info::DISABLE;
    return super::contextualize(pc);
  }

 private:
  bool m_enable;
};

class PT_alter_table_set_default final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  PT_alter_table_set_default(const char *col_name, Item *opt_default_expr)
      : super(Alter_info::ALTER_CHANGE_COLUMN_DEFAULT),
        m_name(col_name),
        m_expr(opt_default_expr) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

 private:
  const char *m_name;
  Item *m_expr;
};

class PT_alter_table_index_visible final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  PT_alter_table_index_visible(const char *name, bool visible)
      : super(Alter_info::ALTER_INDEX_VISIBILITY),
        m_alter_index_visibility(name, visible) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    return (super::contextualize(pc) ||
            pc->alter_info->alter_index_visibility_list.push_back(
                &m_alter_index_visibility));
  }

 private:
  Alter_index_visibility m_alter_index_visibility;
};

class PT_alter_table_rename final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  explicit PT_alter_table_rename(const Table_ident *ident)
      : super(Alter_info::ALTER_RENAME), m_ident(ident) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

  bool is_rename_table() const override { return true; }

 private:
  const Table_ident *const m_ident;
};

class PT_alter_table_rename_key final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  PT_alter_table_rename_key(const char *from, const char *to)
      : super(Alter_info::ALTER_RENAME_INDEX), m_rename_key(from, to) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    return super::contextualize(pc) ||
           pc->alter_info->alter_rename_key_list.push_back(&m_rename_key);
  }

 private:
  Alter_rename_key m_rename_key;
};

class PT_alter_table_rename_column final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  PT_alter_table_rename_column(const char *from, const char *to)
      : super(Alter_info::ALTER_CHANGE_COLUMN), m_rename_column(from, to) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    return super::contextualize(pc) ||
           pc->alter_info->alter_list.push_back(&m_rename_column);
  }

 private:
  Alter_column m_rename_column;
};

class PT_alter_table_convert_to_charset final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  PT_alter_table_convert_to_charset(const CHARSET_INFO *charset,
                                    const CHARSET_INFO *opt_collation)
      : super(Alter_info::ALTER_OPTIONS),
        m_charset(charset),
        m_collation(opt_collation) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

 private:
  const CHARSET_INFO *const m_charset;
  const CHARSET_INFO *const m_collation;
};

class PT_alter_table_force final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  PT_alter_table_force() : super(Alter_info::ALTER_RECREATE) {}
};

class PT_alter_table_order final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  explicit PT_alter_table_order(PT_order_list *order)
      : super(Alter_info::ALTER_ORDER), m_order(order) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

 private:
  PT_order_list *const m_order;
};

class PT_alter_table_partition_by final : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  explicit PT_alter_table_partition_by(PT_partition *partition)
      : super(Alter_info::ALTER_PARTITION), m_partition(partition) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

 private:
  PT_partition *const m_partition;
};

class PT_alter_table_remove_partitioning : public PT_alter_table_action {
  typedef PT_alter_table_action super;

 public:
  PT_alter_table_remove_partitioning()
      : super(Alter_info::ALTER_REMOVE_PARTITIONING) {}
};

class PT_alter_table_standalone_action : public PT_alter_table_action {
  typedef PT_alter_table_action super;

  friend class PT_alter_table_standalone_stmt;  // to access make_cmd()

 protected:
  PT_alter_table_standalone_action(Alter_info::Alter_info_flag alter_info_flag)
      : super(alter_info_flag) {}

 private:
  virtual Sql_cmd *make_cmd(Table_ddl_parse_context *pc) = 0;
};

/**
  Node for the @SQL{ALTER TABLE ADD PARTITION} statement

  @ingroup ptn_alter_table
*/
class PT_alter_table_add_partition : public PT_alter_table_standalone_action {
  typedef PT_alter_table_standalone_action super;

 public:
  explicit PT_alter_table_add_partition(bool no_write_to_binlog)
      : super(Alter_info::ALTER_ADD_PARTITION),
        m_no_write_to_binlog(no_write_to_binlog) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override final {
    return new (pc->mem_root) Sql_cmd_alter_table(pc->alter_info);
  }

 protected:
  partition_info m_part_info;

 private:
  const bool m_no_write_to_binlog;
};

/**
  Node for the @SQL{ALTER TABLE ADD PARTITION (@<partition list@>)} statement

  @ingroup ptn_alter_table
*/
class PT_alter_table_add_partition_def_list final
    : public PT_alter_table_add_partition {
  typedef PT_alter_table_add_partition super;

 public:
  PT_alter_table_add_partition_def_list(
      bool no_write_to_binlog,
      const Mem_root_array<PT_part_definition *> *def_list)
      : super(no_write_to_binlog), m_def_list(def_list) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

 private:
  const Mem_root_array<PT_part_definition *> *m_def_list;
};

/**
  Node for the @SQL{ALTER TABLE ADD PARTITION PARTITIONS (@<n>@)} statement

  @ingroup ptn_alter_table
*/
class PT_alter_table_add_partition_num final
    : public PT_alter_table_add_partition {
  typedef PT_alter_table_add_partition super;

 public:
  PT_alter_table_add_partition_num(bool no_write_to_binlog, uint num_parts)
      : super(no_write_to_binlog) {
    m_part_info.num_parts = num_parts;
  }
};

class PT_alter_table_drop_partition final
    : public PT_alter_table_standalone_action {
  typedef PT_alter_table_standalone_action super;

 public:
  explicit PT_alter_table_drop_partition(const List<String> &partitions)
      : super(Alter_info::ALTER_DROP_PARTITION), m_partitions(partitions) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override final {
    return new (pc->mem_root) Sql_cmd_alter_table(pc->alter_info);
  }

 private:
  const List<String> m_partitions;
};

class PT_alter_table_partition_list_or_all
    : public PT_alter_table_standalone_action {
  typedef PT_alter_table_standalone_action super;

 public:
  explicit PT_alter_table_partition_list_or_all(
      Alter_info::Alter_info_flag alter_info_flag,
      const List<String> *opt_partition_list)
      : super(alter_info_flag), m_opt_partition_list(opt_partition_list) {}

  bool contextualize(Table_ddl_parse_context *pc) override {
    DBUG_ASSERT(pc->alter_info->partition_names.is_empty());
    if (m_opt_partition_list == nullptr)
      pc->alter_info->flags |= Alter_info::ALTER_ALL_PARTITION;
    else
      pc->alter_info->partition_names = *m_opt_partition_list;
    return super::contextualize(pc);
  }

 private:
  const List<String> *m_opt_partition_list;
};

class PT_alter_table_rebuild_partition final
    : public PT_alter_table_partition_list_or_all {
  typedef PT_alter_table_partition_list_or_all super;

 public:
  PT_alter_table_rebuild_partition(bool no_write_to_binlog,
                                   const List<String> *opt_partition_list)
      : super(Alter_info::ALTER_REBUILD_PARTITION, opt_partition_list),
        m_no_write_to_binlog(no_write_to_binlog) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root) Sql_cmd_alter_table(pc->alter_info);
  }

 private:
  const bool m_no_write_to_binlog;
};

class PT_alter_table_optimize_partition final
    : public PT_alter_table_partition_list_or_all {
  typedef PT_alter_table_partition_list_or_all super;

 public:
  PT_alter_table_optimize_partition(bool no_write_to_binlog,
                                    const List<String> *opt_partition_list)
      : super(Alter_info::ALTER_ADMIN_PARTITION, opt_partition_list),
        m_no_write_to_binlog(no_write_to_binlog) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root)
        Sql_cmd_alter_table_optimize_partition(pc->alter_info);
  }

 private:
  const bool m_no_write_to_binlog;
};

class PT_alter_table_analyze_partition
    : public PT_alter_table_partition_list_or_all {
  typedef PT_alter_table_partition_list_or_all super;

 public:
  PT_alter_table_analyze_partition(bool no_write_to_binlog,
                                   const List<String> *opt_partition_list)
      : super(Alter_info::ALTER_ADMIN_PARTITION, opt_partition_list),
        m_no_write_to_binlog(no_write_to_binlog) {}

  bool contextualize(Table_ddl_parse_context *pc) override;
  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root)
        Sql_cmd_alter_table_analyze_partition(pc->thd, pc->alter_info);
  }

 private:
  const bool m_no_write_to_binlog;
};

class PT_alter_table_check_partition
    : public PT_alter_table_partition_list_or_all {
  typedef PT_alter_table_partition_list_or_all super;

 public:
  PT_alter_table_check_partition(const List<String> *opt_partition_list,
                                 uint flags, uint sql_flags)
      : super(Alter_info::ALTER_ADMIN_PARTITION, opt_partition_list),
        m_flags(flags),
        m_sql_flags(sql_flags) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root)
        Sql_cmd_alter_table_check_partition(pc->alter_info);
  }

 private:
  uint m_flags;
  uint m_sql_flags;
};

class PT_alter_table_repair_partition
    : public PT_alter_table_partition_list_or_all {
  typedef PT_alter_table_partition_list_or_all super;

 public:
  PT_alter_table_repair_partition(bool no_write_to_binlog,
                                  const List<String> *opt_partition_list,
                                  uint flags, uint sql_flags)
      : super(Alter_info::ALTER_ADMIN_PARTITION, opt_partition_list),
        m_no_write_to_binlog(no_write_to_binlog),
        m_flags(flags),
        m_sql_flags(sql_flags) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root)
        Sql_cmd_alter_table_repair_partition(pc->alter_info);
  }

 private:
  const bool m_no_write_to_binlog;
  uint m_flags;
  uint m_sql_flags;
};

class PT_alter_table_coalesce_partition final
    : public PT_alter_table_standalone_action {
  typedef PT_alter_table_standalone_action super;

 public:
  PT_alter_table_coalesce_partition(bool no_write_to_binlog, uint num_parts)
      : super(Alter_info::ALTER_COALESCE_PARTITION),
        m_no_write_to_binlog(no_write_to_binlog),
        m_num_parts(num_parts) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root) Sql_cmd_alter_table(pc->alter_info);
  }

 private:
  const bool m_no_write_to_binlog;
  const uint m_num_parts;
};

class PT_alter_table_truncate_partition
    : public PT_alter_table_partition_list_or_all {
  typedef PT_alter_table_partition_list_or_all super;

 public:
  explicit PT_alter_table_truncate_partition(
      const List<String> *opt_partition_list)
      : super(static_cast<Alter_info::Alter_info_flag>(
                  Alter_info::ALTER_ADMIN_PARTITION |
                  Alter_info::ALTER_TRUNCATE_PARTITION),
              opt_partition_list) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root)
        Sql_cmd_alter_table_truncate_partition(pc->alter_info);
  }
};

class PT_alter_table_reorganize_partition final
    : public PT_alter_table_standalone_action {
  typedef PT_alter_table_standalone_action super;

 public:
  explicit PT_alter_table_reorganize_partition(bool no_write_to_binlog)
      : super(Alter_info::ALTER_TABLE_REORG),
        m_no_write_to_binlog(no_write_to_binlog) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root) Sql_cmd_alter_table(pc->alter_info);
  }

 private:
  const bool m_no_write_to_binlog;
  partition_info m_partition_info;
};

class PT_alter_table_reorganize_partition_into final
    : public PT_alter_table_standalone_action {
  typedef PT_alter_table_standalone_action super;

 public:
  explicit PT_alter_table_reorganize_partition_into(
      bool no_write_to_binlog, const List<String> &partition_names,
      const Mem_root_array<PT_part_definition *> *into)
      : super(Alter_info::ALTER_REORGANIZE_PARTITION),
        m_no_write_to_binlog(no_write_to_binlog),
        m_partition_names(partition_names),
        m_into(into) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root) Sql_cmd_alter_table(pc->alter_info);
  }

 private:
  const bool m_no_write_to_binlog;
  const List<String> m_partition_names;
  const Mem_root_array<PT_part_definition *> *m_into;
  partition_info m_partition_info;
};

class PT_alter_table_exchange_partition final
    : public PT_alter_table_standalone_action {
  typedef PT_alter_table_standalone_action super;

 public:
  PT_alter_table_exchange_partition(const LEX_STRING &partition_name,
                                    Table_ident *table_name,
                                    Alter_info::enum_with_validation validation)
      : super(Alter_info::ALTER_EXCHANGE_PARTITION),
        m_partition_name(partition_name),
        m_table_name(table_name),
        m_validation(validation) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root)
        Sql_cmd_alter_table_exchange_partition(pc->alter_info);
  }

 private:
  const LEX_STRING m_partition_name;
  Table_ident *m_table_name;
  const Alter_info::enum_with_validation m_validation;
};

class PT_alter_table_secondary_load final
    : public PT_alter_table_standalone_action {
  using super = PT_alter_table_standalone_action;

 public:
  explicit PT_alter_table_secondary_load()
      : super(Alter_info::ALTER_SECONDARY_LOAD) {}

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root) Sql_cmd_secondary_load_unload(pc->alter_info);
  }
};

class PT_alter_table_secondary_unload final
    : public PT_alter_table_standalone_action {
  using super = PT_alter_table_standalone_action;

 public:
  explicit PT_alter_table_secondary_unload()
      : super(Alter_info::ALTER_SECONDARY_UNLOAD) {}

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root) Sql_cmd_secondary_load_unload(pc->alter_info);
  }
};

class PT_alter_table_discard_partition_tablespace final
    : public PT_alter_table_partition_list_or_all {
  typedef PT_alter_table_partition_list_or_all super;

 public:
  explicit PT_alter_table_discard_partition_tablespace(
      const List<String> *opt_partition_list)
      : super(Alter_info::ALTER_DISCARD_TABLESPACE, opt_partition_list) {}

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root) Sql_cmd_discard_import_tablespace(pc->alter_info);
  }
};

class PT_alter_table_import_partition_tablespace final
    : public PT_alter_table_partition_list_or_all {
  typedef PT_alter_table_partition_list_or_all super;

 public:
  explicit PT_alter_table_import_partition_tablespace(
      const List<String> *opt_partition_list)
      : super(Alter_info::ALTER_IMPORT_TABLESPACE, opt_partition_list) {}

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root) Sql_cmd_discard_import_tablespace(pc->alter_info);
  }
};

class PT_alter_table_discard_tablespace final
    : public PT_alter_table_standalone_action {
  typedef PT_alter_table_standalone_action super;

 public:
  PT_alter_table_discard_tablespace()
      : super(Alter_info::ALTER_DISCARD_TABLESPACE) {}

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root) Sql_cmd_discard_import_tablespace(pc->alter_info);
  }
};

class PT_alter_table_import_tablespace final
    : public PT_alter_table_standalone_action {
  typedef PT_alter_table_standalone_action super;

 public:
  PT_alter_table_import_tablespace()
      : super(Alter_info::ALTER_IMPORT_TABLESPACE) {}

  Sql_cmd *make_cmd(Table_ddl_parse_context *pc) override {
    return new (pc->mem_root) Sql_cmd_discard_import_tablespace(pc->alter_info);
  }
};

class PT_alter_table_stmt final : public PT_table_ddl_stmt_base {
 public:
  explicit PT_alter_table_stmt(
      MEM_ROOT *mem_root, Table_ident *table_name, bool if_exists,
      Mem_root_array<PT_ddl_table_option *> *opt_actions,
      Alter_info::enum_alter_table_algorithm algo,
      Alter_info::enum_alter_table_lock lock,
      Alter_info::enum_with_validation validation)
      : PT_table_ddl_stmt_base(mem_root),
        m_table_name(table_name),
        m_if_exists(if_exists),
        m_opt_actions(opt_actions),
        m_algo(algo),
        m_lock(lock),
        m_validation(validation) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  Table_ident *const m_table_name;
  bool m_if_exists;
  Mem_root_array<PT_ddl_table_option *> *const m_opt_actions;
  const Alter_info::enum_alter_table_algorithm m_algo;
  const Alter_info::enum_alter_table_lock m_lock;
  const Alter_info::enum_with_validation m_validation;

  HA_CREATE_INFO m_create_info;
};

class PT_alter_table_standalone_stmt final : public PT_table_ddl_stmt_base {
 public:
  explicit PT_alter_table_standalone_stmt(
      MEM_ROOT *mem_root, Table_ident *table_name, bool if_exists,
      PT_alter_table_standalone_action *action,
      Alter_info::enum_alter_table_algorithm algo,
      Alter_info::enum_alter_table_lock lock,
      Alter_info::enum_with_validation validation)
      : PT_table_ddl_stmt_base(mem_root),
        m_table_name(table_name),
        m_if_exists(if_exists),
        m_action(action),
        m_algo(algo),
        m_lock(lock),
        m_validation(validation) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  Table_ident *const m_table_name;
  bool m_if_exists;
  PT_alter_table_standalone_action *const m_action;
  const Alter_info::enum_alter_table_algorithm m_algo;
  const Alter_info::enum_alter_table_lock m_lock;
  const Alter_info::enum_with_validation m_validation;

  HA_CREATE_INFO m_create_info;
};

class PT_repair_table_stmt final : public PT_table_ddl_stmt_base {
 public:
  PT_repair_table_stmt(MEM_ROOT *mem_root, bool no_write_to_binlog,
                       Mem_root_array<Table_ident *> *table_list,
                       decltype(HA_CHECK_OPT::flags) flags,
                       decltype(HA_CHECK_OPT::sql_flags) sql_flags)
      : PT_table_ddl_stmt_base(mem_root),
        m_no_write_to_binlog(no_write_to_binlog),
        m_table_list(table_list),
        m_flags(flags),
        m_sql_flags(sql_flags) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  bool m_no_write_to_binlog;
  Mem_root_array<Table_ident *> *m_table_list;
  decltype(HA_CHECK_OPT::flags) m_flags;
  decltype(HA_CHECK_OPT::sql_flags) m_sql_flags;
};

class PT_analyze_table_stmt final : public PT_table_ddl_stmt_base {
 public:
  PT_analyze_table_stmt(MEM_ROOT *mem_root, bool no_write_to_binlog,
                        Mem_root_array<Table_ident *> *table_list,
                        Sql_cmd_analyze_table::Histogram_command command,
                        int num_buckets, List<String> *columns)
      : PT_table_ddl_stmt_base(mem_root),
        m_no_write_to_binlog(no_write_to_binlog),
        m_table_list(table_list),
        m_command(command),
        m_num_buckets(num_buckets),
        m_columns(columns) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  const bool m_no_write_to_binlog;
  const Mem_root_array<Table_ident *> *m_table_list;
  const Sql_cmd_analyze_table::Histogram_command m_command;
  const int m_num_buckets;
  List<String> *m_columns;
};

class PT_check_table_stmt final : public PT_table_ddl_stmt_base {
 public:
  PT_check_table_stmt(MEM_ROOT *mem_root,
                      Mem_root_array<Table_ident *> *table_list,
                      decltype(HA_CHECK_OPT::flags) flags,
                      decltype(HA_CHECK_OPT::sql_flags) sql_flags)
      : PT_table_ddl_stmt_base(mem_root),
        m_table_list(table_list),
        m_flags(flags),
        m_sql_flags(sql_flags) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  Mem_root_array<Table_ident *> *m_table_list;
  decltype(HA_CHECK_OPT::flags) m_flags;
  decltype(HA_CHECK_OPT::sql_flags) m_sql_flags;
};

class PT_optimize_table_stmt final : public PT_table_ddl_stmt_base {
 public:
  PT_optimize_table_stmt(MEM_ROOT *mem_root, bool no_write_to_binlog,
                         Mem_root_array<Table_ident *> *table_list)
      : PT_table_ddl_stmt_base(mem_root),
        m_no_write_to_binlog(no_write_to_binlog),
        m_table_list(table_list) {}

  Sql_cmd *make_cmd(THD *thd) override;

  bool m_no_write_to_binlog;
  Mem_root_array<Table_ident *> *m_table_list;
};

class PT_drop_index_stmt final : public PT_table_ddl_stmt_base {
 public:
  PT_drop_index_stmt(MEM_ROOT *mem_root, const char *index_name,
                     Table_ident *table,
                     Alter_info::enum_alter_table_algorithm algo,
                     Alter_info::enum_alter_table_lock lock)
      : PT_table_ddl_stmt_base(mem_root),
        m_index_name(index_name),
        m_table(table),
        m_algo(algo),
        m_lock(lock),
        m_alter_drop(Alter_drop::KEY, m_index_name) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  const char *m_index_name;
  Table_ident *m_table;
  Alter_info::enum_alter_table_algorithm m_algo;
  Alter_info::enum_alter_table_lock m_lock;

  Alter_drop m_alter_drop;
};

class PT_truncate_table_stmt final : public Parse_tree_root {
 public:
  explicit PT_truncate_table_stmt(Table_ident *table) : m_table(table) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  Table_ident *m_table;

  Sql_cmd_truncate_table m_cmd_truncate_table;
};

class PT_assign_to_keycache final : public Table_ddl_node {
  typedef Table_ddl_node super;

 public:
  PT_assign_to_keycache(Table_ident *table, List<Index_hint> *index_hints)
      : m_table(table), m_index_hints(index_hints) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

 private:
  Table_ident *m_table;
  List<Index_hint> *m_index_hints;
};

class PT_adm_partition final : public Table_ddl_node {
  typedef Table_ddl_node super;

 public:
  explicit PT_adm_partition(List<String> *opt_partitions)
      : m_opt_partitions(opt_partitions) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

 private:
  List<String> *m_opt_partitions;
};

class PT_cache_index_stmt final : public PT_table_ddl_stmt_base {
 public:
  PT_cache_index_stmt(MEM_ROOT *mem_root,
                      Mem_root_array<PT_assign_to_keycache *> *tbl_index_lists,
                      const LEX_CSTRING &key_cache_name)
      : PT_table_ddl_stmt_base(mem_root),
        m_tbl_index_lists(tbl_index_lists),
        m_key_cache_name(key_cache_name) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  Mem_root_array<PT_assign_to_keycache *> *m_tbl_index_lists;
  const LEX_CSTRING m_key_cache_name;
};

class PT_cache_index_partitions_stmt : public PT_table_ddl_stmt_base {
 public:
  PT_cache_index_partitions_stmt(MEM_ROOT *mem_root, Table_ident *table,
                                 PT_adm_partition *partitions,
                                 List<Index_hint> *opt_key_usage_list,
                                 const LEX_CSTRING &key_cache_name)
      : PT_table_ddl_stmt_base(mem_root),
        m_table(table),
        m_partitions(partitions),
        m_opt_key_usage_list(opt_key_usage_list),
        m_key_cache_name(key_cache_name) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  Table_ident *m_table;
  PT_adm_partition *m_partitions;
  List<Index_hint> *m_opt_key_usage_list;
  const LEX_CSTRING m_key_cache_name;
};

class PT_preload_keys final : public Table_ddl_node {
  typedef Table_ddl_node super;

 public:
  PT_preload_keys(Table_ident *table, List<Index_hint> *opt_cache_key_list,
                  bool ignore_leaves)
      : m_table(table),
        m_opt_cache_key_list(opt_cache_key_list),
        m_ignore_leaves(ignore_leaves) {}

  bool contextualize(Table_ddl_parse_context *pc) override;

 private:
  Table_ident *m_table;
  List<Index_hint> *m_opt_cache_key_list;
  bool m_ignore_leaves;
};

class PT_load_index_partitions_stmt final : public PT_table_ddl_stmt_base {
 public:
  PT_load_index_partitions_stmt(MEM_ROOT *mem_root, Table_ident *table,
                                PT_adm_partition *partitions,
                                List<Index_hint> *opt_cache_key_list,
                                bool ignore_leaves)
      : PT_table_ddl_stmt_base(mem_root),
        m_table(table),
        m_partitions(partitions),
        m_opt_cache_key_list(opt_cache_key_list),
        m_ignore_leaves(ignore_leaves) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  Table_ident *m_table;
  PT_adm_partition *m_partitions;
  List<Index_hint> *m_opt_cache_key_list;
  bool m_ignore_leaves;
};

class PT_load_index_stmt final : public PT_table_ddl_stmt_base {
 public:
  PT_load_index_stmt(MEM_ROOT *mem_root,
                     Mem_root_array<PT_preload_keys *> *preload_list)
      : PT_table_ddl_stmt_base(mem_root), m_preload_list(preload_list) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  Mem_root_array<PT_preload_keys *> *m_preload_list;
};

/**
  Base class for Parse tree nodes of SHOW TABLES statements.
*/
class PT_show_tables : public Parse_tree_root {
 public:
  PT_show_tables(const POS &pos, Show_cmd_type show_cmd_type, char *opt_db,
                 const LEX_STRING &wild, Item *where_condition)
      : m_pos(pos),
        m_sql_cmd(SQLCOM_SHOW_TABLES),
        m_opt_db(opt_db),
        m_wild(wild),
        m_where_condition(where_condition),
        m_show_cmd_type(show_cmd_type) {
    DBUG_ASSERT(m_wild.str == nullptr || m_where_condition == nullptr);
  }

 public:
  Sql_cmd *make_cmd(THD *thd) override;

 private:
  /// Textual location of a token just parsed.
  POS m_pos;

  /// Sql_cmd for SHOW TABLES statements.
  Sql_cmd_show m_sql_cmd;

  /// Optional schema name in FROM/IN clause.
  char *m_opt_db;

  /// Wild or where clause used in the statement.
  LEX_STRING m_wild;
  Item *m_where_condition;

  Show_cmd_type m_show_cmd_type;
};

class PT_json_table_column_for_ordinality final : public PT_json_table_column {
  typedef PT_json_table_column super;

 public:
  explicit PT_json_table_column_for_ordinality(LEX_STRING name);
  ~PT_json_table_column_for_ordinality() override;
  bool contextualize(Parse_context *pc) override;
  Json_table_column *get_column() override { return m_column.get(); }

 private:
  unique_ptr_destroy_only<Json_table_column> m_column;
  const char *m_name;
};

class PT_json_table_column_with_path final : public PT_json_table_column {
  typedef PT_json_table_column super;

 public:
  PT_json_table_column_with_path(
      unique_ptr_destroy_only<Json_table_column> column, LEX_STRING name,
      PT_type *type, const CHARSET_INFO *collation);
  ~PT_json_table_column_with_path() override;

  bool contextualize(Parse_context *pc) override;

  Json_table_column *get_column() override { return m_column.get(); }

 private:
  unique_ptr_destroy_only<Json_table_column> m_column;
  const char *m_name;
  PT_type *m_type;
  const CHARSET_INFO *m_collation;
};

class PT_json_table_column_with_nested_path final
    : public PT_json_table_column {
  typedef PT_json_table_column super;

 public:
  PT_json_table_column_with_nested_path(
      Item *path, Mem_root_array<PT_json_table_column *> *nested_cols)
      : m_path(path), m_nested_columns(nested_cols) {}

  bool contextualize(Parse_context *pc) override;

  Json_table_column *get_column() override { return m_column; }

 private:
  Item *m_path;
  const Mem_root_array<PT_json_table_column *> *m_nested_columns;
  Json_table_column *m_column{nullptr};
};

struct Alter_tablespace_parse_context : public Tablespace_options {
  THD *const thd;
  MEM_ROOT *const mem_root;

  explicit Alter_tablespace_parse_context(THD *thd);
};

typedef Parse_tree_node_tmpl<Alter_tablespace_parse_context>
    PT_alter_tablespace_option_base;

template <typename Option_type, Option_type Tablespace_options::*Option>
class PT_alter_tablespace_option final
    : public PT_alter_tablespace_option_base /* purecov: inspected */
{
  typedef PT_alter_tablespace_option_base super;

 public:
  explicit PT_alter_tablespace_option(Option_type value) : m_value(value) {}

  bool contextualize(Alter_tablespace_parse_context *pc) override {
    pc->*Option = m_value;
    return super::contextualize(pc);
  }

 private:
  const Option_type m_value;
};

typedef PT_alter_tablespace_option<decltype(
                                       Tablespace_options::autoextend_size),
                                   &Tablespace_options::autoextend_size>
    PT_alter_tablespace_option_autoextend_size;

typedef PT_alter_tablespace_option<decltype(Tablespace_options::extent_size),
                                   &Tablespace_options::extent_size>
    PT_alter_tablespace_option_extent_size;

typedef PT_alter_tablespace_option<decltype(Tablespace_options::initial_size),
                                   &Tablespace_options::initial_size>
    PT_alter_tablespace_option_initial_size;

typedef PT_alter_tablespace_option<decltype(Tablespace_options::max_size),
                                   &Tablespace_options::max_size>
    PT_alter_tablespace_option_max_size;

typedef PT_alter_tablespace_option<decltype(
                                       Tablespace_options::redo_buffer_size),
                                   &Tablespace_options::redo_buffer_size>
    PT_alter_tablespace_option_redo_buffer_size;

typedef PT_alter_tablespace_option<decltype(
                                       Tablespace_options::undo_buffer_size),
                                   &Tablespace_options::undo_buffer_size>
    PT_alter_tablespace_option_undo_buffer_size;

typedef PT_alter_tablespace_option<
    decltype(Tablespace_options::wait_until_completed),
    &Tablespace_options::wait_until_completed>
    PT_alter_tablespace_option_wait_until_completed;

typedef PT_alter_tablespace_option<decltype(Tablespace_options::encryption),
                                   &Tablespace_options::encryption>
    PT_alter_tablespace_option_encryption;

class PT_alter_tablespace_option_nodegroup final
    : public PT_alter_tablespace_option_base /* purecov: inspected */
{
  typedef PT_alter_tablespace_option_base super;
  typedef decltype(Tablespace_options::nodegroup_id) option_type;

 public:
  explicit PT_alter_tablespace_option_nodegroup(option_type nodegroup_id)
      : m_nodegroup_id(nodegroup_id) {}

  bool contextualize(Alter_tablespace_parse_context *pc) override;

 private:
  const option_type m_nodegroup_id;
};

class PT_alter_tablespace_option_comment final
    : public PT_alter_tablespace_option_base /* purecov: inspected */
{
  typedef PT_alter_tablespace_option_base super;
  typedef decltype(Tablespace_options::ts_comment) option_type;

 public:
  explicit PT_alter_tablespace_option_comment(option_type comment)
      : m_comment(comment) {}

  bool contextualize(Alter_tablespace_parse_context *pc) override {
    if (super::contextualize(pc)) return true; /* purecov: inspected */  // OOM

    if (pc->ts_comment.str) {
      my_error(ER_FILEGROUP_OPTION_ONLY_ONCE, MYF(0), "COMMENT");
      return true;
    }
    pc->ts_comment = m_comment;
    return false;
  }

 private:
  const option_type m_comment;
};

class PT_alter_tablespace_option_engine final
    : public PT_alter_tablespace_option_base /* purecov: inspected */
{
  typedef PT_alter_tablespace_option_base super;
  typedef decltype(Tablespace_options::engine_name) option_type;

 public:
  explicit PT_alter_tablespace_option_engine(option_type engine_name)
      : m_engine_name(engine_name) {}

  bool contextualize(Alter_tablespace_parse_context *pc) override {
    if (super::contextualize(pc)) return true; /* purecov: inspected */  // OOM

    if (pc->engine_name.str) {
      my_error(ER_FILEGROUP_OPTION_ONLY_ONCE, MYF(0), "STORAGE ENGINE");
      return true;
    }
    pc->engine_name = m_engine_name;
    return false;
  }

 private:
  const option_type m_engine_name;
};

class PT_alter_tablespace_option_file_block_size final
    : public PT_alter_tablespace_option_base /* purecov: inspected */
{
  typedef PT_alter_tablespace_option_base super;
  typedef decltype(Tablespace_options::file_block_size) option_type;

 public:
  explicit PT_alter_tablespace_option_file_block_size(
      option_type file_block_size)
      : m_file_block_size(file_block_size) {}

  bool contextualize(Alter_tablespace_parse_context *pc) override {
    if (super::contextualize(pc)) return true; /* purecov: inspected */  // OOM

    if (pc->file_block_size != 0) {
      my_error(ER_FILEGROUP_OPTION_ONLY_ONCE, MYF(0), "FILE_BLOCK_SIZE");
      return true;
    }
    pc->file_block_size = m_file_block_size;
    return false;
  }

 private:
  const option_type m_file_block_size;
};

/**
  Parse tree node for CREATE RESOURCE GROUP statement.
*/

class PT_create_resource_group final : public Parse_tree_root {
  resourcegroups::Sql_cmd_create_resource_group sql_cmd;
  const bool has_priority;

 public:
  PT_create_resource_group(
      const LEX_CSTRING &name, const resourcegroups::Type type,
      const Mem_root_array<resourcegroups::Range> *cpu_list,
      const Value_or_default<int> &opt_priority, bool enabled)
      : sql_cmd(name, type, cpu_list,
                opt_priority.is_default ? 0 : opt_priority.value, enabled),
        has_priority(!opt_priority.is_default) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

/**
  Parse tree node for ALTER RESOURCE GROUP statement.
*/

class PT_alter_resource_group final : public Parse_tree_root {
  resourcegroups::Sql_cmd_alter_resource_group sql_cmd;

 public:
  PT_alter_resource_group(const LEX_CSTRING &name,
                          const Mem_root_array<resourcegroups::Range> *cpu_list,
                          const Value_or_default<int> &opt_priority,
                          const Value_or_default<bool> &enable, bool force)
      : sql_cmd(name, cpu_list,
                opt_priority.is_default ? 0 : opt_priority.value,
                enable.is_default ? false : enable.value, force,
                !enable.is_default) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

/**
  Parse tree node for DROP RESOURCE GROUP statement.
*/

class PT_drop_resource_group final : public Parse_tree_root {
  resourcegroups::Sql_cmd_drop_resource_group sql_cmd;

 public:
  PT_drop_resource_group(const LEX_CSTRING &resource_group_name, bool force)
      : sql_cmd(resource_group_name, force) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

/**
  Parse tree node for SET RESOURCE GROUP statement.
*/

class PT_set_resource_group final : public Parse_tree_root {
  resourcegroups::Sql_cmd_set_resource_group sql_cmd;

 public:
  PT_set_resource_group(const LEX_CSTRING &name,
                        Mem_root_array<ulonglong> *thread_id_list)
      : sql_cmd(name, thread_id_list) {}

  Sql_cmd *make_cmd(THD *thd) override;
};

class PT_explain_for_connection final : public Parse_tree_root {
 public:
  explicit PT_explain_for_connection(my_thread_id thread_id)
      : m_cmd(thread_id) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  Sql_cmd_explain_other_thread m_cmd;
};

class PT_explain final : public Parse_tree_root {
 public:
  PT_explain(Explain_format_type format, Parse_tree_root *explainable_stmt)
      : m_format(format), m_explainable_stmt(explainable_stmt) {}

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  const Explain_format_type m_format;
  Parse_tree_root *const m_explainable_stmt;
};

class PT_load_table final : public Parse_tree_root {
 public:
  PT_load_table(enum_filetype filetype, thr_lock_type lock_type,
                bool is_local_file, const LEX_STRING filename,
                On_duplicate on_duplicate, Table_ident *table,
                bool opt_compressed, List<String> *opt_partitions,
                const CHARSET_INFO *opt_charset,
                String *opt_xml_rows_identified_by,
                const Field_separators &opt_field_separators,
                const Line_separators &opt_line_separators,
                ulong opt_ignore_lines, PT_item_list *opt_fields_or_vars,
                PT_item_list *opt_set_fields, PT_item_list *opt_set_exprs,
                List<String> *opt_set_expr_strings)
      : m_cmd(filetype, is_local_file, filename, on_duplicate, table,
              opt_compressed, opt_partitions, opt_charset,
              opt_xml_rows_identified_by, opt_field_separators,
              opt_line_separators, opt_ignore_lines,
              opt_fields_or_vars ? &opt_fields_or_vars->value : nullptr,
              opt_set_fields ? &opt_set_fields->value : nullptr,
              opt_set_exprs ? &opt_set_exprs->value : nullptr,
              opt_set_expr_strings),
        m_lock_type(lock_type),
        m_opt_fields_or_vars(opt_fields_or_vars),
        m_opt_set_fields(opt_set_fields),
        m_opt_set_exprs(opt_set_exprs) {
    DBUG_ASSERT((opt_set_fields == nullptr) ^ (opt_set_exprs != nullptr));
    DBUG_ASSERT(opt_set_fields == nullptr || opt_set_fields->value.elements ==
                                                 opt_set_exprs->value.elements);
  }

  Sql_cmd *make_cmd(THD *thd) override;

 private:
  Sql_cmd_load_table m_cmd;

  const thr_lock_type m_lock_type;
  PT_item_list *m_opt_fields_or_vars;
  PT_item_list *m_opt_set_fields;
  PT_item_list *m_opt_set_exprs;
};

/**
  Top-level node for the SHUTDOWN statement

  @ingroup ptn_stmt
*/

class PT_restart_server final : public Parse_tree_root {
 public:
  Sql_cmd *make_cmd(THD *thd) override;

 private:
  Sql_cmd_restart_server sql_cmd;
};

#endif /* PARSE_TREE_NODES_INCLUDED */
