/* Copyright (c) 2002, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _SP_RCONTEXT_H_
#define _SP_RCONTEXT_H_

#include <stddef.h>
#include <sys/types.h>

#include "my_dbug.h"
#include "my_inttypes.h"
#include "prealloced_array.h"  // Prealloced_array
#include "sql/item.h"
#include "sql/query_result.h"  // Query_result_interceptor
#include "sql/sql_array.h"
#include "sql/sql_error.h"
#include "sql/table.h"

class Field;
class Query_arena;
class SELECT_LEX_UNIT;
class Server_side_cursor;
class THD;
class sp_cursor;
class sp_handler;
class sp_head;
class sp_instr;
class sp_instr_cpush;
class sp_pcontext;
class sp_variable;
template <class T>
class List;

///////////////////////////////////////////////////////////////////////////
// sp_rcontext declaration.
///////////////////////////////////////////////////////////////////////////

/*
  This class is a runtime context of a Stored Routine. It is used in an
  execution and is intended to contain all dynamic objects (i.e.  objects, which
  can be changed during execution), such as:
    - stored routine variables;
    - cursors;
    - handlers;

  Runtime context is used with sp_head class. sp_head class is intended to
  contain all static things, related to the stored routines (code, for example).
  sp_head instance creates runtime context for the execution of a stored
  routine.

  There is a parsing context (an instance of sp_pcontext class), which is used
  on parsing stage. However, now it contains some necessary for an execution
  things, such as definition of used stored routine variables. That's why
  runtime context needs a reference to the parsing context.
*/

class sp_rcontext {
 public:
  /// Construct and properly initialize a new sp_rcontext instance. The static
  /// create-function is needed because we need a way to return an error from
  /// the constructor.
  ///
  /// @param thd              Thread handle.
  /// @param root_parsing_ctx Top-level parsing context for this stored program.
  /// @param return_value_fld Field object to store the return value
  ///                         (for stored functions only).
  ///
  /// @return valid sp_rcontext object or NULL in case of OOM-error.
  static sp_rcontext *create(THD *thd, const sp_pcontext *root_parsing_ctx,
                             Field *return_value_fld);

  ~sp_rcontext();

 private:
  sp_rcontext(const sp_pcontext *root_parsing_ctx, Field *return_value_fld,
              bool in_sub_stmt);

  // Prevent use of copying constructor and operator.
  sp_rcontext(const sp_rcontext &);
  void operator=(sp_rcontext &);

 private:
  /// This is an auxillary class to store entering instruction pointer for an
  /// SQL-handler.
  class sp_handler_entry {
   public:
    /// Handler definition (from parsing context).
    const sp_handler *handler;

    /// Instruction pointer to the first instruction.
    uint first_ip;

    /// The constructor.
    ///
    /// @param _handler   sp_handler object.
    /// @param _first_ip  first instruction pointer.
    sp_handler_entry(const sp_handler *_handler, uint _first_ip)
        : handler(_handler), first_ip(_first_ip) {}
  };

 public:
  /// This class represents a call frame of SQL-handler (one invocation of a
  /// handler). Basically, it's needed to store continue instruction pointer for
  /// CONTINUE SQL-handlers.
  class Handler_call_frame {
   public:
    /// Handler definition (from parsing context).
    const sp_handler *handler;

    /// SQL-condition, triggered handler activation.
    Sql_condition *sql_condition;

    /// Continue-instruction-pointer for CONTINUE-handlers.
    /// The attribute contains 0 for EXIT-handlers.
    uint continue_ip;

    /// The Diagnostics Area which will be pushed when the handler activates
    /// and popped when the handler completes.
    Diagnostics_area handler_da;

    /// The constructor.
    ///
    /// @param _handler       SQL-handler
    /// @param _sql_condition SQL-condition, triggered handler activation.
    /// @param _continue_ip   Continue instruction pointer.
    Handler_call_frame(const sp_handler *_handler,
                       Sql_condition *_sql_condition, uint _continue_ip)
        : handler(_handler),
          sql_condition(_sql_condition),
          continue_ip(_continue_ip),
          handler_da(false) {}
  };

 public:
  /// Arena used to (re) allocate items on. E.g. reallocate INOUT/OUT
  /// SP-variables when they don't fit into prealloced items. This is common
  /// situation with String items. It is used mainly in sp_eval_func_item().
  Query_arena *callers_arena;

  /// Flag to end an open result set before start executing an SQL-handler
  /// (if one is found). Otherwise the client will hang due to a violation
  /// of the client/server protocol.
  bool end_partial_result_set;

  /// The stored program for which this runtime context is created.
  sp_head *sp;

