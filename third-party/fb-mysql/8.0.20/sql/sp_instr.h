/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _SP_INSTR_H_
#define _SP_INSTR_H_

#include <limits.h>
#include <string.h>
#include <sys/types.h>

#include "field_types.h"
#include "lex_string.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "sql/sql_class.h"  // Query_arena
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql_string.h"

class Item;
class Item_case_expr;
class Item_trigger_field;
class sp_condition_value;
class sp_handler;
class sp_head;
class sp_pcontext;
class sp_variable;
struct TABLE_LIST;

///////////////////////////////////////////////////////////////////////////
// This file contains SP-instruction classes.
///////////////////////////////////////////////////////////////////////////

/**
  sp_printable defines an interface which should be implemented if a class wants
  report some internal information about its state.
*/
class sp_printable {
 public:
  virtual void print(const THD *thd, String *str) = 0;

  virtual ~sp_printable() {}
};

/**
  An interface for all SP-instructions with destinations that
  need to be updated by the SP-optimizer.
*/
class sp_branch_instr {
 public:
  /**
    Update the destination; used by the SP-instruction-optimizer.

    @param old_dest current (old) destination (instruction pointer).
    @param new_dest new destination (instruction pointer).
  */
  virtual void set_destination(uint old_dest, uint new_dest) = 0;

  /**
    Update all instruction with the given label in the backpatch list to
    the specified instruction pointer.

    @param dest     destination instruction pointer.
  */
  virtual void backpatch(uint dest) = 0;

  virtual ~sp_branch_instr() {}
};

///////////////////////////////////////////////////////////////////////////

/**
  Base class for every SP-instruction. sp_instr defines interface and provides
  base implementation.
*/
class sp_instr : public sp_printable {
 public:
  sp_instr(uint ip, sp_pcontext *ctx)
      : m_arena(nullptr, Query_arena::STMT_INITIALIZED_FOR_SP),
        m_marked(false),
        m_ip(ip),
        m_parsing_ctx(ctx) {}

  virtual ~sp_instr() { m_arena.free_items(); }

  /**
    Execute this instruction

    @param thd         Thread context
    @param[out] nextp  index of the next instruction to execute. (For most
                       instructions this will be the instruction following this
                       one). Note that this parameter is undefined in case of
                       errors, use get_cont_dest() to find the continuation
                       instruction for CONTINUE error handlers.

    @return Error status.
  */
  virtual bool execute(THD *thd, uint *nextp) = 0;
#ifdef HAVE_PSI_INTERFACE
  virtual PSI_statement_info *get_psi_info() = 0;
#endif

  uint get_ip() const { return m_ip; }

  /**
    Get the continuation destination (instruction pointer for the CONTINUE
    HANDLER) of this instruction.
    @return the continuation destination
  */
  virtual uint get_cont_dest() const { return get_ip() + 1; }

  sp_pcontext *get_parsing_ctx() const { return m_parsing_ctx; }

 protected:
  /**
    Clear diagnostics area.
    @param thd         Thread context
  */
  void clear_da(THD *thd) const {
    thd->get_stmt_da()->reset_diagnostics_area();
    thd->get_stmt_da()->reset_condition_info(thd);
  }

  ///////////////////////////////////////////////////////////////////////////
  // The following operations are used solely for SP-code-optimizer.
  ///////////////////////////////////////////////////////////////////////////

 public:
  /**
    Mark this instruction as reachable during optimization and return the
    index to the next instruction. Jump instruction will add their
    destination to the leads list.
  */
  virtual uint opt_mark(sp_head *,
                        List<sp_instr> *leads MY_ATTRIBUTE((unused))) {
    m_marked = true;
    return get_ip() + 1;
  }

  /**
    Short-cut jumps to jumps during optimization. This is used by the
    jump instructions' opt_mark() methods. 'start' is the starting point,
    used to prevent the mark sweep from looping for ever. Return the
    end destination.
  */
  virtual uint opt_shortcut_jump(sp_head *,
                                 sp_instr *start MY_ATTRIBUTE((unused))) {
    return get_ip();
  }

  /**
    Inform the instruction that it has been moved during optimization.
    Most instructions will simply update its index, but jump instructions
    must also take care of their destination pointers. Forward jumps get
    pushed to the backpatch list 'ibp'.
  */
  virtual void opt_move(uint dst,
                        List<sp_branch_instr> *ibp MY_ATTRIBUTE((unused))) {
    m_ip = dst;
  }

