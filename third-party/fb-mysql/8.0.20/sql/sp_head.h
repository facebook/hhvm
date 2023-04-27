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

#ifndef _SP_HEAD_H_
#define _SP_HEAD_H_

#include <stddef.h>
#include <sys/types.h>
#include <string>
#include <vector>

#include "lex_string.h"
#include "map_helpers.h"
#include "my_alloc.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_psi_config.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "mysqld_error.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/create_field.h"
#include "sql/mem_root_array.h"  // Mem_root_array
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/system_variables.h"
#include "sql/table.h"

class Field;
class Item;
class Item_trigger_field;
class Sroutine_hash_entry;
class Table_trigger_field_support;
class THD;
class sp_head;
struct CHARSET_INFO;
struct MY_BITMAP;

/**
  @defgroup Stored_Routines Stored Routines
  @ingroup Runtime_Environment
  @{
*/

class sp_branch_instr;
class sp_instr;
class sp_label;
class sp_lex_branch_instr;
class sp_pcontext;

/**
  Number of PSI_statement_info instruments
  for internal stored programs statements.
*/
#define SP_PSI_STATEMENT_INFO_COUNT 16

#ifdef HAVE_PSI_INTERFACE
void init_sp_psi_keys(void);
#endif

///////////////////////////////////////////////////////////////////////////

/**
  Stored_program_creation_ctx -- base class for creation context of stored
  programs (stored routines, triggers, events).
*/

class Stored_program_creation_ctx : public Default_object_creation_ctx {
 public:
  const CHARSET_INFO *get_db_cl() { return m_db_cl; }

 public:
  virtual Stored_program_creation_ctx *clone(MEM_ROOT *mem_root) = 0;

 protected:
  explicit Stored_program_creation_ctx(THD *thd);

  Stored_program_creation_ctx(const CHARSET_INFO *client_cs,
                              const CHARSET_INFO *connection_cl,
                              const CHARSET_INFO *db_cl)
      : Default_object_creation_ctx(client_cs, connection_cl), m_db_cl(db_cl) {}

 protected:
  void change_env(THD *thd) const override;

 protected:
  /**
    db_cl stores the value of the database collation. Both character set
    and collation attributes are used.

    Database collation is included into the context because it defines the
    default collation for stored-program variables.
  */
  const CHARSET_INFO *m_db_cl;
};

///////////////////////////////////////////////////////////////////////////

class sp_name {
 public:
  LEX_CSTRING m_db;
  LEX_STRING m_name;
  LEX_STRING m_qname;
  bool m_explicit_name; /**< Prepend the db name? */

  sp_name(const LEX_CSTRING &db, const LEX_STRING &name, bool use_explicit_name)
      : m_db(db), m_name(name), m_explicit_name(use_explicit_name) {
    m_qname.str = nullptr;
    m_qname.length = 0;
  }

  /** Create temporary sp_name object for Sroutine_hash_entry. */
  sp_name(const Sroutine_hash_entry *rt, char *qname_buff);

  // Init. the qualified name from the db and name.
  void init_qname(THD *thd);  // thd for memroot allocation
};

///////////////////////////////////////////////////////////////////////////

/**
  sp_parser_data provides a scope for attributes used at the SP-parsing
  stage only.
*/
class sp_parser_data {
 private:
  struct Backpatch_info {
    sp_label *label;
    sp_branch_instr *instr;
  };

 public:
  sp_parser_data()
      : m_current_stmt_start_ptr(nullptr),
        m_option_start_ptr(nullptr),
        m_param_start_ptr(nullptr),
        m_param_end_ptr(nullptr),
        m_body_start_ptr(nullptr),
        m_cont_level(0),
        m_saved_memroot(nullptr),
        m_saved_item_list(nullptr) {}

  ///////////////////////////////////////////////////////////////////////

  /**
    Start parsing a stored program body statement.

    This method switches THD::mem_root and THD::m_item_list in order to parse
    SP-body. The current values are kept to be restored after the body
    statement is parsed.

    @param thd  Thread context.
    @param sp   Stored Program being parsed.
  */
  void start_parsing_sp_body(THD *thd, sp_head *sp);

