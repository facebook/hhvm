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

#ifndef PARSE_TREE_HINTS_INCLUDED
#define PARSE_TREE_HINTS_INCLUDED

#include <sys/types.h>

#include "lex_string.h"
#include "my_compiler.h"
#include "sql/mem_root_array.h"
#include "sql/opt_hints.h"
#include "sql/parse_tree_node_base.h"
#include "sql/sql_show.h"
#include "sql_string.h"

class Item;
class THD;
struct MEM_ROOT;

struct Hint_param_table {
  LEX_CSTRING table;
  LEX_CSTRING opt_query_block;
};

typedef Mem_root_array_YY<LEX_CSTRING> Hint_param_index_list;
typedef Mem_root_array_YY<Hint_param_table> Hint_param_table_list;

/**
  The class is a base class for representation of the
  different types of the hints. For the complex hints
  it is also used as a container for additional argumnets.
*/
class PT_hint : public Parse_tree_node {
  opt_hints_enum hint_type;  // Hint type
  bool state;                // true if hints is on, false otherwise
 public:
  PT_hint(opt_hints_enum hint_type_arg, bool switch_state_arg)
      : hint_type(hint_type_arg), state(switch_state_arg) {}

  opt_hints_enum type() const { return hint_type; }
  bool switch_on() const { return state; }
  /**
    Print warning issuing in processing of the hint.

    @param thd             Pointer to THD object
    @param err_code        Error code
    @param qb_name_arg     QB name
    @param table_name_arg  table name
    @param key_name_arg    key name
    @param hint            Pointer to hint object
  */
  virtual void print_warn(THD *thd, uint err_code,
                          const LEX_CSTRING *qb_name_arg,
                          LEX_CSTRING *table_name_arg,
                          LEX_CSTRING *key_name_arg, PT_hint *hint) const;
  /**
    Append additional hint arguments.

    @param thd             Pointer to THD object
    @param str             Pointer to String object
  */
  virtual void append_args(const THD *thd MY_ATTRIBUTE((unused)),
                           String *str MY_ATTRIBUTE((unused))) const {}
  bool supports_view() {
    /*
      Only index hints are supported to be used in views.
      Other hints can be added separately.
    */
    return (type() >= INDEX_HINT_ENUM && type() <= ORDER_INDEX_HINT_ENUM);
  }
};

class PT_hint_list : public Parse_tree_node {
  typedef Parse_tree_node super;

  Mem_root_array<PT_hint *> hints;

 public:
  explicit PT_hint_list(MEM_ROOT *mem_root) : hints(mem_root) {}

  /**
    Function handles list of the hints we get after
    parse procedure. It also creates query block hint
    object(Opt_hints_qb) if it does not exists.

    @param pc   Pointer to Parse_context object

    @return  true in case of error,
             false otherwise
  */
  bool contextualize(Parse_context *pc) override;

  bool push_back(PT_hint *hint) { return hints.push_back(hint); }
};

/**
  Parse tree hint object for query block level hints.
*/
class PT_qb_level_hint : public PT_hint {
  /** Name of query block. */
  const LEX_CSTRING qb_name;
  /** Bit mask of arguments to hint. */
  uint args;
  /** List of tables specified in join order hint */
  Hint_param_table_list table_list;

  typedef PT_hint super;

 public:
  PT_qb_level_hint(const LEX_CSTRING qb_name_arg, bool switch_state_arg,
                   enum opt_hints_enum hint_type_arg, uint arg)
      : PT_hint(hint_type_arg, switch_state_arg),
        qb_name(qb_name_arg),
        args(arg) {}

  PT_qb_level_hint(const LEX_CSTRING qb_name_arg, bool switch_state_arg,
                   enum opt_hints_enum hint_type_arg,
                   const Hint_param_table_list &table_list_arg)
      : PT_hint(hint_type_arg, switch_state_arg),
        qb_name(qb_name_arg),
        args(0),
        table_list(table_list_arg) {}

  uint get_args() const { return args; }

  /**
    Function handles query block level hint. It also creates query block hint
    object (Opt_hints_qb) if it does not exist.

    @param pc  Pointer to Parse_context object

    @return  true in case of error,
             false otherwise
  */
  bool contextualize(Parse_context *pc) override;

  /**
    Append hint arguments to given string

    @param thd             Pointer to THD object
    @param str             Pointer to String object
  */
  void append_args(const THD *thd, String *str) const override;
  virtual Hint_param_table_list *get_table_list() { return &table_list; }
};

/**
  Parse tree hint object for table level hints.
*/

class PT_table_level_hint : public PT_hint {
  const LEX_CSTRING qb_name;
  Hint_param_table_list table_list;

