/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

/*
  Parse tree node classes for optimizer hint syntax
*/

#ifndef OPT_HINTS_INCLUDED
#define OPT_HINTS_INCLUDED

#include <stddef.h>
#include <sys/types.h>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/enum_query_type.h"
#include "sql/mem_root_array.h"  // Mem_root_array
#include "sql/sql_bitmap.h"      // Bitmap
#include "sql/sql_show.h"        // append_identifier
#include "sql_string.h"          // String

enum class SubqueryExecMethod : int;
class Item;
class JOIN;
class Opt_hints_table;
class Sys_var_hint;
class THD;
class set_var;
class sys_var;
struct MEM_ROOT;
struct TABLE;
struct TABLE_LIST;

/**
  Hint types, MAX_HINT_ENUM should be always last.
  This enum should be synchronized with opt_hint_info
  array(see opt_hints.cc).
*/
enum opt_hints_enum {
  BKA_HINT_ENUM = 0,
  BNL_HINT_ENUM,
  ICP_HINT_ENUM,
  MRR_HINT_ENUM,
  NO_RANGE_HINT_ENUM,
  MAX_EXEC_TIME_HINT_ENUM,
  QB_NAME_HINT_ENUM,
  SEMIJOIN_HINT_ENUM,
  SUBQUERY_HINT_ENUM,
  DERIVED_MERGE_HINT_ENUM,
  JOIN_PREFIX_HINT_ENUM,
  JOIN_SUFFIX_HINT_ENUM,
  JOIN_ORDER_HINT_ENUM,
  JOIN_FIXED_ORDER_HINT_ENUM,
  INDEX_MERGE_HINT_ENUM,
  RESOURCE_GROUP_HINT_ENUM,
  SKIP_SCAN_HINT_ENUM,
  GROUP_BY_LIS_HINT_ENUM,
  HASH_JOIN_HINT_ENUM,
  INDEX_HINT_ENUM,
  JOIN_INDEX_HINT_ENUM,
  GROUP_INDEX_HINT_ENUM,
  ORDER_INDEX_HINT_ENUM,
  MAX_HINT_ENUM
};

struct st_opt_hint_info {
  const char *hint_name;  // Hint name.
  bool check_upper_lvl;   // true if upper level hint check is needed (for hints
                          // which can be specified on more than one level).
  bool switch_hint;       // true if hint is not complex.
  bool irregular_hint;    ///< true if hint requires some special handling.
                          ///< Currently it's used only for join order hints
                          ///< since they need special printing procedure.
};

/**
  Opt_hints_map contains information
  about hint state(specified or not, hint value).
*/

class Opt_hints_map {
  Bitmap<64> hints;            // hint state
  Bitmap<64> hints_specified;  // true if hint is specified
 public:
  /**
     Check if hint is specified.

     @param type_arg   hint type

     @return true if hint is specified
  */
  bool is_specified(opt_hints_enum type_arg) const {
    return hints_specified.is_set(type_arg);
  }
  /**
     Set switch value and set hint into specified state.

     @param type_arg           hint type
     @param switch_state_arg   switch value
  */
  void set_switch(opt_hints_enum type_arg, bool switch_state_arg) {
    if (switch_state_arg)
      hints.set_bit(type_arg);
    else
      hints.clear_bit(type_arg);
    hints_specified.set_bit(type_arg);
  }
  /**
     Get switch value.

     @param type_arg    hint type

     @return switch value.
  */
  bool switch_on(opt_hints_enum type_arg) const {
    return hints.is_set(type_arg);
  }
};

class Opt_hints_key;
class PT_hint;
class PT_hint_max_execution_time;

/**
  Opt_hints class is used as ancestor for Opt_hints_global,
  Opt_hints_qb, Opt_hints_table, Opt_hints_key classes.

  Opt_hints_global class is hierarchical structure.
  It contains information about global hints and also
  conains array of QUERY BLOCK level objects (Opt_hints_qb class).
  Each QUERY BLOCK level object contains array of TABLE level hints
  (class Opt_hints_table). Each TABLE level hint contains array of
  KEY lelev hints (Opt_hints_key class).
  Hint information(specified, on|off state) is stored in hints_map object.
*/