  /////////////////////////////////////////////////////////////////////////
  // SP-variables.
  /////////////////////////////////////////////////////////////////////////

  bool set_variable(THD *thd, uint var_idx, Item **value) {
    return set_variable(thd, m_var_table->field[var_idx], value);
  }

  Item *get_item(uint var_idx) const { return m_var_items[var_idx]; }

  Item **get_item_addr(uint var_idx) const {
    return m_var_items.array() + var_idx;
  }

  bool set_return_value(THD *thd, Item **return_value_item);

  bool is_return_value_set() const { return m_return_value_set; }

  /////////////////////////////////////////////////////////////////////////
  // SQL-handlers.
  /////////////////////////////////////////////////////////////////////////

  /// Create a new sp_handler_entry instance and push it to the handler call
  /// stack.
  ///
  /// @param handler  SQL-handler object.
  /// @param first_ip First instruction pointer of the handler.
  ///
  /// @return error flag.
  /// @retval false on success.
  /// @retval true on error.
  bool push_handler(sp_handler *handler, uint first_ip);

  /// Pop and delete given number of sp_handler_entry instances from the handler
  /// call stack.
  ///
  /// @param current_scope  The current BEGIN..END block.
  void pop_handlers(sp_pcontext *current_scope);

  /// Get the Handler_call_frame representing the currently active handler.
  Handler_call_frame *current_handler_frame() const {
    return m_activated_handlers.size() ? m_activated_handlers.back() : NULL;
  }

  /// Handle current SQL condition (if any).
  ///
  /// This is the public-interface function to handle SQL conditions in
  /// stored routines.
  ///
  /// @param thd            Thread handle.
  /// @param [out] ip       Instruction pointer to the first handler
  ///                       instruction.
  /// @param cur_spi        Current SP instruction.
  ///
  /// @retval true if an SQL-handler has been activated. That means, all of
  /// the following conditions are satisfied:
  ///   - the SP-instruction raised SQL-condition(s),
  ///   - and there is an SQL-handler to process at least one of those
  ///     SQL-conditions,
  ///   - and that SQL-handler has been activated.
  /// Note, that the return value has nothing to do with "error flag"
  /// semantics.
  ///
  /// @retval false otherwise.
  bool handle_sql_condition(THD *thd, uint *ip, const sp_instr *cur_spi);

  /// Handle return from SQL-handler.
  ///
  /// @param thd            Thread handle.
  /// @param target_scope   The BEGIN..END block, containing
  ///                       the target (next) instruction.
  void exit_handler(THD *thd, sp_pcontext *target_scope);

  /// @return the continue instruction pointer of the last activated CONTINUE
  /// handler. This function must not be called for the EXIT handlers.
  uint get_last_handler_continue_ip() const {
    uint ip = m_activated_handlers.back()->continue_ip;
    DBUG_ASSERT(ip != 0);

    return ip;
  }

  /////////////////////////////////////////////////////////////////////////
  // Cursors.
  /////////////////////////////////////////////////////////////////////////

  /// Create a new sp_cursor instance and push it to the cursor stack.
  ///
  /// @param i          Cursor-push instruction.
  ///
  /// @return error flag.
  /// @retval false on success.
  /// @retval true on error.
  bool push_cursor(sp_instr_cpush *i);

  /// Pop and delete given number of sp_cursor instance from the cursor stack.
  ///
  /// @param count Number of cursors to pop & delete.
  void pop_cursors(uint count);

  void pop_all_cursors() { pop_cursors(m_ccount); }

  sp_cursor *get_cursor(uint i) const { return m_cstack[i]; }

  /////////////////////////////////////////////////////////////////////////
  // CASE expressions.
  /////////////////////////////////////////////////////////////////////////

  /// Set CASE expression to the specified value.
  ///
  /// @param thd             Thread handler.
  /// @param case_expr_id    The CASE expression identifier.
  /// @param case_expr_item_ptr  The CASE expression value
  ///
  /// @return error flag.
  /// @retval false on success.
  /// @retval true on error.
  ///
  /// @note The idea is to reuse Item_cache for the expression of the one
  /// CASE statement. This optimization takes place when there is CASE
  /// statement inside of a loop. So, in other words, we will use the same
  /// object on each iteration instead of creating a new one for each
  /// iteration.
  ///
  /// TODO
  ///   Hypothetically, a type of CASE expression can be different for each
  ///   iteration. For instance, this can happen if the expression contains
  ///   a session variable (something like @@VAR) and its type is changed
  ///   from one iteration to another.
  ///
  ///   In order to cope with this problem, we check type each time, when we
  ///   use already created object. If the type does not match, we re-create
  ///   Item.  This also can (should?) be optimized.
  bool set_case_expr(THD *thd, int case_expr_id, Item **case_expr_item_ptr);