  bool opt_is_marked() const { return m_marked; }

  virtual SQL_I_List<Item_trigger_field> *get_instr_trig_field_list() {
    return nullptr;
  }

  Query_arena m_arena;

 protected:
  /// Show if this instruction is reachable within the SP
  /// (used by SP-optimizer).
  bool m_marked;

  /// Instruction pointer.
  uint m_ip;

  /// Instruction parsing context.
  sp_pcontext *m_parsing_ctx;

 private:
  // Prevent use of copy constructor and assignment operator.
  sp_instr(const sp_instr &);
  void operator=(sp_instr &);
};

///////////////////////////////////////////////////////////////////////////

/**
  sp_lex_instr is a class providing the interface and base implementation
  for SP-instructions, whose execution is based on expression evaluation.

  sp_lex_instr keeps LEX-object to be able to evaluate the expression.

  sp_lex_instr also provides possibility to re-parse the original query
  string if for some reason the LEX-object is not valid any longer.
*/
class sp_lex_instr : public sp_instr {
 public:
  sp_lex_instr(uint ip, sp_pcontext *ctx, LEX *lex, bool is_lex_owner)
      : sp_instr(ip, ctx),
        m_lex(nullptr),
        m_is_lex_owner(false),
        m_first_execution(true),
        m_prelocking_tables(nullptr),
        m_lex_query_tables_own_last(nullptr) {
    set_lex(lex, is_lex_owner);
  }

  virtual ~sp_lex_instr() {
    free_lex();
    /*
      If the instruction is reparsed, m_lex_mem_root was used to allocate
      the items, then freeing the memroot, frees the items. Also free the
      items allocated on heap as well.
    */
    if (alloc_root_inited(&m_lex_mem_root)) m_arena.free_items();
  }

  /**
    Make a few attempts to execute the instruction.

    Basically, this operation does the following things:
      - install Reprepare_observer to catch metadata changes (if any);
      - calls reset_lex_and_exec_core() to execute the instruction;
      - if the execution fails due to a change in metadata, re-parse the
        instruction's SQL-statement and repeat execution.

    @param      thd           Thread context.
    @param[out] nextp         Next instruction pointer
    @param      open_tables   Flag to specify if the function should check read
                              access to tables in LEX's table list and open and
                              lock them (used in instructions which need to
                              calculate some expression and don't execute
                              complete statement).

    @return Error status.
  */
  bool validate_lex_and_execute_core(THD *thd, uint *nextp, bool open_tables);

  virtual SQL_I_List<Item_trigger_field> *get_instr_trig_field_list() {
    return &m_trig_field_list;
  }

 private:
  /**
    Prepare LEX and thread for execution of instruction, if requested open
    and lock LEX's tables, execute instruction's core function, perform
    cleanup afterwards.

    @param thd           thread context
    @param [out] nextp   next instruction pointer
    @param open_tables   if true then check read access to tables in LEX's table
                         list and open and lock them (used in instructions which
                         need to calculate some expression and don't execute
                         complete statement).

    @note
      We are not saving/restoring some parts of THD which may need this because
      we do this once for whole routine execution in sp_head::execute().

    @return Error status.
  */
  bool reset_lex_and_exec_core(THD *thd, uint *nextp, bool open_tables);

  /**
    (Re-)parse the query corresponding to this instruction and return a new
    LEX-object.

    @param thd  Thread context.
    @param sp   The stored program.

    @return new LEX-object or NULL in case of failure.
  */
  LEX *parse_expr(THD *thd, sp_head *sp);

  /**
     Set LEX-object.

     Previously assigned LEX-object (if any) will be properly cleaned up
     and destroyed.

     @param lex          LEX-object to be used by this instance of sp_lex_instr.
     @param is_lex_owner the flag specifying if this instance sp_lex_instr
                         owns (and thus deletes when needed) passed LEX-object.
  */
  void set_lex(LEX *lex, bool is_lex_owner);

  /**
     Cleanup and destroy assigned LEX-object if needed.
  */
  void free_lex();

 public:
  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool execute(THD *thd, uint *nextp) {
    /*
      SP instructions with expressions should clear DA before execution.
      Note that sp_instr_stmt will override execute(), but it clears DA
      during normal mysql_execute_command().
    */
    clear_da(thd);
    return validate_lex_and_execute_core(thd, nextp, true);
  }