  /**
    Finish parsing of a stored program body statement.

    This method switches THD::mem_root and THD::m_item_list back when SP-body
    parsing is completed.

    @param thd  Thread context.
  */
  void finish_parsing_sp_body(THD *thd);

  /**
    @retval true if SP-body statement is being parsed.
    @retval false otherwise.
  */
  bool is_parsing_sp_body() const { return m_saved_memroot != nullptr; }

  ///////////////////////////////////////////////////////////////////////

  void process_new_sp_instr(THD *thd, sp_instr *i);

  ///////////////////////////////////////////////////////////////////////

  const char *get_current_stmt_start_ptr() const {
    return m_current_stmt_start_ptr;
  }

  void set_current_stmt_start_ptr(const char *stmt_start_ptr) {
    m_current_stmt_start_ptr = stmt_start_ptr;
  }

  ///////////////////////////////////////////////////////////////////////

  const char *get_option_start_ptr() const { return m_option_start_ptr; }

  void set_option_start_ptr(const char *option_start_ptr) {
    m_option_start_ptr = option_start_ptr;
  }

  ///////////////////////////////////////////////////////////////////////

  const char *get_parameter_start_ptr() const { return m_param_start_ptr; }

  void set_parameter_start_ptr(const char *ptr) { m_param_start_ptr = ptr; }

  const char *get_parameter_end_ptr() const { return m_param_end_ptr; }

  void set_parameter_end_ptr(const char *ptr) { m_param_end_ptr = ptr; }

  ///////////////////////////////////////////////////////////////////////

  const char *get_body_start_ptr() const { return m_body_start_ptr; }

  void set_body_start_ptr(const char *ptr) { m_body_start_ptr = ptr; }

  ///////////////////////////////////////////////////////////////////////

  void push_lex(LEX *lex) { m_lex_stack.push_front(lex); }

  LEX *pop_lex() { return m_lex_stack.pop(); }

  ///////////////////////////////////////////////////////////////////////
  // Backpatch-list operations.
  ///////////////////////////////////////////////////////////////////////

  /**
    Put the instruction on the backpatch list, associated with the label.

    @param i      The SP-instruction.
    @param label  The label.

    @return Error flag.
  */
  bool add_backpatch_entry(sp_branch_instr *i, sp_label *label);

  /**
    Update all instruction with the given label in the backpatch list
    to the given instruction pointer.

    @param label  The label.
    @param dest   The instruction pointer.
  */
  void do_backpatch(sp_label *label, uint dest);

  ///////////////////////////////////////////////////////////////////////
  // Backpatch operations for supporting CONTINUE handlers.
  ///////////////////////////////////////////////////////////////////////

  /**
    Start a new backpatch level for the SP-instruction requiring continue
    destination. If the SP-instruction is NULL, the level is just increased.

    @note Only subclasses of sp_lex_branch_instr need backpatching of
    continue destinations (and no other classes do):
      - sp_instr_jump_if_not
      - sp_instr_set_case_expr
      - sp_instr_jump_case_when

    That's why the methods below accept sp_lex_branch_instr to make this
    relationship clear. And these two functions are the only places where
    set_cont_dest() is used, so set_cont_dest() is also a member of
    sp_lex_branch_instr.

    @todo These functions should probably be declared in a separate
    interface class, but currently we try to minimize the sp_instr
    hierarchy.

    @return false always.
  */
  bool new_cont_backpatch() {
    ++m_cont_level;
    return false;
  }

  /**
    Add a SP-instruction to the current level.

    @param i    The SP-instruction.

    @return Error flag.
  */
  bool add_cont_backpatch_entry(sp_lex_branch_instr *i);

  /**
    Backpatch (and pop) the current level to the given instruction pointer.

    @param dest The instruction pointer.
  */
  void do_cont_backpatch(uint dest);

 private:
  /// Start of the current statement's query string.
  const char *m_current_stmt_start_ptr;

  /// Start of the SET-expression query string.
  const char *m_option_start_ptr;

  /**
    Stack of LEX-objects. It's needed to handle processing of
    sub-statements.
  */
  List<LEX> m_lex_stack;

  /**
    Position in the CREATE PROCEDURE- or CREATE FUNCTION-statement's query
    string corresponding to the start of parameter declarations (stored
    procedure or stored function parameters).
  */
  const char *m_param_start_ptr;