class Opt_hints {
  /*
    Name of object referred by the hint.
    This name is empty for global level,
    query block name for query block level,
    table name for table level and key name
    for key level.
  */
  const LEX_CSTRING *name;
  /*
    Parent object. There is no parent for global level,
    for query block level parent is Opt_hints_global object,
    for table level parent is Opt_hints_qb object,
    for key level parent is Opt_hints_key object.
  */
  Opt_hints *parent;

  Opt_hints_map hints_map;  // Hint map

  /* Array of child objects. i.e. array of the lower level objects */
  Mem_root_array<Opt_hints *> child_array;
  /* true if hint is connected to the real object */
  bool resolved;
  /* Number of resolved children */
  uint resolved_children;

 public:
  Opt_hints(const LEX_CSTRING *name_arg, Opt_hints *parent_arg,
            MEM_ROOT *mem_root_arg)
      : name(name_arg),
        parent(parent_arg),
        child_array(mem_root_arg),
        resolved(false),
        resolved_children(0) {}

  virtual ~Opt_hints() {}

  bool is_specified(opt_hints_enum type_arg) const {
    return hints_map.is_specified(type_arg);
  }

  /**
    Function sets switch hint state.

    @param switch_state_arg  switch hint state
    @param type_arg          hint type
    @param check_parent      true if hint can be on parent level

    @return  true if hint is already specified,
             false otherwise
  */
  bool set_switch(bool switch_state_arg, opt_hints_enum type_arg,
                  bool check_parent) {
    if (is_specified(type_arg) ||
        (check_parent && parent->is_specified(type_arg)))
      return true;

    hints_map.set_switch(type_arg, switch_state_arg);
    return false;
  }

  /**
    Function returns switch hint state.

    @param type_arg          hint type

    @return  hint value if hint is specified,
            false otherwise
  */
  bool get_switch(opt_hints_enum type_arg) const;

  virtual const LEX_CSTRING *get_name() const { return name; }
  virtual const LEX_CSTRING *get_print_name() { return name; }
  void set_name(const LEX_CSTRING *name_arg) { name = name_arg; }
  Opt_hints *get_parent() const { return parent; }
  virtual void set_resolved() { resolved = true; }
  /**
    Returns 'resolved' flag value for depending on hint type.

    @param type_arg  hint type

    @return  true if all hint objects are resolved, false otherwise.
  */
  virtual bool is_resolved(opt_hints_enum type_arg MY_ATTRIBUTE((unused))) {
    return resolved;
  }
  /**
    Set hint to unresolved state.

    @param type_arg  hint type
  */
  virtual void set_unresolved(opt_hints_enum type_arg MY_ATTRIBUTE((unused))) {}
  /**
    If ignore_print() returns true, hint is not printed
    in Opt_hints::print() function. Atm used for
    INDEX_MERGE, SKIP_SCAN, INDEX, JOIN_INDEX, GROUP_INDEX
    ORDER_INDEX hints.

    @param type_arg  hint type

    @return  true if the hint should not be printed
    in Opt_hints::print() function, false otherwise.
  */
  virtual bool ignore_print(
      opt_hints_enum type_arg MY_ATTRIBUTE((unused))) const {
    return false;
  }
  void incr_resolved_children() { resolved_children++; }
  Mem_root_array<Opt_hints *> *child_array_ptr() { return &child_array; }

  bool is_all_resolved() const {
    return child_array.size() == resolved_children;
  }

  void register_child(Opt_hints *hint_arg) { child_array.push_back(hint_arg); }

  /**
    Returns pointer to complex hint for a given type.

    A complex hint is a hint that has arguments.
    (It is not just an on/off switch.)

    @param type  hint type

    @return  pointer to complex hint for a given type.
  */
  virtual PT_hint *get_complex_hints(
      opt_hints_enum type MY_ATTRIBUTE((unused))) {
    DBUG_ASSERT(0);
    return nullptr; /* error C4716: must return a value */
  }