 protected:
  /////////////////////////////////////////////////////////////////////////
  // Interface (virtual) methods.
  /////////////////////////////////////////////////////////////////////////

  /**
    Execute core function of instruction after all preparations
    (e.g. setting of proper LEX, saving part of the thread context).

    @param thd  Thread context.
    @param [out] nextp    next instruction pointer

    @return Error flag.
  */
  virtual bool exec_core(THD *thd, uint *nextp) = 0;

  /**
    @retval false if the object (i.e. LEX-object) is valid and exec_core() can
    be just called.

    @retval true if the object is not valid any longer, exec_core() can not be
    called. The original query string should be re-parsed and a new LEX-object
    should be used.
  */
  virtual bool is_invalid() const = 0;

  /**
    Invalidate the object.
  */
  virtual void invalidate() = 0;

  /**
    Return the query string, which can be passed to the parser. I.e. the
    operation should return a valid SQL-statement query string.

    @param[out] sql_query SQL-statement query string.
  */
  virtual void get_query(String *sql_query) const;

  /**
    @return the expression query string. This string can not be passed directly
    to the parser as it is most likely not a valid SQL-statement.

    @note as it can be seen in the get_query() implementation, get_expr_query()
    might return EMPTY_CSTR. EMPTY_CSTR means that no query-expression is
    available. That happens when class provides different implementation of
    get_query(). Strictly speaking, this is a drawback of the current class
    hierarchy.
  */
  virtual LEX_CSTRING get_expr_query() const { return EMPTY_CSTR; }

  /**
    Callback function which is called after the statement query string is
    successfully parsed, and the thread context has not been switched to the
    outer context. The thread context contains new LEX-object corresponding to
    the parsed query string.

    @param thd  Thread context.

    @return Error flag.
  */
  virtual bool on_after_expr_parsing(THD *thd MY_ATTRIBUTE((unused))) {
    return false;
  }

  /**
    Destroy items in the free list before re-parsing the statement query
    string (and thus, creating new items).

    @param thd  Thread context.
  */
  virtual void cleanup_before_parsing(THD *thd);

  /// LEX-object.
  LEX *m_lex;

 private:
  /**
    Mem-root for storing the LEX-tree during reparse. This
    mem-root is freed when a reparse is triggered or the stored
    routine is dropped.
  */
  MEM_ROOT m_lex_mem_root;

  /**
    Indicates whether this sp_lex_instr instance is responsible for
    LEX-object deletion.
  */
  bool m_is_lex_owner;

  /**
    Indicates whether exec_core() has not been already called on the current
    LEX-object.
  */
  bool m_first_execution;

  /*****************************************************************************
    Support for being able to execute this statement in two modes:
    a) inside prelocked mode set by the calling procedure or its ancestor.
    b) outside of prelocked mode, when this statement enters/leaves
       prelocked mode itself.
  *****************************************************************************/

  /**
    List of additional tables this statement needs to lock when it
    enters/leaves prelocked mode on its own.
  */
  TABLE_LIST *m_prelocking_tables;

  /**
    The value m_lex->query_tables_own_last should be set to this when the
    statement enters/leaves prelocked mode on its own.
  */
  TABLE_LIST **m_lex_query_tables_own_last;

  /**
    List of all the Item_trigger_field's of instruction.
  */
  SQL_I_List<Item_trigger_field> m_trig_field_list;
};

///////////////////////////////////////////////////////////////////////////

/**
  sp_instr_stmt represents almost all conventional SQL-statements, which are
  supported outside stored programs.

  SET-statements, which deal with SP-variable or NEW/OLD trigger pseudo-rows are
  not represented by this instruction.
*/
class sp_instr_stmt : public sp_lex_instr {
 public:
  sp_instr_stmt(uint ip, LEX *lex, LEX_CSTRING query)
      : sp_lex_instr(ip, lex->get_sp_current_parsing_ctx(), lex, true),
        m_query(query),
        m_valid(true) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool execute(THD *thd, uint *nextp);

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_lex_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool exec_core(THD *thd, uint *nextp);

  virtual bool is_invalid() const { return !m_valid; }

  virtual void invalidate() { m_valid = false; }

  virtual void get_query(String *sql_query) const {
    sql_query->append(m_query.str, m_query.length);
  }

  virtual bool on_after_expr_parsing(THD *) {
    m_valid = true;
    return false;
  }