  /**
    Position in the CREATE PROCEDURE- or CREATE FUNCTION-statement's query
    string corresponding to the end of parameter declarations (stored
    procedure or stored function parameters).
  */
  const char *m_param_end_ptr;

  /**
    Position in the CREATE-/ALTER-stored-program statement's query string
    corresponding to the start of the first SQL-statement.
  */
  const char *m_body_start_ptr;

  /// Instructions needing backpatching
  List<Backpatch_info> m_backpatch;

  /**
    We need a special list for backpatching of instructions with a continue
    destination (in the case of a continue handler catching an error in
    the test), since it would otherwise interfere with the normal backpatch
    mechanism - e.g. jump_if_not instructions have two different destinations
    which are to be patched differently.
    Since these occur in a more restricted way (always the same "level" in
    the code), we don't need the label.
  */
  List<sp_lex_branch_instr> m_cont_backpatch;

  /// The current continue backpatch level
  uint m_cont_level;

  /**********************************************************************
    The following attributes are used to store THD values during parsing
    of stored program body.

    @sa start_parsing_sp_body()
    @sa finish_parsing_sp_body()
  **********************************************************************/

  /// THD's memroot.
  MEM_ROOT *m_saved_memroot;

  /// THD's item list.
  Item *m_saved_item_list;
};

///////////////////////////////////////////////////////////////////////////

struct SP_TABLE;

/**
  sp_head represents one instance of a stored program. It might be of any type
  (stored procedure, function, trigger, event).
*/
class sp_head {
 public:
  /** Possible values of m_flags */
  enum {
    HAS_RETURN = 1,             // For FUNCTIONs only: is set if has RETURN
    MULTI_RESULTS = 8,          // Is set if a procedure with SELECT(s)
    CONTAINS_DYNAMIC_SQL = 16,  // Is set if a procedure with PREPARE/EXECUTE
    IS_INVOKED = 32,            // Is set if this sp_head is being used
    HAS_SET_AUTOCOMMIT_STMT =
        64,  // Is set if a procedure with 'set autocommit'
    /* Is set if a procedure with COMMIT (implicit or explicit) | ROLLBACK */
    HAS_COMMIT_OR_ROLLBACK = 128,
    LOG_SLOW_STATEMENTS = 256,  // Used by events
    LOG_GENERAL_LOG = 512,      // Used by events
    HAS_SQLCOM_RESET = 1024,
    HAS_SQLCOM_FLUSH = 2048,

    /**
      Marks routines that directly (i.e. not by calling other routines)
      change tables. Note that this flag is set automatically based on
      type of statements used in the stored routine and is different
      from routine characteristic provided by user in a form of CONTAINS
      SQL, READS SQL DATA, MODIFIES SQL DATA clauses. The latter are
      accepted by parser but pretty much ignored after that.
      We don't rely on them:
      a) for compatibility reasons.
      b) because in CONTAINS SQL case they don't provide enough
      information anyway.
     */
    MODIFIES_DATA = 4096
  };

 public:
  /************************************************************************
    Public attributes.
  ************************************************************************/

  /// Stored program type.
  enum_sp_type m_type;

  /// Stored program flags.
  uint m_flags;

  /**
    Instrumentation interface for SP.
  */
  PSI_sp_share *m_sp_share;

  /**
    Definition of the RETURN-field (from the RETURNS-clause).
    It's used (and valid) for stored functions only.
  */
  Create_field m_return_field_def;

  /// Attributes used during the parsing stage only.
  sp_parser_data m_parser_data;

  /// Stored program characteristics.
  st_sp_chistics *m_chistics;

  /**
    The value of sql_mode system variable at the CREATE-time.

    It should be stored along with the character sets in the
    Stored_program_creation_ctx.
  */
  sql_mode_t m_sql_mode;

  /// Fully qualified name (@<db name@>.@<sp name@>).
  LEX_STRING m_qname;

  bool m_explicit_name;  ///< Prepend the db name? */

  LEX_STRING m_db;
  LEX_STRING m_name;
  LEX_STRING m_params;
  LEX_CSTRING m_body;
  LEX_CSTRING m_body_utf8;
  LEX_STRING m_defstr;
  LEX_STRING m_definer_user;
  LEX_STRING m_definer_host;