  /**
    Find hint among lower-level hint objects.

    @param name_arg        hint name
    @param cs              Pointer to character set

    @return  hint if found,
             NULL otherwise
  */
  Opt_hints *find_by_name(const LEX_CSTRING *name_arg,
                          const CHARSET_INFO *cs) const;
  /**
    Print all hints except of QB_NAME hint.

    @param thd              Pointer to THD object
    @param str              Pointer to String object
    @param query_type       If query type is QT_NORMALIZED_FORMAT,
                            un-resolved hints will also be printed
  */
  void print(const THD *thd, String *str, enum_query_type query_type);
  /**
    Check if there are any unresolved hint objects and
    print warnings for them.

    @param thd             Pointer to THD object
  */
  void check_unresolved(THD *thd);
  virtual void append_name(const THD *thd, String *str) = 0;

 private:
  /**
    Append hint type.

    @param str             Pointer to String object
    @param type            Hint type
  */
  void append_hint_type(String *str, opt_hints_enum type);
  /**
    Print warning for unresolved hint name.

    @param thd             Pointer to THD object
  */
  void print_warn_unresolved(THD *thd);
  /**
    Function prints hints which are non-standard and don't
    fit into existing hint infrastructure.

    @param thd             pointer to THD object
    @param str             pointer to String object
  */
  virtual void print_irregular_hints(const THD *thd MY_ATTRIBUTE((unused)),
                                     String *str MY_ATTRIBUTE((unused))) {}
};

/**
  Global level hints.
*/

class Opt_hints_global : public Opt_hints {
 public:
  PT_hint_max_execution_time *max_exec_time;
  Sys_var_hint *sys_var_hint;

  Opt_hints_global(MEM_ROOT *mem_root_arg)
      : Opt_hints(nullptr, nullptr, mem_root_arg) {
    max_exec_time = nullptr;
    sys_var_hint = nullptr;
  }

  void append_name(const THD *, String *) override {}
  PT_hint *get_complex_hints(opt_hints_enum type) override;
  void print_irregular_hints(const THD *thd, String *str) override;
};

class PT_qb_level_hint;

/**
  Query block level hints.
*/

class Opt_hints_qb : public Opt_hints {
  uint select_number;    // SELECT_LEX number
  LEX_CSTRING sys_name;  // System QB name
  char buff[32];         // Buffer to hold sys name

  PT_qb_level_hint *subquery_hint, *semijoin_hint;

  /// Array of join order hints
  Mem_root_array<PT_qb_level_hint *> join_order_hints;
  /// Bit map of which hints are ignored.
  ulonglong join_order_hints_ignored;

  /*
    PT_qb_level_hint::contextualize sets subquery/semijoin_hint during parsing.
    it also registers join order hints during parsing.
  */
  friend class PT_qb_level_hint;

 public:
  Opt_hints_qb(Opt_hints *opt_hints_arg, MEM_ROOT *mem_root_arg,
               uint select_number_arg);

  const LEX_CSTRING *get_print_name() override {
    const LEX_CSTRING *str = Opt_hints::get_name();
    return str ? str : &sys_name;
  }

  /**
    Append query block hint.

    @param thd   pointer to THD object
    @param str   pointer to String object
  */
  void append_qb_hint(const THD *thd, String *str) {
    if (get_name()) {
      str->append(STRING_WITH_LEN("QB_NAME("));
      append_identifier(thd, str, get_name()->str, get_name()->length);
      str->append(STRING_WITH_LEN(") "));
    }
  }
  /**
    Append query block name.

    @param thd   pointer to THD object
    @param str   pointer to String object
  */
  void append_name(const THD *thd, String *str) override {
    str->append(STRING_WITH_LEN("@"));
    append_identifier(thd, str, get_print_name()->str,
                      get_print_name()->length);
  }

  PT_hint *get_complex_hints(opt_hints_enum type) override;

  /**
    Function finds Opt_hints_table object corresponding to
    table alias in the query block and attaches corresponding
    key hint objects to appropriate KEY structures.

    @param table  Table reference

    @return  pointer Opt_hints_table object if this object is found,
             NULL otherwise.
  */
  Opt_hints_table *adjust_table_hints(TABLE_LIST *table);