 private:
  /// Complete query of the SQL-statement.
  LEX_CSTRING m_query;

  /// Specify if the stored LEX-object is up-to-date.
  bool m_valid;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////

/**
  sp_instr_set represents SET-statements, which deal with SP-variables.
*/
class sp_instr_set : public sp_lex_instr {
 public:
  sp_instr_set(uint ip, LEX *lex, uint offset, Item *value_item,
               LEX_CSTRING value_query, bool is_lex_owner)
      : sp_lex_instr(ip, lex->get_sp_current_parsing_ctx(), lex, is_lex_owner),
        m_offset(offset),
        m_value_item(value_item),
        m_value_query(value_query) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_lex_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool exec_core(THD *thd, uint *nextp);

  virtual bool is_invalid() const { return m_value_item == nullptr; }

  virtual void invalidate() { m_value_item = nullptr; }

  virtual bool on_after_expr_parsing(THD *thd) {
    DBUG_ASSERT(thd->lex->select_lex->item_list.elements == 1);

    m_value_item = thd->lex->select_lex->item_list.head();

    return false;
  }

  virtual LEX_CSTRING get_expr_query() const { return m_value_query; }

 private:
  /// Frame offset.
  uint m_offset;

  /// Value expression item of the SET-statement.
  Item *m_value_item;

  /// SQL-query corresponding to the value expression.
  LEX_CSTRING m_value_query;

#ifdef HAVE_PSI_INTERFACE
 public:
  static PSI_statement_info psi_info;
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }
#endif
};

///////////////////////////////////////////////////////////////////////////

/**
  sp_instr_set_trigger_field represents SET-statements, which deal with NEW/OLD
  trigger pseudo-rows.
*/
class sp_instr_set_trigger_field : public sp_lex_instr {
 public:
  sp_instr_set_trigger_field(uint ip, LEX *lex, LEX_CSTRING trigger_field_name,
                             Item_trigger_field *trigger_field,
                             Item *value_item, LEX_CSTRING value_query)
      : sp_lex_instr(ip, lex->get_sp_current_parsing_ctx(), lex, true),
        m_trigger_field_name(trigger_field_name),
        m_trigger_field(trigger_field),
        m_value_item(value_item),
        m_value_query(value_query) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_lex_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool exec_core(THD *thd, uint *nextp);

  virtual bool is_invalid() const { return m_value_item == nullptr; }

  virtual void invalidate() { m_value_item = nullptr; }

  virtual bool on_after_expr_parsing(THD *thd);

  virtual void cleanup_before_parsing(THD *thd);

  virtual LEX_CSTRING get_expr_query() const { return m_value_query; }

 private:
  /// Trigger field name ("field_name" of the "NEW.field_name").
  LEX_CSTRING m_trigger_field_name;

  /// Item corresponding to the NEW/OLD trigger field.
  Item_trigger_field *m_trigger_field;

  /// Value expression item of the SET-statement.
  Item *m_value_item;

  /// SQL-query corresponding to the value expression.
  LEX_CSTRING m_value_query;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////

/**
  sp_instr_freturn represents RETURN statement in stored functions.
*/
class sp_instr_freturn : public sp_lex_instr {
 public:
  sp_instr_freturn(uint ip, LEX *lex, Item *expr_item, LEX_CSTRING expr_query,
                   enum enum_field_types return_field_type)
      : sp_lex_instr(ip, lex->get_sp_current_parsing_ctx(), lex, true),
        m_expr_item(expr_item),
        m_expr_query(expr_query),
        m_return_field_type(return_field_type) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual uint opt_mark(sp_head *, List<sp_instr> *) {
    m_marked = true;
    return UINT_MAX;
  }

  /////////////////////////////////////////////////////////////////////////
  // sp_lex_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool exec_core(THD *thd, uint *nextp);

  virtual bool is_invalid() const { return m_expr_item == nullptr; }

  virtual void invalidate() {
    // it's already deleted.
    m_expr_item = nullptr;
  }

  virtual bool on_after_expr_parsing(THD *thd) {
    DBUG_ASSERT(thd->lex->select_lex->item_list.elements == 1);

    m_expr_item = thd->lex->select_lex->item_list.head();

    return false;
  }

  virtual LEX_CSTRING get_expr_query() const { return m_expr_query; }