  longlong m_created;
  longlong m_modified;

  /// Recursion level of the current SP instance. The levels are numbered from
  /// 0.
  ulong m_recursion_level;

  /**
    A list of diferent recursion level instances for the same procedure.
    For every recursion level we have a sp_head instance. This instances
    connected in the list. The list ordered by increasing recursion level
    (m_recursion_level).
  */
  sp_head *m_next_cached_sp;

  /// Pointer to the first element of the above list
  sp_head *m_first_instance;

  /**
    Pointer to the first free (non-INVOKED) routine in the list of
    cached instances for this SP. This pointer is set only for the first
    SP in the list of instances (see above m_first_cached_sp pointer).
    The pointer equal to 0 if we have no free instances.
    For non-first instance value of this pointer meaningless (point to itself);
  */
  sp_head *m_first_free_instance;

  /**
    Pointer to the last element in the list of instances of the SP.
    For non-first instance value of this pointer meaningless (point to itself);
  */
  sp_head *m_last_cached_sp;

  /**
    Set containing names of stored routines used by this routine.
    Note that unlike elements of similar set for statement elements of this
    set are not linked in one list. Because of this we are able save memory
    by using for this set same objects that are used in 'sroutines' sets
    for statements of which this stored routine consists.

    See Sroutine_hash_entry for explanation why this hash uses binary
    key comparison.
  */
  malloc_unordered_map<std::string, Sroutine_hash_entry *> m_sroutines;

  /*
    Security context for stored routine which should be run under
    definer privileges.
  */
  Security_context m_security_ctx;

  /////////////////////////////////////////////////////////////////////////
  // Trigger-specific public attributes.
  /////////////////////////////////////////////////////////////////////////

  /**
    List of item (Item_trigger_field objects)'s lists representing fields
    in old/new version of row in trigger. We use this list for checking
    whether all such fields are valid or not at trigger creation time and for
    binding these fields to TABLE object at table open (although for latter
    pointer to table being opened is probably enough).
  */
  SQL_I_List<SQL_I_List<Item_trigger_field>> m_list_of_trig_fields_item_lists;
  /**
    List of all the Item_trigger_field items created while parsing
    sp instruction. After parsing, in add_instr method this list
    is moved to per instruction Item_trigger_field list
    "sp_lex_instr::m_trig_field_list".
  */
  SQL_I_List<Item_trigger_field> m_cur_instr_trig_field_items;

  /// Trigger characteristics.
  st_trg_chistics m_trg_chistics;

  /// The Table_trigger_dispatcher instance, where this trigger belongs to.
  class Table_trigger_dispatcher *m_trg_list;

 public:
  static void destroy(sp_head *sp);

  /// Is this routine being executed?
  bool is_invoked() const { return m_flags & IS_INVOKED; }

  /**
    Get the value of the SP cache version, as remembered
    when the routine was inserted into the cache.
  */
  int64 sp_cache_version() const { return m_sp_cache_version; }

  /// Set the value of the SP cache version.
  void set_sp_cache_version(int64 sp_cache_version) {
    m_sp_cache_version = sp_cache_version;
  }

  Stored_program_creation_ctx *get_creation_ctx() { return m_creation_ctx; }

  void set_creation_ctx(Stored_program_creation_ctx *creation_ctx) {
    m_creation_ctx = creation_ctx->clone(&main_mem_root);
  }

  /// Set the body-definition start position.
  void set_body_start(THD *thd, const char *begin_ptr);

  /// Set the statement-definition (body-definition) end position.
  void set_body_end(THD *thd);

  bool setup_trigger_fields(THD *thd, Table_trigger_field_support *tfs,
                            GRANT_INFO *subject_table_grant,
                            bool need_fix_fields);

  void mark_used_trigger_fields(TABLE *subject_table);

  bool has_updated_trigger_fields(const MY_BITMAP *used_fields) const;