  /**
    Returns whether semi-join is enabled for this query block

    A SEMIJOIN hint will force semi-join regardless of optimizer_switch
    settings. A NO_SEMIJOIN hint will only turn off semi-join if the variant
    with no strategies is used. A SUBQUERY hint will turn off semi-join. If
    there is no SEMIJOIN/SUBQUERY hint, optimizer_switch setting determines
    whether SEMIJOIN is used.

    @param thd  Pointer to THD object for session.
                Used to access optimizer_switch

    @return true if semijoin is enabled
  */
  bool semijoin_enabled(THD *thd) const;

  /**
    Returns bit mask of which semi-join strategies are enabled for this query
    block.

    @param opt_switches Bit map of strategies enabled by optimizer_switch

    @return Bit mask of strategies that are enabled
  */
  uint sj_enabled_strategies(uint opt_switches) const;

  /**
    Returns which subquery execution strategy has been specified by hints
    for this query block.

    @retval EXEC_MATERIALIZATION  Subquery Materialization should be used
    @retval EXEC_EXISTS In-to-exists execution should be used
    @retval EXEC_UNSPECIFIED No SUBQUERY hint for this query block
  */
  SubqueryExecMethod subquery_strategy() const;

  void print_irregular_hints(const THD *thd, String *str) override;

  /**
    Checks if join order hints are applicable and
    applies table dependencies if possible.

    @param join JOIN object
  */
  void apply_join_order_hints(JOIN *join);

 private:
  void register_join_order_hint(PT_qb_level_hint *hint_arg) {
    join_order_hints.push_back(hint_arg);
  }
};

class PT_key_level_hint;

/**
  Auxiluary class for compound key objects.
*/
class Compound_key_hint {
  PT_key_level_hint *pt_hint;  // Pointer to PT_key_level_hint object.
  Key_map key_map;             // Indexes, specified in the hint.
  bool resolved;               // true if hint does not have unresolved index.

 public:
  Compound_key_hint() {
    key_map.init();
    resolved = false;
    pt_hint = nullptr;
  }

  virtual ~Compound_key_hint() {}

  void set_pt_hint(PT_key_level_hint *pt_hint_arg) { pt_hint = pt_hint_arg; }
  PT_key_level_hint *get_pt_hint() { return pt_hint; }

  void set_resolved(bool arg) { resolved = arg; }
  bool is_resolved() { return resolved; }

  void set_key_map(uint i) { key_map.set_bit(i); }
  bool is_set_key_map(uint i) { return key_map.is_set(i); }
  bool is_key_map_clear_all() { return key_map.is_clear_all(); }
  Key_map *get_key_map() { return &key_map; }
  virtual bool is_hint_conflicting(
      Opt_hints_table *table_hint MY_ATTRIBUTE((unused)),
      Opt_hints_key *key_hint MY_ATTRIBUTE((unused))) {
    return false;
  }
};

/**
  Auxiliary class for JOIN_INDEX, GROUP_INDEX, ORDER_INDEX hints.
*/
class Index_key_hint : public Compound_key_hint {
 public:
  bool is_hint_conflicting(Opt_hints_table *table_hint,
                           Opt_hints_key *key_hint) override;
};

/**
  Auxiliary class for INDEX hint.
*/
class Glob_index_key_hint : public Compound_key_hint {
 public:
  bool is_hint_conflicting(Opt_hints_table *table_hint,
                           Opt_hints_key *key_hint) override;
};

bool is_compound_hint(opt_hints_enum type_arg);

/**
  Table level hints.
*/

class Opt_hints_table : public Opt_hints {
 public:
  Mem_root_array<Opt_hints_key *> keyinfo_array;
  Compound_key_hint index_merge;
  Compound_key_hint skip_scan;
  Compound_key_hint lis_group_by;
  Glob_index_key_hint index;
  Index_key_hint join_index;
  Index_key_hint group_index;
  Index_key_hint order_index;

  Opt_hints_table(const LEX_CSTRING *table_name_arg, Opt_hints_qb *qb_hints_arg,
                  MEM_ROOT *mem_root_arg)
      : Opt_hints(table_name_arg, qb_hints_arg, mem_root_arg),
        keyinfo_array(mem_root_arg) {}