 private:
  /// RETURN-expression item.
  Item *m_expr_item;

  /// SQL-query corresponding to the RETURN-expression.
  LEX_CSTRING m_expr_query;

  /// RETURN-field type code.
  enum enum_field_types m_return_field_type;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////

/**
  This is base class for all kinds of jump instructions.

  @note this is the only class, we directly construct instances of, that has
  subclasses. We also redefine sp_instr_jump behavior in those subclasses.

  @todo later we will consider introducing a new class, which will be the base
  for sp_instr_jump, sp_instr_set_case_expr and sp_instr_jump_case_when.
  Something like sp_regular_branch_instr (similar to sp_lex_branch_instr).
*/
class sp_instr_jump : public sp_instr, public sp_branch_instr {
 public:
  sp_instr_jump(uint ip, sp_pcontext *ctx)
      : sp_instr(ip, ctx), m_dest(0), m_optdest(nullptr) {}

  sp_instr_jump(uint ip, sp_pcontext *ctx, uint dest)
      : sp_instr(ip, ctx), m_dest(dest), m_optdest(nullptr) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool execute(THD *, uint *nextp) {
    *nextp = m_dest;
    return false;
  }

  virtual uint opt_mark(sp_head *sp, List<sp_instr> *leads);

  virtual uint opt_shortcut_jump(sp_head *sp, sp_instr *start);

  virtual void opt_move(uint dst, List<sp_branch_instr> *ibp);

  /////////////////////////////////////////////////////////////////////////
  // sp_branch_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void set_destination(uint old_dest, uint new_dest) {
    if (m_dest == old_dest) m_dest = new_dest;
  }

  virtual void backpatch(uint dest) {
    /* Calling backpatch twice is a logic flaw in jump resolution. */
    DBUG_ASSERT(m_dest == 0);
    m_dest = dest;
  }

 protected:
  /// Where we will go.
  uint m_dest;

  // The following attribute is used by SP-optimizer.
  sp_instr *m_optdest;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////

/**
  sp_lex_branch_instr is a base class for SP-instructions, which might perform
  conditional jump depending on the value of an SQL-expression.
*/
class sp_lex_branch_instr : public sp_lex_instr, public sp_branch_instr {
 protected:
  sp_lex_branch_instr(uint ip, sp_pcontext *ctx, LEX *lex, Item *expr_item,
                      LEX_CSTRING expr_query)
      : sp_lex_instr(ip, ctx, lex, true),
        m_dest(0),
        m_cont_dest(0),
        m_optdest(nullptr),
        m_cont_optdest(nullptr),
        m_expr_item(expr_item),
        m_expr_query(expr_query) {}

  sp_lex_branch_instr(uint ip, sp_pcontext *ctx, LEX *lex, Item *expr_item,
                      LEX_CSTRING expr_query, uint dest)
      : sp_lex_instr(ip, ctx, lex, true),
        m_dest(dest),
        m_cont_dest(0),
        m_optdest(nullptr),
        m_cont_optdest(nullptr),
        m_expr_item(expr_item),
        m_expr_query(expr_query) {}

 public:
  void set_cont_dest(uint cont_dest) { m_cont_dest = cont_dest; }

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual uint opt_mark(sp_head *sp, List<sp_instr> *leads);

  virtual void opt_move(uint dst, List<sp_branch_instr> *ibp);

  virtual uint get_cont_dest() const { return m_cont_dest; }

  /////////////////////////////////////////////////////////////////////////
  // sp_lex_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool is_invalid() const { return m_expr_item == nullptr; }

  virtual void invalidate() {
    m_expr_item = nullptr; /* it's already deleted. */
  }

  virtual LEX_CSTRING get_expr_query() const { return m_expr_query; }

  /////////////////////////////////////////////////////////////////////////
  // sp_branch_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void set_destination(uint old_dest, uint new_dest) {
    if (m_dest == old_dest) m_dest = new_dest;

    if (m_cont_dest == old_dest) m_cont_dest = new_dest;
  }

  virtual void backpatch(uint dest) {
    /* Calling backpatch twice is a logic flaw in jump resolution. */
    DBUG_ASSERT(m_dest == 0);
    m_dest = dest;
  }

 protected:
  /// Where we will go.
  uint m_dest;

  /// Where continue handlers will go.
  uint m_cont_dest;

  // The following attributes are used by SP-optimizer.
  sp_instr *m_optdest;
  sp_instr *m_cont_optdest;