  Item *get_case_expr(int case_expr_id) const {
    return m_case_expr_holders[case_expr_id];
  }

  Item **get_case_expr_addr(int case_expr_id) const {
    return (Item **)m_case_expr_holders.array() + case_expr_id;
  }

 private:
  /// Internal function to allocate memory for arrays.
  ///
  /// @param thd Thread handle.
  ///
  /// @return error flag: false on success, true in case of failure.
  bool alloc_arrays(THD *thd);

  /// Create and initialize a table to store SP-variables.
  ///
  /// param thd Thread handle.
  ///
  /// @return error flag.
  /// @retval false on success.
  /// @retval true on error.
  bool init_var_table(THD *thd);

  /// Create and initialize an Item-adapter (Item_field) for each SP-var field.
  ///
  /// param thd Thread handle.
  ///
  /// @return error flag.
  /// @retval false on success.
  /// @retval true on error.
  bool init_var_items(THD *thd);

  /// Create an instance of appropriate Item_cache class depending on the
  /// specified type in the callers arena.
  ///
  /// @note We should create cache items in the callers arena, as they are
  /// used between in several instructions.
  ///
  /// @param thd   Thread handler.
  /// @param item  Item to get the expression type.
  ///
  /// @return Pointer to valid object on success, or NULL in case of error.
  Item_cache *create_case_expr_holder(THD *thd, const Item *item) const;

  bool set_variable(THD *thd, Field *field, Item **value);

  /// Pop the Handler_call_frame on top of the stack of active handlers.
  /// Also pop the matching Diagnostics Area and transfer conditions.
  void pop_handler_frame(THD *thd);

 private:
  /// Top-level (root) parsing context for this runtime context.
  const sp_pcontext *m_root_parsing_ctx;

  /// Virtual table for storing SP-variables.
  TABLE *m_var_table;

  /// Collection of Item_field proxies, each of them points to the
  /// corresponding field in m_var_table.
  Bounds_checked_array<Item *> m_var_items;

  /// This is a pointer to a field, which should contain return value for
  /// stored functions (only). For stored procedures, this pointer is NULL.
  Field *m_return_value_fld;

  /// Indicates whether the return value (in m_return_value_fld) has been
  /// set during execution.
  bool m_return_value_set;

  /// Flag to tell if the runtime context is created for a sub-statement.
  bool m_in_sub_stmt;

  /// Stack of visible handlers.
  Prealloced_array<sp_handler_entry *, 16> m_visible_handlers;

  /// Stack of caught SQL conditions.
  Prealloced_array<Handler_call_frame *, 16> m_activated_handlers;

  /// Stack of cursors.
  Bounds_checked_array<sp_cursor *> m_cstack;

  /// Current number of cursors in m_cstack.
  uint m_ccount;

  /// Array of CASE expression holders.
  Bounds_checked_array<Item_cache *> m_case_expr_holders;
};

///////////////////////////////////////////////////////////////////////////
// sp_cursor declaration.
///////////////////////////////////////////////////////////////////////////

/* A mediator between stored procedures and server side cursors */

class sp_cursor {
 private:
  /**
    An interceptor of cursor result set used to implement
    FETCH @<cname@> INTO @<varlist@>.
  */
  class Query_fetch_into_spvars : public Query_result_interceptor {
    List<sp_variable> *spvar_list;
    uint field_count;

   public:
    Query_fetch_into_spvars() : Query_result_interceptor() {}
    uint get_field_count() { return field_count; }
    void set_spvar_list(List<sp_variable> *vars) { spvar_list = vars; }

    virtual bool send_eof(THD *) { return false; }
    virtual bool send_data(THD *thd, List<Item> &items);
    virtual bool prepare(THD *thd, List<Item> &list, SELECT_LEX_UNIT *u);
  };

 public:
  explicit sp_cursor(sp_instr_cpush *i)
      : m_result(), m_server_side_cursor(nullptr), m_push_instr(i) {}

  virtual ~sp_cursor() { destroy(); }

  bool open(THD *thd);

  bool close();

  bool is_open() const { return m_server_side_cursor != nullptr; }

  bool fetch(List<sp_variable> *vars);

  sp_instr_cpush *get_push_instr() { return m_push_instr; }

 private:
  Query_fetch_into_spvars m_result;

  Server_side_cursor *m_server_side_cursor;
  sp_instr_cpush *m_push_instr;

 private:
  void destroy();
};  // class sp_cursor

#endif /* _SP_RCONTEXT_H_ */