  typedef PT_hint super;

 public:
  PT_table_level_hint(const LEX_CSTRING qb_name_arg,
                      const Hint_param_table_list &table_list_arg,
                      bool switch_state_arg, opt_hints_enum hint_type_arg)
      : PT_hint(hint_type_arg, switch_state_arg),
        qb_name(qb_name_arg),
        table_list(table_list_arg) {}

  /**
    Function handles table level hint. It also creates
    table hint object (Opt_hints_table) if it does not
    exist.

    @param pc  Pointer to Parse_context object

    @return  true in case of error,
             false otherwise
  */
  bool contextualize(Parse_context *pc) override;
};

/**
  Parse tree hint object for key level hints.
*/

class PT_key_level_hint : public PT_hint {
  Hint_param_table table_name;
  Hint_param_index_list key_list;

  typedef PT_hint super;

 public:
  PT_key_level_hint(Hint_param_table &table_name_arg,
                    const Hint_param_index_list &key_list_arg,
                    bool switch_state_arg, opt_hints_enum hint_type_arg)
      : PT_hint(hint_type_arg, switch_state_arg),
        table_name(table_name_arg),
        key_list(key_list_arg) {}

  /**
    Function handles key level hint.
    It also creates key hint object
    (Opt_hints_key) if it does not
    exist.

    @param pc  Pointer to Parse_context object

    @return  true in case of error,
             false otherwise
  */
  bool contextualize(Parse_context *pc) override;
  void append_args(const THD *thd, String *str) const override;
};

/**
  Parse tree hint object for QB_NAME hint.
*/

class PT_hint_qb_name : public PT_hint {
  const LEX_CSTRING qb_name;

  typedef PT_hint super;

 public:
  PT_hint_qb_name(const LEX_CSTRING qb_name_arg)
      : PT_hint(QB_NAME_HINT_ENUM, true), qb_name(qb_name_arg) {}

  /**
    Function sets query block name.

    @param pc  Pointer to Parse_context object

    @return  true in case of error,
             false otherwise
  */
  bool contextualize(Parse_context *pc) override;
  void append_args(const THD *thd, String *str) const override {
    append_identifier(thd, str, qb_name.str, qb_name.length);
  }
};

/**
  Parse tree hint object for MAX_EXECUTION_TIME hint.
*/

class PT_hint_max_execution_time : public PT_hint {
  typedef PT_hint super;

 public:
  ulong milliseconds;

  explicit PT_hint_max_execution_time(ulong milliseconds_arg)
      : PT_hint(MAX_EXEC_TIME_HINT_ENUM, true),
        milliseconds(milliseconds_arg) {}
  /**
    Function initializes MAX_EXECUTION_TIME hint

    @param pc   Pointer to Parse_context object

    @return  true in case of error,
             false otherwise
  */
  bool contextualize(Parse_context *pc) override;
  void append_args(const THD *, String *str) const override {
    str->append_ulonglong(milliseconds);
  }
};

class PT_hint_sys_var : public PT_hint {
  const LEX_CSTRING sys_var_name;
  Item *sys_var_value;

  typedef PT_hint super;

 public:
  explicit PT_hint_sys_var(const LEX_CSTRING sys_var_name_arg,
                           Item *sys_var_value_arg)
      : PT_hint(MAX_HINT_ENUM, true),
        sys_var_name(sys_var_name_arg),
        sys_var_value(sys_var_value_arg) {}
  /**
    Function initializes SET_VAR hint.

    @param pc   Pointer to Parse_context object

    @return  true in case of error,
             false otherwise
  */
  bool contextualize(Parse_context *pc) override;
};

/**
  Parse tree hint object for RESOURCE_GROUP hint.
*/

class PT_hint_resource_group : public PT_hint {
  const LEX_CSTRING m_resource_group_name;

  typedef PT_hint super;

 public:
  PT_hint_resource_group(const LEX_CSTRING &name)
      : PT_hint(RESOURCE_GROUP_HINT_ENUM, true), m_resource_group_name(name) {}

  /**
    Function initializes resource group name and checks for presence of
    resource group. Also it checks for invocation of hint from stored
    routines or sub query.

     @param pc Pointer to Parse_context object

     @return true in case of error,
             false otherwise
  */

  bool contextualize(Parse_context *pc) override;

  /**
    Append hint arguments to given string.

    @param thd      Pointer to THD object.
    @param str      Pointer to String object.
  */

  void append_args(const THD *thd, String *str) const override {
    append_identifier(thd, str, m_resource_group_name.str,
                      m_resource_group_name.length);
  }
};

#endif /* PARSE_TREE_HINTS_INCLUDED */