  /// Expression item.
  Item *m_expr_item;

  /// SQL-query corresponding to the expression.
  LEX_CSTRING m_expr_query;
};

///////////////////////////////////////////////////////////////////////////

/**
  sp_instr_jump_if_not implements SP-instruction, which does the jump if its
  SQL-expression is false.
*/
class sp_instr_jump_if_not : public sp_lex_branch_instr {
 public:
  sp_instr_jump_if_not(uint ip, LEX *lex, Item *expr_item,
                       LEX_CSTRING expr_query)
      : sp_lex_branch_instr(ip, lex->get_sp_current_parsing_ctx(), lex,
                            expr_item, expr_query) {}

  sp_instr_jump_if_not(uint ip, LEX *lex, Item *expr_item,
                       LEX_CSTRING expr_query, uint dest)
      : sp_lex_branch_instr(ip, lex->get_sp_current_parsing_ctx(), lex,
                            expr_item, expr_query, dest) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_lex_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool exec_core(THD *thd, uint *nextp);

  virtual bool on_after_expr_parsing(THD *thd) {
    DBUG_ASSERT(thd->lex->select_lex->item_list.elements == 1);

    m_expr_item = thd->lex->select_lex->item_list.head();

    return false;
  }

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////
// Instructions used for the "simple CASE" implementation.
///////////////////////////////////////////////////////////////////////////

/**
  sp_instr_set_case_expr is used in the "simple CASE" implementation to evaluate
  and store the CASE-expression in the runtime context.
*/
class sp_instr_set_case_expr : public sp_lex_branch_instr {
 public:
  sp_instr_set_case_expr(uint ip, LEX *lex, uint case_expr_id,
                         Item *case_expr_item, LEX_CSTRING case_expr_query)
      : sp_lex_branch_instr(ip, lex->get_sp_current_parsing_ctx(), lex,
                            case_expr_item, case_expr_query),
        m_case_expr_id(case_expr_id) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual uint opt_mark(sp_head *sp, List<sp_instr> *leads);

  virtual void opt_move(uint dst, List<sp_branch_instr> *ibp);

  /////////////////////////////////////////////////////////////////////////
  // sp_branch_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  /*
    NOTE: set_destination() and backpatch() are overriden here just because the
    m_dest attribute is not used by this class, so there is no need to do
    anything about it.

    @todo These operations probably should be left as they are (i.e. do not
    override them here). The m_dest attribute would be set and not used, but
    that should not be a big deal.

    @todo This also indicates deficiency of the current SP-istruction class
    hierarchy.
  */

  virtual void set_destination(uint old_dest, uint new_dest) {
    if (m_cont_dest == old_dest) m_cont_dest = new_dest;
  }

  virtual void backpatch(uint) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_lex_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool exec_core(THD *thd, uint *nextp);

  virtual bool on_after_expr_parsing(THD *thd) {
    DBUG_ASSERT(thd->lex->select_lex->item_list.elements == 1);

    m_expr_item = thd->lex->select_lex->item_list.head();

    return false;
  }

 private:
  /// Identifier (index) of the CASE-expression in the runtime context.
  uint m_case_expr_id;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////

/**
  sp_instr_jump_case_when instruction is used in the "simple CASE"
  implementation. It's a jump instruction with the following condition:
    (CASE-expression = WHEN-expression)
  CASE-expression is retrieved from sp_rcontext;
  WHEN-expression is kept by this instruction.
*/
class sp_instr_jump_case_when : public sp_lex_branch_instr {
 public:
  sp_instr_jump_case_when(uint ip, LEX *lex, int case_expr_id,
                          Item *when_expr_item, LEX_CSTRING when_expr_query)
      : sp_lex_branch_instr(ip, lex->get_sp_current_parsing_ctx(), lex,
                            when_expr_item, when_expr_query),
        m_case_expr_id(case_expr_id) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_lex_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool exec_core(THD *thd, uint *nextp);

  virtual void invalidate() {
    // Items should be already deleted in lex-keeper.
    m_case_expr_item = nullptr;
    m_eq_item = nullptr;
    m_expr_item = nullptr;  // it's a WHEN-expression.
  }

