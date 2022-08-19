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

#ifndef PARSER_YYSTYPE_INCLUDED
#define PARSER_YYSTYPE_INCLUDED

#include "my_base.h"
#include "my_time.h"  // interval_type
#include "sql/comp_creator.h"
#include "sql/handler.h"
#include "sql/item_create.h"    // Cast_target
#include "sql/key_spec.h"       // keytype, fk_option
#include "sql/lexer_yystype.h"  // Lexer_yystype
#include "sql/opt_hints.h"      // opt_hints_enum
#include "sql/parse_tree_hints.h"
#include "sql/resourcegroups/platform/thread_attrs_api.h"  // ...::cpu_id_t
#include "sql/resourcegroups/resource_group_basic_types.h"  // resourcegroups::Range
#include "sql/sql_admin.h"     // Sql_cmd_analyze_table::Histogram_command
#include "sql/sql_alter.h"     // Alter_info::enum_with_validation
#include "sql/sql_exchange.h"  // Line_separators, enum_filetype
#include "sql/sql_get_diagnostics.h"  // Diagnostics_information::Which_area
#include "sql/sql_signal.h"           // enum_condition_item_name
#include "sql/table.h"                // index_hint_type
#include "sql/trigger_def.h"          // enum_trigger_order_type
#include "sql/window_lex.h"           // enum_window_frame_unit
#include "sql/xa.h"                   // xa_option_words
#include "sql_string.h"               // String
#include "thr_lock.h"                 // thr_lock_type

class PT_ddl_table_option;
class PT_json_table_column;
class Parse_tree_root;
class Table_ident;
enum class Acl_type;
enum class enum_ha_read_modes;
enum class enum_jt_column;
enum class enum_jtc_on : uint16;
enum class enum_key_algorithm;
enum class partition_type;
struct Alter_tablespace_parse_context;
struct Sql_cmd_srs_attributes;
struct udf_func;

typedef Parse_tree_node_tmpl<Alter_tablespace_parse_context>
    PT_alter_tablespace_option_base;

enum enum_yes_no_unknown { TVL_YES, TVL_NO, TVL_UNKNOWN };

enum class enum_ha_read_modes;

/**
  used by the parser to store internal variable name
*/
struct sys_var_with_base {
  sys_var *var;
  LEX_CSTRING base_name;
};

enum enum_drop_mode {
  DROP_DEFAULT,  // mode is not specified
  DROP_CASCADE,  // CASCADE option
  DROP_RESTRICT  // RESTRICT option
};

enum olap_type { UNSPECIFIED_OLAP_TYPE, ROLLUP_TYPE };

struct Cast_type {
  Cast_target target;
  const CHARSET_INFO *charset;
  const char *length;
  const char *dec;
};

struct Limit_options {
  Item *limit;
  Item *opt_offset;
  /*
    true for "LIMIT offset,limit" and false for "LIMIT limit OFFSET offset"
  */
  bool is_offset_first;
};

struct Query_options {
  ulonglong query_spec_options;

  bool merge(const Query_options &a, const Query_options &b);
  bool save_to(Parse_context *);
};

enum delete_option_enum {
  DELETE_QUICK = 1 << 0,
  DELETE_LOW_PRIORITY = 1 << 1,
  DELETE_IGNORE = 1 << 2
};

enum class Lock_strength { UPDATE, SHARE };

/// We will static_cast this one to thr_lock_type.
enum class Locked_row_action {
  DEFAULT = THR_DEFAULT,
  WAIT = THR_WAIT,
  NOWAIT = THR_NOWAIT,
  SKIP = THR_SKIP
};

/**
  Internally there is no CROSS JOIN join type, as cross joins are just a
  special case of inner joins with a join condition that is always true. The
  only difference is the nesting, and that is handled by the parser.
*/
enum PT_joined_table_type {
  JTT_INNER = 0x01,
  JTT_STRAIGHT = 0x02,
  JTT_NATURAL = 0x04,
  JTT_LEFT = 0x08,
  JTT_RIGHT = 0x10,

  JTT_STRAIGHT_INNER = JTT_STRAIGHT | JTT_INNER,
  JTT_NATURAL_INNER = JTT_NATURAL | JTT_INNER,
  JTT_NATURAL_LEFT = JTT_NATURAL | JTT_LEFT,
  JTT_NATURAL_RIGHT = JTT_NATURAL | JTT_RIGHT
};