  /**
    Append table name.

    @param thd   pointer to THD object
    @param str   pointer to String object
  */
  void append_name(const THD *thd, String *str) override {
    append_identifier(thd, str, get_name()->str, get_name()->length);
    get_parent()->append_name(thd, str);
  }
  /**
    Function sets correlation between key hint objects and
    appropriate KEY structures.

    @param table      Pointer to TABLE_LIST object
  */
  void adjust_key_hints(TABLE_LIST *table);
  virtual PT_hint *get_complex_hints(opt_hints_enum type) override;

  void set_resolved() override {
    Opt_hints::set_resolved();
    if (is_specified(INDEX_MERGE_HINT_ENUM)) index_merge.set_resolved(true);
    if (is_specified(SKIP_SCAN_HINT_ENUM)) skip_scan.set_resolved(true);
    if (is_specified(GROUP_BY_LIS_HINT_ENUM)) lis_group_by.set_resolved(true);
    if (is_specified(INDEX_HINT_ENUM)) index.set_resolved(true);
    if (is_specified(JOIN_INDEX_HINT_ENUM)) join_index.set_resolved(true);
    if (is_specified(GROUP_INDEX_HINT_ENUM)) group_index.set_resolved(true);
    if (is_specified(ORDER_INDEX_HINT_ENUM)) order_index.set_resolved(true);
  }

  void set_unresolved(opt_hints_enum type_arg) override {
    if (is_specified(type_arg) && is_compound_hint(type_arg))
      get_compound_key_hint(type_arg)->set_resolved(false);
  }

  bool is_resolved(opt_hints_enum type_arg) override {
    if (is_compound_hint(type_arg))
      return Opt_hints::is_resolved(type_arg) &&
             get_compound_key_hint(type_arg)->is_resolved();
    return Opt_hints::is_resolved(type_arg);
  }

  void set_compound_key_hint_map(Opt_hints *hint, uint arg) {
    if (hint->is_specified(INDEX_MERGE_HINT_ENUM)) index_merge.set_key_map(arg);
    if (hint->is_specified(SKIP_SCAN_HINT_ENUM)) skip_scan.set_key_map(arg);
    if (hint->is_specified(GROUP_BY_LIS_HINT_ENUM))
      lis_group_by.set_key_map(arg);
    if (hint->is_specified(INDEX_HINT_ENUM)) index.set_key_map(arg);
    if (hint->is_specified(JOIN_INDEX_HINT_ENUM)) join_index.set_key_map(arg);
    if (hint->is_specified(GROUP_INDEX_HINT_ENUM)) group_index.set_key_map(arg);
    if (hint->is_specified(ORDER_INDEX_HINT_ENUM)) order_index.set_key_map(arg);
  }

  Compound_key_hint *get_compound_key_hint(opt_hints_enum type_arg) {
    if (type_arg == INDEX_MERGE_HINT_ENUM) return &index_merge;
    if (type_arg == SKIP_SCAN_HINT_ENUM) return &skip_scan;
    if (type_arg == GROUP_BY_LIS_HINT_ENUM) return &lis_group_by;
    if (type_arg == INDEX_HINT_ENUM) return &index;
    if (type_arg == JOIN_INDEX_HINT_ENUM) return &join_index;
    if (type_arg == GROUP_INDEX_HINT_ENUM) return &group_index;
    if (type_arg == ORDER_INDEX_HINT_ENUM) return &order_index;
    DBUG_ASSERT(0);
    return nullptr;
  }

  bool is_force_index_hint(opt_hints_enum type_arg) {
    return (get_compound_key_hint(type_arg)->is_resolved() &&
            get_switch(type_arg));
  }

  bool is_hint_conflicting(Opt_hints_key *key_hint, opt_hints_enum type);
  void update_index_hint_map(Key_map *keys_to_use,
                             Key_map *available_keys_to_use,
                             opt_hints_enum type_arg);
  bool update_index_hint_maps(THD *thd, TABLE *tbl);
};

/**
  Key level hints.
*/

class Opt_hints_key : public Opt_hints {
 public:
  Opt_hints_key(const LEX_CSTRING *key_name_arg,
                Opt_hints_table *table_hints_arg, MEM_ROOT *mem_root_arg)
      : Opt_hints(key_name_arg, table_hints_arg, mem_root_arg) {}