  /**
    Execute trigger stored program.

    - changes security context for triggers
    - switch to new memroot
    - call sp_head::execute
    - restore old memroot
    - restores security context

    @param thd               Thread context
    @param db_name           database name
    @param table_name        table name
    @param grant_info        GRANT_INFO structure to be filled with
                             information about definer's privileges
                             on subject table

    @todo
      We should create sp_rcontext once per command and reuse it
      on subsequent executions of a trigger.

    @return Error status.
  */
  bool execute_trigger(THD *thd, const LEX_CSTRING &db_name,
                       const LEX_CSTRING &table_name, GRANT_INFO *grant_info);

  /**
    Execute a function.

     - evaluate parameters
     - changes security context for SUID routines
     - switch to new memroot
     - call sp_head::execute
     - restore old memroot
     - evaluate the return value
     - restores security context

    @param thd               Thread context.
    @param args              Passed arguments (these are items from containing
                             statement?)
    @param argcount          Number of passed arguments. We need to check if
                             this is correct.
    @param return_fld        Save result here.

    @todo
      We should create sp_rcontext once per command and reuse
      it on subsequent executions of a function/trigger.

    @todo
      In future we should associate call arena/mem_root with
      sp_rcontext and allocate all these objects (and sp_rcontext
      itself) on it directly rather than juggle with arenas.

    @return Error status.
  */
  bool execute_function(THD *thd, Item **args, uint argcount,
                        Field *return_fld);

  /**
    Execute a procedure.

    The function does the following steps:
     - Set all parameters
     - changes security context for SUID routines
     - call sp_head::execute
     - copy back values of INOUT and OUT parameters
     - restores security context

    @param thd  Thread context.
    @param args List of values passed as arguments.

    @return Error status.
  */

  bool execute_procedure(THD *thd, List<Item> *args);

  /**
    Add instruction to SP.

    @param thd    Thread context.
    @param instr  Instruction.

    @return Error status.
  */
  bool add_instr(THD *thd, sp_instr *instr);

  /**
    Returns true if any substatement in the routine directly
    (not through another routine) modifies data/changes table.

    @sa Comment for MODIFIES_DATA flag.
  */
  bool modifies_data() const { return m_flags & MODIFIES_DATA; }

  uint instructions() { return static_cast<uint>(m_instructions.size()); }

  sp_instr *last_instruction() { return m_instructions.back(); }

  /**
    Reset LEX-object during parsing, before we parse a sub statement.

    @param thd  Thread context.

    @return Error status.
  */
  bool reset_lex(THD *thd);

  /**
    Restore LEX-object during parsing, after we have parsed a sub statement.

    @param thd  Thread context.

    @return Error status.
  */
  bool restore_lex(THD *thd);

  char *name(uint *lenp = nullptr) const {
    if (lenp) *lenp = (uint)m_name.length;
    return m_name.str;
  }

  /**
    Create Field-object corresponding to the RETURN field of a stored function.
    This operation makes sense for stored functions only.

    @param thd              thread context.
    @param field_max_length the max length (in the sense of Item classes).
    @param field_name       the field name (item name).
    @param table            the field's table.

    @return newly created and initialized Field-instance,
    or NULL in case of error.
  */
  Field *create_result_field(THD *thd, size_t field_max_length,
                             const char *field_name, TABLE *table) const;

  void returns_type(THD *thd, String *result) const;

  void set_info(longlong created, longlong modified, st_sp_chistics *chistics,
                sql_mode_t sql_mode);

  void set_definer(const char *definer, size_t definerlen);
  void set_definer(const LEX_CSTRING &user_name, const LEX_CSTRING &host_name);

  /**
    Do some minimal optimization of the code:
      -# Mark used instructions
      -# While doing this, shortcut jumps to jump instructions
      -# Compact the code, removing unused instructions.

    This is the main mark and move loop; it relies on the following methods
    in sp_instr and its subclasses:

      - opt_mark()         :  Mark instruction as reachable
      - opt_shortcut_jump():  Shortcut jumps to the final destination;
                             used by opt_mark().
      - opt_move()         :  Update moved instruction
      - set_destination()  :  Set the new destination (jump instructions only)
  */
  void optimize();

  /**
    Helper used during flow analysis during code optimization.
    See the implementation of <code>opt_mark()</code>.
    @param ip the instruction to add to the leads list
    @param leads the list of remaining paths to explore in the graph that
    represents the code, during flow analysis.
  */
  void add_mark_lead(uint ip, List<sp_instr> *leads);