typedef Mem_root_array_YY<LEX_CSTRING> Create_col_name_list;

enum class Ternary_option { DEFAULT, ON, OFF };

enum class On_duplicate { ERROR, IGNORE_DUP, REPLACE_DUP };

enum class Virtual_or_stored { VIRTUAL, STORED };

enum class Int_type : ulong {
  INT = MYSQL_TYPE_LONG,
  TINYINT = MYSQL_TYPE_TINY,
  SMALLINT = MYSQL_TYPE_SHORT,
  MEDIUMINT = MYSQL_TYPE_INT24,
  BIGINT = MYSQL_TYPE_LONGLONG,
};

enum class Numeric_type : ulong {
  DECIMAL = MYSQL_TYPE_NEWDECIMAL,
  FLOAT = MYSQL_TYPE_FLOAT,
  DOUBLE = MYSQL_TYPE_DOUBLE,
};

enum class Show_cmd_type {
  STANDARD,
  FULL_SHOW,
  EXTENDED_SHOW,
  EXTENDED_FULL_SHOW
};

/**
  std::optional-like wrapper for simple bitmaps (usually enums of binary flags)

  This template wraps trivial bitmap implementations to add two features:

  * std::optional-like behavior -- the "unset" flag, so we don't have
    to inject a special "invalid" value into existent enum types, this
    wrapper class does that for us.

  * the merge() function to merge two bitmap values in a type-safe way.

  @tparam Enum           Usually a enum type which simulates a bit set.
  @tparam Default_value  A default Enum value for "unset" variables.

*/
template <typename Enum, Enum Default_value>
class Enum_parser {
 public:
  /// Constructor-like function
  ///
  /// The Enum_parser<> class is designed for use as a field of restricted
  /// unions, so it can't have C++ constructors.
  void init() { m_is_set = false; }

  /// False if the wrapped Enum value is not assigned.
  bool is_set() const { return m_is_set; }

  /// Return the wrapped Enum value.
  ///
  /// @note The wrapped value must be assigned.
  Enum get() const {
    DBUG_ASSERT(is_set());
    return m_enum;
  }

  /// Return the wrapped Enum value (if any) or the Default_value.
  Enum get_or_default() const { return is_set() ? get() : Default_value; }

  /// Assign the wrapped Enum value.
  void set(Enum value) {
    m_is_set = true;
    m_enum = value;
  }

  /// Merge the x bit set into the wrapped Enum value (if any), or replace it
  void merge(const Enum_parser &x) {
    if (x.is_set()) set(x.get());
  }

 private:
  bool m_is_set;  ///< True if m_enum is assigned with some value
  Enum m_enum;    ///< The wrapped Enum value.
};

template <typename T>
struct Value_or_default {
  bool is_default;
  T value;  ///< undefined if is_default is true
};

enum class Explain_format_type { TRADITIONAL, JSON, TREE, TREE_WITH_EXECUTE };

// Compatibility with Bison 2.3:
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif  // YYSTYPE_IS_DECLARED

union YYSTYPE {
  Lexer_yystype lexer;  // terminal values from the lexical scanner
  /*
    Hint parser section (sql_hints.yy)
  */
  opt_hints_enum hint_type;
  class PT_hint *hint;
  class PT_hint_list *hint_list;
  Hint_param_index_list hint_param_index_list;
  Hint_param_table hint_param_table;
  Hint_param_table_list hint_param_table_list;