  /**
    Build CASE-expression item tree:
      Item_func_eq(case-expression, when-i-expression)

    This function is used for the following form of CASE statement:
      CASE case-expression
        WHEN when-1-expression THEN ...
        WHEN when-2-expression THEN ...
        ...
        WHEN when-n-expression THEN ...
      END CASE

    The thing is that after the parsing we have an item (item tree) for the
    case-expression and for each when-expression. Here we build jump
    conditions: expressions like (case-expression = when-i-expression).

    @param thd  Thread context.

    @return Error flag.
  */
  virtual bool on_after_expr_parsing(THD *thd);

 private:
  /// Identifier (index) of the CASE-expression in the runtime context.
  int m_case_expr_id;

  /// Item representing the CASE-expression.
  Item_case_expr *m_case_expr_item;

  /**
    Item corresponding to the main item of the jump-condition-expression:
    it's the equal function (=) in the (case-expression = when-i-expression)
    expression.
  */
  Item *m_eq_item;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////
// SQL-condition handler instructions.
///////////////////////////////////////////////////////////////////////////

class sp_instr_hpush_jump : public sp_instr_jump {
 public:
  sp_instr_hpush_jump(uint ip, sp_pcontext *ctx, sp_handler *handler);

  virtual ~sp_instr_hpush_jump();

  void add_condition(sp_condition_value *condition_value);

  sp_handler *get_handler() { return m_handler; }

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool execute(THD *thd, uint *nextp);

  virtual uint opt_mark(sp_head *sp, List<sp_instr> *leads);

  /** Override sp_instr_jump's shortcut; we stop here. */
  virtual uint opt_shortcut_jump(sp_head *, sp_instr *) { return get_ip(); }

  /////////////////////////////////////////////////////////////////////////
  // sp_branch_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void backpatch(uint dest) {
    DBUG_ASSERT(!m_dest || !m_opt_hpop);
    if (!m_dest)
      m_dest = dest;
    else
      m_opt_hpop = dest;
  }

 private:
  /// Handler.
  sp_handler *m_handler;

  /// hpop marking end of handler scope.
  uint m_opt_hpop;

  // This attribute is needed for SHOW PROCEDURE CODE only (i.e. it's needed in
  // debug version only). It's used in print().
  uint m_frame;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////

class sp_instr_hpop : public sp_instr {
 public:
  sp_instr_hpop(uint ip, sp_pcontext *ctx) : sp_instr(ip, ctx) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *, String *str) {
    str->append(STRING_WITH_LEN("hpop"));
  }

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool execute(THD *thd, uint *nextp);

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////

class sp_instr_hreturn : public sp_instr_jump {
 public:
  sp_instr_hreturn(uint ip, sp_pcontext *ctx);

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool execute(THD *thd, uint *nextp);

  /** Override sp_instr_jump's shortcut; we stop here. */
  virtual uint opt_shortcut_jump(sp_head *, sp_instr *) { return get_ip(); }

  virtual uint opt_mark(sp_head *sp, List<sp_instr> *leads);

 private:
  // This attribute is needed for SHOW PROCEDURE CODE only (i.e. it's needed in
  // debug version only). It's used in print().
  uint m_frame;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////
// Cursor implementation.
///////////////////////////////////////////////////////////////////////////

/**
  sp_instr_cpush corresponds to DECLARE CURSOR, implements DECLARE CURSOR and
  OPEN.

  This is the most important instruction in cursor implementation. It is created
  and added to sp_head when DECLARE CURSOR is being parsed. The arena of this
  instruction contains LEX-object for the cursor's SELECT-statement.

  This instruction is actually used to open the cursor.

  execute() operation "implements" DECLARE CURSOR statement -- it merely pushes
  a new cursor object into the stack in sp_rcontext object.

  exec_core() operation implements OPEN statement. It is important to implement
  OPEN statement in this instruction, because OPEN may lead to re-parsing of the
  SELECT-statement. So, the original Arena and parsing context must be used.
*/
class sp_instr_cpush : public sp_lex_instr {
 public:
  sp_instr_cpush(uint ip, sp_pcontext *ctx, LEX *cursor_lex,
                 LEX_CSTRING cursor_query, int cursor_idx)
      : sp_lex_instr(ip, ctx, cursor_lex, true),
        m_cursor_query(cursor_query),
        m_valid(true),
        m_cursor_idx(cursor_idx) {
    /*
      Cursors cause queries to depend on external state, so they are
      noncacheable.
    */
    cursor_lex->safe_to_cache_query = false;
  }

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool execute(THD *thd, uint *nextp);