  /**
    Get SP-instruction at given index.

    NOTE: it is important to have *unsigned* int here, sometimes we get (-1)
    passed here, so it get's converted to MAX_INT, and the result of the
    function call is NULL.
  */
  sp_instr *get_instr(uint i) {
    return (i < (uint)m_instructions.size()) ? m_instructions.at(i) : NULL;
  }

  /**
    Add tables used by routine to the table list.

      Converts multi-set of tables used by this routine to table list and adds
      this list to the end of table list specified by 'query_tables_last_ptr'.

      Elements of list will be allocated in PS memroot, so this list will be
      persistent between PS executions.

    @param[in] thd                        Thread context
    @param[in,out] query_tables_last_ptr  Pointer to the next_global member of
                                          last element of the list where tables
                                          will be added (or to its root).
    @param[in] sql_command                SQL-command for which we are adding
                                          elements to the table list.
    @param[in] belong_to_view             Uppermost view which uses this
    routine, NULL if none.
  */
  void add_used_tables_to_table_list(THD *thd,
                                     TABLE_LIST ***query_tables_last_ptr,
                                     enum_sql_command sql_command,
                                     TABLE_LIST *belong_to_view);

  /**
    Check if this stored routine contains statements disallowed
    in a stored function or trigger, and set an appropriate error message
    if this is the case.
  */
  bool is_not_allowed_in_function(const char *where) {
    if (m_flags & CONTAINS_DYNAMIC_SQL)
      my_error(ER_STMT_NOT_ALLOWED_IN_SF_OR_TRG, MYF(0), "Dynamic SQL");
    else if (m_flags & MULTI_RESULTS)
      my_error(ER_SP_NO_RETSET, MYF(0), where);
    else if (m_flags & HAS_SET_AUTOCOMMIT_STMT)
      my_error(ER_SP_CANT_SET_AUTOCOMMIT, MYF(0));
    else if (m_flags & HAS_COMMIT_OR_ROLLBACK)
      my_error(ER_COMMIT_NOT_ALLOWED_IN_SF_OR_TRG, MYF(0));
    else if (m_flags & HAS_SQLCOM_RESET)
      my_error(ER_STMT_NOT_ALLOWED_IN_SF_OR_TRG, MYF(0), "RESET");
    else if (m_flags & HAS_SQLCOM_FLUSH)
      my_error(ER_STMT_NOT_ALLOWED_IN_SF_OR_TRG, MYF(0), "FLUSH");

    return (m_flags &
            (CONTAINS_DYNAMIC_SQL | MULTI_RESULTS | HAS_SET_AUTOCOMMIT_STMT |
             HAS_COMMIT_OR_ROLLBACK | HAS_SQLCOM_RESET | HAS_SQLCOM_FLUSH));
  }

#ifndef DBUG_OFF
  /**
    Return the routine instructions as a result set.
    @return Error status.
  */
  bool show_routine_code(THD *thd);
#endif

  /*
    This method is intended for attributes of a routine which need
    to propagate upwards to the Query_tables_list of the caller (when
    a property of a sp_head needs to "taint" the calling statement).
  */
  void propagate_attributes(Query_tables_list *prelocking_ctx) {
    /*
      If this routine needs row-based binary logging, the entire top statement
      too (we cannot switch from statement-based to row-based only for this
      routine, as in statement-based the top-statement may be binlogged and
      the sub-statements not).
    */
    DBUG_PRINT("info", ("lex->get_stmt_unsafe_flags(): 0x%x",
                        prelocking_ctx->get_stmt_unsafe_flags()));
    DBUG_PRINT("info", ("sp_head(0x%p=%s)->unsafe_flags: 0x%x", this, name(),
                        unsafe_flags));
    prelocking_ctx->set_stmt_unsafe_flags(unsafe_flags);
  }

  /**
    @return root parsing context for this stored program.
  */
  sp_pcontext *get_root_parsing_context() const {
    return const_cast<sp_pcontext *>(m_root_parsing_ctx);
  }

  /**
    @return SP-persistent mem-root. Instructions and expressions are stored in
    its memory between executions.
  */
  MEM_ROOT *get_persistent_mem_root() const {
    return const_cast<MEM_ROOT *>(&main_mem_root);
  }