  /*
    Main parser section (sql_yacc.yy)
  */
  int num;
  ulong ulong_num;
  ulonglong ulonglong_number;
  LEX_CSTRING lex_cstr;
  LEX_STRING *lex_str_ptr;
  Table_ident *table;
  char *simple_string;
  Item *item;
  Item_num *item_num;
  List<Item> *item_list;
  List<String> *string_list;
  String *string;
  Mem_root_array<Table_ident *> *table_list;
  udf_func *udf;
  LEX_USER *lex_user;
  List<LEX_USER> *user_list;
  struct sys_var_with_base variable;
  enum enum_var_type var_type;
  keytype key_type;
  enum ha_key_alg key_alg;
  enum row_type row_type;
  enum ha_rkey_function ha_rkey_mode;
  enum_ha_read_modes ha_read_mode;
  enum enum_tx_isolation tx_isolation;
  const char *c_str;
  struct {
    const CHARSET_INFO *charset;
    bool force_binary;
  } charset_with_opt_binary;
  struct {
    const char *length;
    const char *dec;
  } precision;
  struct Cast_type cast_type;
  thr_lock_type lock_type;
  interval_type interval, interval_time_st;
  enum_mysql_timestamp_type date_time_type;
  SELECT_LEX *select_lex;
  chooser_compare_func_creator boolfunc2creator;
  class sp_condition_value *spcondvalue;
  struct {
    int vars, conds, hndlrs, curs;
  } spblock;
  sp_name *spname;
  LEX *lex;
  sp_head *sphead;
  enum index_hint_type index_hint;
  enum enum_filetype filetype;
  enum fk_option m_fk_option;
  enum enum_yes_no_unknown m_yes_no_unk;
  enum_condition_item_name da_condition_item_name;
  Diagnostics_information::Which_area diag_area;
  Diagnostics_information *diag_info;
  Statement_information_item *stmt_info_item;
  Statement_information_item::Name stmt_info_item_name;
  List<Statement_information_item> *stmt_info_list;
  Condition_information_item *cond_info_item;
  Condition_information_item::Name cond_info_item_name;
  List<Condition_information_item> *cond_info_list;
  bool is_not_empty;
  Set_signal_information *signal_item_list;
  enum enum_trigger_order_type trigger_action_order_type;
  struct {
    enum enum_trigger_order_type ordering_clause;
    LEX_CSTRING anchor_trigger_name;
  } trg_characteristics;
  class Index_hint *key_usage_element;
  List<Index_hint> *key_usage_list;
  class PT_subselect *subselect;
  class PT_item_list *item_list2;
  class PT_order_expr *order_expr;
  class PT_order_list *order_list;
  struct Limit_options limit_options;
  Query_options select_options;
  class PT_limit_clause *limit_clause;
  Parse_tree_node *node;
  enum olap_type olap_type;
  class PT_group *group;
  class PT_window_list *windows;
  class PT_window *window;
  class PT_frame *window_frame;
  enum_window_frame_unit frame_units;
  class PT_borders *frame_extent;
  class PT_border *bound;
  class PT_exclusion *frame_exclusion;
  enum enum_null_treatment null_treatment;
  enum enum_from_first_last from_first_last;
  Item_string *item_string;
  class PT_order *order;
  class PT_table_reference *table_reference;
  class PT_joined_table *join_table;
  enum PT_joined_table_type join_type;
  class PT_internal_variable_name *internal_variable_name;
  class PT_option_value_following_option_type
      *option_value_following_option_type;
  class PT_option_value_no_option_type *option_value_no_option_type;
  class PT_option_value_list_head *option_value_list;
  class PT_start_option_value_list *start_option_value_list;
  class PT_transaction_access_mode *transaction_access_mode;
  class PT_isolation_level *isolation_level;
  class PT_transaction_characteristics *transaction_characteristics;
  class PT_start_option_value_list_following_option_type
      *start_option_value_list_following_option_type;
  class PT_set *set;
  Line_separators line_separators;
  Field_separators field_separators;
  class PT_into_destination *into_destination;
  class PT_select_var *select_var_ident;
  class PT_select_var_list *select_var_list;
  Mem_root_array_YY<PT_table_reference *> table_reference_list;
  class Item_param *param_marker;
  class PTI_text_literal *text_literal;
  class PT_query_expression *query_expression;
  class PT_derived_table *derived_table;
  class PT_query_expression_body *query_expression_body;
  class PT_query_primary *query_primary;
  class PT_subquery *subquery;
  class PT_key_part_specification *key_part;