  /**
    Append key name.

    @param thd   pointer to THD object
    @param str   pointer to String object
  */
  void append_name(const THD *thd, String *str) override {
    get_parent()->append_name(thd, str);
    str->append(' ');
    append_identifier(thd, str, get_name()->str, get_name()->length);
  }
  /**
    Ignore printing of the object since parent complex hint has
    its own printing method.
  */
  bool ignore_print(opt_hints_enum type_arg) const override {
    return is_compound_hint(type_arg);
  }
};

/**
  Container for set_var object and original variable value.
*/

class Hint_set_var {
 public:
  Hint_set_var(set_var *var_arg) : var(var_arg), save_value(nullptr) {}
  set_var *var;      // Pointer to set_var object
  Item *save_value;  // Original variable value
};

/**
  SET_VAR hints.
*/

class Sys_var_hint {
  // List of str_var variables which need to be updated.
  Mem_root_array<Hint_set_var *> var_list;

 public:
  Sys_var_hint(MEM_ROOT *mem_root_arg) : var_list(mem_root_arg) {}
  /**
    Add variable to hint list.

    @param thd            pointer to THD object
    @param sys_var        pointer to sys_var object
    @param sys_var_value  variable value

    @return true if variable is added,
            false otherwise
  */
  bool add_var(THD *thd, sys_var *sys_var, Item *sys_var_value);
  /**
    Find variable in hint list.

    @param thd   Pointer to thread object
  */
  void update_vars(THD *thd);
  /**
    Restore system variables with original values.

    @param thd   Pointer to thread object
  */
  void restore_vars(THD *thd);
  /**
    Print applicable hints.

    @param thd   Thread handle
    @param str   Pointer to string object
  */
  void print(const THD *thd, String *str);
};

/**
  Returns key hint value if hint is specified, returns
  optimizer switch value if hint is not specified.

  @param thd               Pointer to THD object
  @param table             Pointer to TABLE_LIST object
  @param keyno             Key number
  @param type_arg          Hint type
  @param optimizer_switch  Optimizer switch flag

  @return key hint value if hint is specified,
          otherwise optimizer switch value.
*/
bool hint_key_state(const THD *thd, const TABLE_LIST *table, uint keyno,
                    opt_hints_enum type_arg, uint optimizer_switch);

/**
  Returns table hint value if hint is specified, returns
  optimizer switch value if hint is not specified.

  @param thd                Pointer to THD object
  @param table              Pointer to TABLE_LIST object
  @param type_arg           Hint type
  @param optimizer_switch   Optimizer switch flag

  @return table hint value if hint is specified,
          otherwise optimizer switch value.
*/
bool hint_table_state(const THD *thd, const TABLE_LIST *table,
                      opt_hints_enum type_arg, uint optimizer_switch);
/**
   Append table and query block name.

  @param thd        pointer to THD object
  @param str        pointer to String object
  @param qb_name    pointer to query block name, may be null
  @param table_name pointer to table name
*/
void append_table_name(const THD *thd, String *str, const LEX_CSTRING *qb_name,
                       const LEX_CSTRING *table_name);

/**
  Returns true if compound hint state is on with or without
  specified keys, otherwise returns false.
  If compound hint state is on and hint is specified without indexes,
  function returns 'true' for any 'keyno' argument. If hint specified
  with indexes, function returns true only for appropriate 'keyno' index.


  @param table              Pointer to TABLE object
  @param keyno              Key number
  @param type_arg           Hint type

  @return true if compound hint state is on with or without
          specified keys, otherwise returns false.
*/

bool compound_hint_key_enabled(const TABLE *table, uint keyno,
                               opt_hints_enum type_arg);

/**
  Returns true if index merge hint state is on otherwise returns false.

  @param table                     Pointer to TABLE object
  @param use_cheapest_index_merge  IN/OUT Returns true if INDEX_MERGE hint is
                                          used without any specified key.

  @return true if index merge hint state is on otherwise returns false.
*/

bool idx_merge_hint_state(const TABLE *table, bool *use_cheapest_index_merge);

int cmp_lex_string(const LEX_CSTRING *s, const LEX_CSTRING *t,
                   const CHARSET_INFO *cs);

#endif /* OPT_HINTS_INCLUDED */