  /////////////////////////////////////////////////////////////////////////
  // sp_lex_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool exec_core(THD *thd, uint *nextp);

  virtual bool is_invalid() const { return !m_valid; }

  virtual void invalidate() { m_valid = false; }

  virtual void get_query(String *sql_query) const {
    sql_query->append(m_cursor_query.str, m_cursor_query.length);
  }

  virtual bool on_after_expr_parsing(THD *) {
    m_valid = true;
    return false;
  }

 private:
  /// This attribute keeps the cursor SELECT statement.
  LEX_CSTRING m_cursor_query;

  /// Flag if the LEX-object of this instruction is valid or not.
  /// The LEX-object is not valid when metadata have changed.
  bool m_valid;

  /// Used to identify the cursor in the sp_rcontext.
  int m_cursor_idx;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////

/**
  sp_instr_cpop instruction is added at the end of BEGIN..END block.
  It's used to remove declared cursors so that they are not visible any longer.
*/
class sp_instr_cpop : public sp_instr {
 public:
  sp_instr_cpop(uint ip, sp_pcontext *ctx, uint count)
      : sp_instr(ip, ctx), m_count(count) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool execute(THD *thd, uint *nextp);

 private:
  uint m_count;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////

/**
  sp_instr_copen represents OPEN statement (opens the cursor).
  However, the actual implementation is in sp_instr_cpush::exec_core().
*/
class sp_instr_copen : public sp_instr {
 public:
  sp_instr_copen(uint ip, sp_pcontext *ctx, int cursor_idx)
      : sp_instr(ip, ctx), m_cursor_idx(cursor_idx) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool execute(THD *thd, uint *nextp);

 private:
  /// Used to identify the cursor in the sp_rcontext.
  int m_cursor_idx;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////

/**
  The instruction corresponds to the CLOSE statement.
  It just forwards the close-call to the appropriate sp_cursor object in the
  sp_rcontext.
*/
class sp_instr_cclose : public sp_instr {
 public:
  sp_instr_cclose(uint ip, sp_pcontext *ctx, int cursor_idx)
      : sp_instr(ip, ctx), m_cursor_idx(cursor_idx) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool execute(THD *thd, uint *nextp);

 private:
  /// Used to identify the cursor in the sp_rcontext.
  int m_cursor_idx;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////

/**
  The instruction corresponds to the FETCH statement.
  It just forwards the close-call to the appropriate sp_cursor object in the
  sp_rcontext.
*/
class sp_instr_cfetch : public sp_instr {
 public:
  sp_instr_cfetch(uint ip, sp_pcontext *ctx, int cursor_idx)
      : sp_instr(ip, ctx), m_cursor_idx(cursor_idx) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool execute(THD *thd, uint *nextp);

  void add_to_varlist(sp_variable *var) { m_varlist.push_back(var); }

 private:
  /// List of SP-variables to store fetched values.
  List<sp_variable> m_varlist;

  /// Used to identify the cursor in the sp_rcontext.
  int m_cursor_idx;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/**
  sp_instr_error just throws an SQL-condition if the execution flow comes to it.
  It's used in the CASE implementation to perform runtime-check that the
  CASE-expression is handled by some WHEN/ELSE clause.
*/
class sp_instr_error : public sp_instr {
 public:
  sp_instr_error(uint ip, sp_pcontext *ctx, int errcode)
      : sp_instr(ip, ctx), m_errcode(errcode) {}

  /////////////////////////////////////////////////////////////////////////
  // sp_printable implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual void print(const THD *thd, String *str);

  /////////////////////////////////////////////////////////////////////////
  // sp_instr implementation.
  /////////////////////////////////////////////////////////////////////////

  virtual bool execute(THD *, uint *nextp) {
    my_error(m_errcode, MYF(0));
    *nextp = get_ip() + 1;
    return true;
  }

  virtual uint opt_mark(sp_head *, List<sp_instr> *) {
    m_marked = true;
    return UINT_MAX;
  }

 private:
  /// The error code, which should be raised by this instruction.
  int m_errcode;

#ifdef HAVE_PSI_INTERFACE
 public:
  virtual PSI_statement_info *get_psi_info() { return &psi_info; }

  static PSI_statement_info psi_info;
#endif
};

///////////////////////////////////////////////////////////////////////////

#endif  // _SP_INSTR_H_