  XID *xid;
  enum xa_option_words xa_option_type;
  struct {
    Item *column;
    Item *value;
  } column_value_pair;
  struct {
    class PT_item_list *column_list;
    class PT_item_list *value_list;
  } column_value_list_pair;
  struct {
    class PT_item_list *column_list;
    class PT_insert_values_list *row_value_list;
  } column_row_value_list_pair;
  struct {
    class PT_item_list *column_list;
    class PT_query_primary *insert_query_expression;
  } insert_query_expression;
  struct {
    class Item *offset;
    class Item *default_value;
  } lead_lag_info;
  class PT_insert_values_list *values_list;
  Parse_tree_root *top_level_node;
  class Table_ident *table_ident;
  Mem_root_array_YY<Table_ident *> table_ident_list;
  delete_option_enum opt_delete_option;
  enum alter_instance_action_enum alter_instance_action;
  class PT_create_index_stmt *create_index_stmt;
  class PT_table_constraint_def *table_constraint_def;
  List<PT_key_part_specification> *index_column_list;
  struct {
    LEX_STRING name;
    class PT_base_index_option *type;
  } index_name_and_type;
  PT_base_index_option *index_option;
  Mem_root_array_YY<PT_base_index_option *> index_options;
  Mem_root_array_YY<LEX_STRING> lex_str_list;
  bool visibility;
  class PT_with_clause *with_clause;
  class PT_with_list *with_list;
  class PT_common_table_expr *common_table_expr;
  Create_col_name_list simple_ident_list;
  class PT_partition_option *partition_option;
  Mem_root_array<PT_partition_option *> *partition_option_list;
  class PT_subpartition *sub_part_definition;
  Mem_root_array<PT_subpartition *> *sub_part_list;
  class PT_part_value_item *part_value_item;
  Mem_root_array<PT_part_value_item *> *part_value_item_list;
  class PT_part_value_item_list_paren *part_value_item_list_paren;
  Mem_root_array<PT_part_value_item_list_paren *> *part_value_list;
  class PT_part_values *part_values;
  struct {
    partition_type type;
    PT_part_values *values;
  } opt_part_values;
  class PT_part_definition *part_definition;
  Mem_root_array<PT_part_definition *> *part_def_list;
  List<char> *name_list;  // TODO: merge with string_list
  enum_key_algorithm opt_key_algo;
  class PT_sub_partition *opt_sub_part;
  class PT_part_type_def *part_type_def;
  class PT_partition *partition_clause;
  class PT_add_partition *add_partition_rule;
  struct {
    decltype(HA_CHECK_OPT::flags) flags;
    decltype(HA_CHECK_OPT::sql_flags) sql_flags;
  } mi_type;
  enum_drop_mode opt_restrict;
  Ternary_option ternary_option;
  class PT_create_table_option *create_table_option;
  Mem_root_array<PT_create_table_option *> *create_table_options;
  Mem_root_array<PT_ddl_table_option *> *space_separated_alter_table_opts;
  On_duplicate on_duplicate;
  class PT_column_attr_base *col_attr;
  column_format_type column_format;
  ha_storage_media storage_media;
  Mem_root_array<PT_column_attr_base *> *col_attr_list;
  Virtual_or_stored virtual_or_stored;
  ulong field_option;  // 0 or combinations of UNSIGNED_FLAG and ZEROFILL_FLAG
  Int_type int_type;
  class PT_type *type;
  Numeric_type numeric_type;
  struct {
    const char *expr_start;
    Item *expr;
  } sp_default;
  class PT_field_def_base *field_def;
  struct {
    fk_option fk_update_opt;
    fk_option fk_delete_opt;
  } fk_options;
  fk_match_opt opt_match_clause;
  List<Key_part_spec> *reference_list;
  struct {
    Table_ident *table_name;
    List<Key_part_spec> *reference_list;
    fk_match_opt fk_match_option;
    fk_option fk_update_opt;
    fk_option fk_delete_opt;
  } fk_references;
  class PT_column_def *column_def;
  class PT_table_element *table_element;
  Mem_root_array<PT_table_element *> *table_element_list;
  struct {
    Mem_root_array<PT_create_table_option *> *opt_create_table_options;
    PT_partition *opt_partitioning;
    On_duplicate on_duplicate;
    PT_query_primary *opt_query_expression;
  } create_table_tail;
  Lock_strength lock_strength;
  Locked_row_action locked_row_action;
  class PT_locking_clause *locking_clause;
  class PT_locking_clause_list *locking_clause_list;
  Mem_root_array<PT_json_table_column *> *jtc_list;
  struct jt_on_response {
    enum_jtc_on type;
    Item *default_string;
  } jt_on_response;
  struct {
    struct jt_on_response error;
    struct jt_on_response empty;
  } jt_on_error_or_empty;
  PT_json_table_column *jt_column;
  enum_jt_column jt_column_type;
  struct {
    LEX_STRING wild;
    Item *where;
  } wild_or_where;
  Show_cmd_type show_cmd_type;
  struct {
    Sql_cmd_analyze_table::Histogram_command command;
    List<String> *columns;
    int num_buckets;
  } histogram;
  Acl_type acl_type;
  Mem_root_array<LEX_CSTRING> *lex_cstring_list;
  class PT_role_or_privilege *role_or_privilege;
  Mem_root_array<PT_role_or_privilege *> *role_or_privilege_list;
  enum_order order_direction;
  Alter_info::enum_with_validation with_validation;
  class PT_alter_table_action *alter_table_action;
  class PT_alter_table_standalone_action *alter_table_standalone_action;
  Alter_info::enum_alter_table_algorithm alter_table_algorithm;
  Alter_info::enum_alter_table_lock alter_table_lock;
  struct Algo_and_lock {
    Enum_parser<Alter_info::enum_alter_table_algorithm,
                Alter_info::ALTER_TABLE_ALGORITHM_DEFAULT>
        algo;
    Enum_parser<Alter_info::enum_alter_table_lock,
                Alter_info::ALTER_TABLE_LOCK_DEFAULT>
        lock;
    void init() {
      algo.init();
      lock.init();
    }
  } opt_index_lock_and_algorithm;
  struct Algo_and_lock_and_validation {
    Enum_parser<Alter_info::enum_alter_table_algorithm,
                Alter_info::ALTER_TABLE_ALGORITHM_DEFAULT>
        algo;
    Enum_parser<Alter_info::enum_alter_table_lock,
                Alter_info::ALTER_TABLE_LOCK_DEFAULT>
        lock;
    Enum_parser<Alter_info::enum_with_validation,
                Alter_info::ALTER_VALIDATION_DEFAULT>
        validation;
    void init() {
      algo.init();
      lock.init();
      validation.init();
    }
    void merge(const Algo_and_lock_and_validation &x) {
      algo.merge(x.algo);
      lock.merge(x.lock);
      validation.merge(x.validation);
    }
  } algo_and_lock_and_validation;
  struct {
    Algo_and_lock_and_validation flags;
    Mem_root_array<PT_ddl_table_option *> *actions;
  } alter_list;
  struct {
    Algo_and_lock_and_validation flags;
    PT_alter_table_standalone_action *action;
  } standalone_alter_table_action;
  class PT_assign_to_keycache *assign_to_keycache;
  Mem_root_array<PT_assign_to_keycache *> *keycache_list;
  class PT_adm_partition *adm_partition;
  class PT_preload_keys *preload_keys;
  Mem_root_array<PT_preload_keys *> *preload_list;
  PT_alter_tablespace_option_base *ts_option;
  Mem_root_array<PT_alter_tablespace_option_base *> *ts_options;
  struct {
    resourcegroups::platform::cpu_id_t start;
    resourcegroups::platform::cpu_id_t end;
  } vcpu_range_type;
  Mem_root_array<resourcegroups::Range> *resource_group_vcpu_list_type;
  Value_or_default<int> resource_group_priority_type;
  Value_or_default<bool> resource_group_state_type;
  bool resource_group_flag_type;
  resourcegroups::Type resource_group_type;
  Mem_root_array<ulonglong> *thread_id_list_type;
  Explain_format_type explain_format_type;
  struct {
    Item *set_var;
    Item *set_expr;
    String *set_expr_str;
  } load_set_element;
  struct {
    PT_item_list *set_var_list;
    PT_item_list *set_expr_list;
    List<String> *set_expr_str_list;
  } load_set_list;
  ts_alter_tablespace_type alter_tablespace_type;
  Sql_cmd_srs_attributes *sql_cmd_srs_attributes;
  struct {
    LEX_CSTRING table_alias;
    Create_col_name_list *column_list;
  } insert_update_values_reference;
};

static_assert(sizeof(YYSTYPE) <= 32, "YYSTYPE is too big");

#endif  // PARSER_YYSTYPE_INCLUDED