  /**
    Check if a user has access right to a SP.

    @param      thd          Thread context.
    @param[out] full_access  Set to 1 if the user is the owner
                             of the stored program.

    @return Error status.
  */
  bool check_show_access(THD *thd, bool *full_access);

  /**
    Change routine security context, and check if there is an EXECUTE privilege
    in new context. If there is no EXECUTE privilege, change the context back
    and return an error.

    @param      thd      Thread context.
    @param[out] save_ctx Where to save the old security context.

    @todo Cache if the definer has the rights to use the object on the first
    usage and reset the cache only if someone does a GRANT statement that 'may'
    affect this.

    @return Error status.
  */
  bool set_security_ctx(THD *thd, Security_context **save_ctx);

 private:
  /// Use sp_start_parsing() to create instances of sp_head.
  sp_head(MEM_ROOT &&mem_root, enum_sp_type type);

  /// Use destroy() to destoy instances of sp_head.
  ~sp_head();

  /// SP-persistent memory root (for instructions and expressions).
  MEM_ROOT main_mem_root;

  /// Root parsing context (topmost BEGIN..END block) of this SP.
  sp_pcontext *m_root_parsing_ctx;

  /// The SP-instructions.
  Mem_root_array<sp_instr *> m_instructions;

  /**
    Multi-set representing optimized list of tables to be locked by this
    routine. Does not include tables which are used by invoked routines.

    @note
    For prelocking-free SPs this multiset is constructed too.
    We do so because the same instance of sp_head may be called both
    in prelocked mode and in non-prelocked mode.
  */
  collation_unordered_map<std::string, SP_TABLE *> m_sptabs;

  /*
    The same information as in m_sptabs, but sorted (by an arbitrary key).
    This is useful to get consistent locking order, which makes MTR tests
    more deterministic across platforms. It does not have a bearing on the
    actual behavior of the server.
  */
  std::vector<SP_TABLE *> m_sptabs_sorted;

  /**
    Version of the stored routine cache at the moment when the
    routine was added to it. Is used only for functions and
    procedures, not used for triggers or events.  When sp_head is
    created, its version is 0. When it's added to the cache, the
    version is assigned the global value 'Cversion'.
    If later on Cversion is incremented, we know that the routine
    is obsolete and should not be used --
    sp_cache_flush_obsolete() will purge it.
  */
  int64 m_sp_cache_version;

  /// Snapshot of several system variables at CREATE-time.
  Stored_program_creation_ctx *m_creation_ctx;

  /// Flags of LEX::enum_binlog_stmt_unsafe.
  uint32 unsafe_flags;

 private:
  /// Copy sp name from parser.
  void init_sp_name(THD *thd, sp_name *spname);

  /**
    Execute the routine. The main instruction jump loop is there.
    Assume the parameters already set.

    @param thd                  Thread context.
    @param merge_da_on_success  Flag specifying if Warning Info should be
                                propagated to the caller on Completion
                                Condition or not.

    @todo
      - Will write this SP statement into binlog separately
      (TODO: consider changing the condition to "not inside event union")

    @return Error status.
  */
  bool execute(THD *thd, bool merge_da_on_success);

  /**
    Perform a forward flow analysis in the generated code.
    Mark reachable instructions, for the optimizer.
  */
  void opt_mark();

  /**
    Merge the list of tables used by some query into the multi-set of
    tables used by routine.

    @param thd                 Thread context.
    @param table               Table list.
    @param lex_for_tmp_check   LEX of the query for which we are merging
                               table list.

    @note
      This method will use LEX provided to check whenever we are creating
      temporary table and mark it as such in target multi-set.

    @return Error status.
  */
  bool merge_table_list(THD *thd, TABLE_LIST *table, LEX *lex_for_tmp_check);

  friend sp_head *sp_start_parsing(THD *thd, enum_sp_type sp_type,
                                   sp_name *sp_name);

  // Prevent use of copy constructor and assignment operator.
  sp_head(const sp_head &);
  void operator=(sp_head &);
};

///////////////////////////////////////////////////////////////////////////

/**
  @} (end of group Stored_Routines)
*/

#endif /* _SP_HEAD_H_ */
