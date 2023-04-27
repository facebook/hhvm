/* Copyright (c) 2011, 2020, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA  */

#ifndef OPT_TRACE_INCLUDED
#define OPT_TRACE_INCLUDED

#include "my_config.h"

#include <limits.h>
#include <string.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "my_sqlcommand.h"          // enum_sql_command
#include "sql/opt_trace_context.h"  // Opt_trace_context

class Cost_estimate;
class Item;
class THD;
class set_var_base;
class sp_head;
class sp_printable;
struct TABLE_LIST;
template <class T>
class List;

/**
  @file sql/opt_trace.h
  API for the Optimizer trace (WL#5257)
*/

/**
  @page PAGE_OPT_TRACE The Optimizer Trace

  @section INTRODUCTION Introduction

  This optimizer trace is aimed at producing output, which is readable by
  humans and by programs, to aid understanding of decisions and actions taken
  by the MySQL Optimizer.

  @section OUTPUT_FORMAT Output format

  The chosen output format is JSON (JavaScript Object Notation).
  In JSON there are:
  @li "objects" (unordered set of key-value pairs); equivalent to Python's
  dictionary or Perl's associative array or hash or STL's hash_map.
  @li "arrays" (ordered set of values); equivalent to Python's and Perl's list
  or STL's vector.
  @li "values": a value can be a string, number, boolean, null,
  which we all call "scalars", or be an object, array.

  For example (explanations after "<<" are not part of output):
@verbatim
  {                           << start of top object
    "first_name": "Gustave",  << key/value pair (value is string)
    "last_name": "Eiffel",    << key/value pair (value is string)
    "born": 1832,             << key/value pair (value is integer)
    "contributions_to": [     << key/value pair (value is array)
       {                      << 1st item of array is an object (a building)
         "name": "Eiffel tower",
         "location": Paris
       },                     << end of 1st item of array
       {
         "name": "Liberty statue",
         "location": "New York"
       }                      << end of 2nd item of array
    ]                         << end of array
  }                           << end of top object
@endverbatim
  For more details, have a look at the syntax at json.org.
  Note that indentation and newlines are superfluous, useful only for
  human-readability.
  Note also that there is nothing like a "named object": an object, array or
  value has no name; but if it is the value of a key/value pair in an
  enclosing, outer object, then the key can be seen as the inner object's
  "name".

  @section USER_ENABLE_TRACING How a user enables/views the trace

@verbatim
  SET SESSION OPTIMIZER_TRACE="enabled=on"; # enable tracing
  <statement to trace>; # like SELECT, EXPLAIN SELECT, UPDATE, DELETE...
  SELECT * FROM information_schema.OPTIMIZER_TRACE;
  [ repeat last two steps at will ]
  SET SESSION OPTIMIZER_TRACE="enabled=off"; # disable tracing
@endverbatim

  @c SELECT and @c EXPLAIN SELECT produce the same trace. But there are
  exceptions regarding subqueries because the two commands treat subqueries
  differently, for example in
@verbatim
  SELECT ... WHERE x IN (subq1) AND y IN (subq2)
@endverbatim
  SELECT terminates after executing the first subquery if the related IN
  predicate is false, so we won't see @c JOIN::optimize() tracing for subq2;
  whereas EXPLAIN SELECT analyzes all subqueries (see loop at the end of
  @c select_describe()).

  @section USER_SELECT_TRACING_STATEMENTS How a user traces only certain
statements

  When tracing is in force, each SQL statement generates a trace; more
  exactly, so does any of
  SELECT,
  EXPLAIN SELECT,
  INSERT or REPLACE ( with VALUES or SELECT),
  UPDATE/DELETE and their multi-table variants,
  SET (unless it manipulates @@@@optimizer_trace),
  DO,
  DECLARE/CASE/IF/RETURN (stored routines language elements),
  CALL.
  If a command above is prepared and executed in separate steps, preparation
  and execution are separately traced.
  By default each new trace overwrites the previous trace. Thus, if a
  statement contains sub-statements (example: invokes stored procedures,
  stored functions, triggers), the top statement and sub-statements each
  generate traces, but at the execution's end only the last sub-statement's
  trace is visible.
  If the user wants to see the trace of another sub-statement, she/he can
  enable/disable tracing around the desired sub-statement, but this requires
  editing the routine's code, which may not be possible. Another solution is
  to use
@verbatim
  SET optimizer_trace_offset=<OFFSET>, optimizer_trace_limit=<LIMIT>
@endverbatim
  where OFFSET is a signed integer, and LIMIT is a positive integer.
  The effect of this SET is the following:

  @li all remembered traces are cleared

  @li a later SELECT on OPTIMIZER_TRACE returns the first LIMIT traces of
  the OFFSET oldest remembered traces (if OFFSET >= 0), or the first LIMIT
  traces of the -OFFSET newest remembered traces (if OFFSET < 0).

  For example,
  a combination of OFFSET=-1 and LIMIT=1 will make the last trace be shown (as
  is default), OFFSET=-2 and LIMIT=1 will make the next-to-last be shown,
  OFFSET=-5 and LIMIT=5 will make the last five traces be shown. Such negative
  OFFSET can be useful when one knows that the interesting sub-statements are
  the few last ones of a stored routine, like this:
@verbatim
  SET optimizer_trace_offset=-5, optimizer_trace_limit=5;
  CALL stored_routine(); # more than 5 sub-statements in this routine
  SELECT * FROM information_schema.OPTIMIZER_TRACE; # see only last 5 traces
@endverbatim
  On the opposite, a positive OFFSET can be useful when one knows that the
  interesting sub-statements are the few first ones of a stored routine.

  The more those two variables are accurately adjusted, the less memory is
  used. For example, OFFSET=0 and LIMIT=5 will use memory to remember 5
  traces, so if only the three first are needed, OFFSET=0 and LIMIT=3 is
  better (tracing stops after the LIMITth trace, so the 4th and 5th trace are
  not created and don't take up memory). A stored routine may have a loop
  which executes many sub-statements and thus generates many traces, which
  would use a lot of memory; proper OFFSET and LIMIT can restrict tracing to
  one iteration of the loop for example. This also gains speed, as tracing a
  sub-statement impacts performance.

  If OFFSET>=0, only LIMIT traces are kept in memory. If OFFSET<0, that is not
  true: instead, (-OFFSET) traces are kept in memory; indeed even if LIMIT is
  smaller than (-OFFSET), so excludes the last statement, the last statement
  must still be traced because it will be inside LIMIT after executing one
  more statement (remember than OFFSET<0 is counted from the end: the "window"
  slides as more statements execute).

  Such memory and speed gains are the reason why optimizer_trace_offset/limit,
  which are restrictions at the trace producer level, are offered. They are
  better than using
@verbatim
  SELECT * FROM OPTIMIZER_TRACE LIMIT <LIMIT> OFFSET <OFFSET>;
@endverbatim
  which is a restriction on the trace consumer level, which saves almost
  nothing.

  @section USER_SELECT_TRACING_FEATURES How a user traces only certain
  optimizer features

@verbatim
  SET OPTIMIZER_TRACE_FEATURES="feature1=on|off,...";
@endverbatim
  where "feature1" is one optimizer feature. For example "greedy_search": a
  certain Opt_trace_array at the start of @c
  Optimize_table_order::choose_table_order() has a flag "GREEDY_SEARCH" passed
  to its constructor: this means that if the user has turned tracing of greedy
  search off, this array will not be written to the I_S trace, neither will
  any children structures. All this disabled "trace chunk" will be replaced by
  an ellipsis "...".

  @section DEV_ADDS_TRACING How a developer adds tracing to a function

  Check @c Opt_trace* usage in @c advance_sj_state():

@verbatim
  Opt_trace_array trace_choices(trace, "semijoin_strategy_choice");
@endverbatim

  This creates an array for key "semijoin_strategy_choice". We are going to
  list possible semijoin strategy choices.

@verbatim
  Opt_trace_object trace_one_strategy(trace);
@endverbatim

  This creates an object without key (normal, it's in an array). This
  object will describe one single strategy choice.

@verbatim
  trace_one_strategy.add_alnum("strategy", "FirstMatch");
@endverbatim

  This adds a key/value pair to the just-created object: key is "strategy",
  value is "FirstMatch". This is the strategy to be described in the
  just-created object.

@verbatim
  trace_one_strategy.add("cost", *current_read_time).
    add("records", *current_record_count);
  trace_one_strategy.add("chosen", (pos->sj_strategy == SJ_OPT_FIRST_MATCH));
@endverbatim

  This adds 3 key/value pairs: cost of strategy, number of records produced
  by this strategy, and whether this strategy is chosen.

  After that, there is similar code for other semijoin strategies.

  The resulting trace piece (seen in @c information_schema.OPTIMIZER_TRACE) is
@verbatim
          "semijoin_strategy_choice": [
            {
              "strategy": "FirstMatch",
              "cost": 1,
              "records": 1,
              "chosen": true
            },
            {
              "strategy": "DuplicatesWeedout",
              "cost": 1.1,
              "records": 1,
              "duplicate_tables_left": false,
              "chosen": false
            }
          ]
@endverbatim

  For more output examples, check result files of the opt_trace suite in
  @c mysql-test.

  Feature can be un-compiled with @code cmake -DOPTIMIZER_TRACE=0 @endcode.

  @section WITH_DBUG Interaction between trace and DBUG

  We don't want to have to duplicate code like this:
@verbatim
  DBUG_PRINT("info",("cost %g",cost));
  Opt_trace_object(thd->opt_trace).add("cost",cost);
@endverbatim

  Thus, any optimizer trace operation, *even* if tracing is run-time disabled,
  has an implicit DBUG_PRINT("opt",...) inside. This way, only the
  second line above is needed, and several DBUG_PRINT() could be removed from
  the Optimizer code.
  When tracing is run-time disabled, in a debug binary, traces are still
  created in order to catch the @c add() calls and write their text to DBUG,
  but those traces are not visible into INFORMATION_SCHEMA.OPTIMIZER_TRACE: we
  then say that they "don't support I_S".
  A debug binary without optimizer trace compiled in, will intentionally not
  compile.

  Because opening an object or array, or add()-ing to it, writes to DBUG
  immediately, a key/value pair and its outer object may be 100 lines
  apart in the DBUG log.

  @section ADDING_TRACING Guidelines for adding tracing

  @li Try to limit the number of distinct "words". For example, when
  describing an optimizer's decision, the words "chosen" (true/false value,
  tells whether we are choosing the said optimization), "cause" (free text
  value, tells why we are making this choice, when it's not obvious)
  can and should often be used. Having a restricted vocabulary helps
  consistency. Use "row" instead of "record". Use "tmp" instead of
  "temporary".

  @li Use only simple characters for key names: a-ZA-Z_#, and no space. '#'
  serves to denote a number, like in "select#" .

  @li Keep in mind than in an object, keys are not ordered; an application may
  parse the JSON output and output it again with keys order changed; thus
  when order matters, use an array (which may imply having anonymous objects
  as items of the array, with keys inside the anonymous objects, see how it's
  done in @c JOIN::optimize()). Keep in mind that in an object keys should
  be unique, an application may lose duplicate keys.

  @section OOM_HANDLING Handling of "out-of-memory" errors

  All memory allocations (with exceptions: see below) in the Optimizer trace
  use @c my_error() to report errors, which itself calls @c
  error_handler_hook. It is the responsibility of the API user to set up a
  proper @c error_handler_hook which will alert her/him of the OOM
  problem. When in the server, this is already the case (@c error_handler_hook
  is @c my_message_sql() which makes the statement fail).
  Note that the debug binary may crash if OOM (OOM can cause syntax
  errors...).

  @section TRACE_SECURITY Description of trace-induced security checks.

  A trace exposes information. For example if one does SELECT on a view, the
  trace contains the view's body. So, the user should be allowed to see the
  trace only if she/he has privilege to see the body, i.e. privilege to do
  SHOW CREATE VIEW.
  There are similar issues with stored procedures, functions, triggers.

  We implement this by doing additional checks on SQL objects when tracing is
  on:
  @li stored procedures, functions, triggers: checks are done when executing
  those objects
  @li base tables and views.

  Base tables or views are listed in some @c LEX::query_tables.
  The LEX may be of the executing statement (statement executed by
  @c mysql_execute_command(), or by
  @c sp_lex_keeper::reset_lex_and_exec_core()), we check this LEX in the
  constructor of Opt_trace_start.
  Or it may be a LEX describing a view, we check this LEX when
  opening the view (@c open_and_read_view()).

  Those checks are greatly simplified by disabling traces in case of security
  context changes. @see opt_trace_disable_if_no_security_context_access().

  Those checks must be done with the security context of the connected
  user. Checks with the SUID context would be useless: assume the design is
  that the basic user does not have DML privileges on tables, but only
  EXECUTE on SUID-highly-privileged routines (which implement _controlled_
  _approved_ DMLs): then the SUID context would successfully pass all
  additional privilege checks, routine would generate tracing, and the
  connected user would view the trace after the routine's execution, seeing
  secret information.

  @section NEXT What a developer should read next

  The documentation of those classes, in order
@code
  Opt_trace_context
  Opt_trace_context_impl
  Opt_trace_stmt
  Opt_trace_struct
  Opt_trace_object
  Opt_trace_array
@endcode
  and then @ref opt_trace.h as a whole.
*/

class Opt_trace_stmt;  // implementation detail local to opt_trace.cc

/**
   User-visible information about a trace. @sa Opt_trace_iterator.
*/
struct Opt_trace_info {
  /**
     String containing trace.
     If trace has been end()ed, this is 0-terminated, which is only to aid
     debugging or unit testing; this property is not relied upon in normal
     server usage.
     If trace has not been ended, this is not 0-terminated. That rare case can
     happen when a substatement reads OPTIMIZER_TRACE (at that stage, the top
     statement is still executing so its trace is not ended yet, but may still
     be read by the sub-statement).
  */
  const char *trace_ptr;
  size_t trace_length;  ///< length of trace string
  //// String containing original query. 0-termination: like trace_ptr.
  const char *query_ptr;
  size_t query_length;                ///< length of query string
  const CHARSET_INFO *query_charset;  ///< charset of query string
  /**
    How many bytes this trace is missing (for traces which were truncated
    because of @@@@optimizer-trace-max-mem-size).
  */
  size_t missing_bytes;
  bool missing_priv;  ///< whether user lacks privilege to see this trace
};

/**
   Iterator over the list of remembered traces.
   @note due to implementation, the list must not change during an
   iterator's lifetime, or results may be unexpected (no crash though).
*/
class Opt_trace_iterator {
 public:
  /**
    @param  ctx  context
  */
  Opt_trace_iterator(Opt_trace_context *ctx);

  void next();  ///< Advances iterator to next trace.

  /**
     Provides information about the trace on which the iterator is
     positioned.
     @param[out]  info  information returned.
     The usage pattern is
     1) instantiate the iterator
     2) test at_end(), if false: call get_value() and then next()
     3) repeat (2) until at_end() is true.
  */
  void get_value(Opt_trace_info *info) const;

  /// @returns whether iterator is positioned to the end.
  bool at_end() const { return cursor == nullptr; }

 private:
  /// Pointer to context, from which traces are retrieved
  Opt_trace_context *ctx;
  const Opt_trace_stmt *cursor;  ///< trace which the iterator is positioned on
  long row_count;                ///< how many traces retrieved so far
};

/**
   Object and array are both "structured data" and have lots in common, so the
   Opt_trace_struct is a base class for them.
   When you want to add a structure to the trace, you create an instance of
   Opt_trace_object or Opt_trace_array, then you add information to it with
   add(), then the destructor closes the structure (we use RAII, Resource
   Acquisition Is Initialization).
*/

class Opt_trace_struct {
 protected:
  /**
     @param  ctx_arg  Optimizer trace context for this structure
     @param  requires_key_arg  whether this structure requires/forbids keys
                      for values put inside it (an object requires them, an
                      array forbids them)
     @param  key      key if this structure is the value of a key/value pair,
                      NULL otherwise. This pointer must remain constant and
                      valid until the object is destroyed (to support
                      @ref saved_key).
     @param  feature  optimizer feature to which this structure belongs

     This constructor is never called directly, only from subclasses.
  */
  Opt_trace_struct(Opt_trace_context *ctx_arg, bool requires_key_arg,
                   const char *key, Opt_trace_context::feature_value feature)
      : started(false) {
    // A first inlined test
    if (unlikely(ctx_arg->is_started())) {
      // Tracing enabled: must fully initialize the structure.
      do_construct(ctx_arg, requires_key_arg, key, feature);
    }
    /*
      Otherwise, just leave "started" to false, it marks that the structure is
      dummy.
    */
  }
  ~Opt_trace_struct() {
    if (unlikely(started)) do_destruct();
  }

 public:
  /**
    The exception to RAII: this function is an explicit way of ending a
    structure before it goes out of scope. Don't use it unless RAII mandates
    a new scope which mandates re-indenting lots of code lines.
  */
  void end() {
    if (unlikely(started)) do_destruct();
  }

  /**
     Adds a value (of string type) to the structure. A key is specified, so it
     adds the key/value pair (the structure must thus be an object).

     There are two "add_*" variants to add a string value.
     If the value is 0-terminated and each character
     - is ASCII 7-bit
     - has ASCII code >=32 and is neither '"' nor '\\'
     then add_alnum() should be used. That should be the case for all fixed
     strings like add_alnum("cause", "cost").
     Otherwise, add_utf8() should be used; it accepts any UTF8-encoded
     character in 'value' and will escape characters which JSON requires (and
     is thus slower than add_alnum()). It should be used for all strings which
     we get from the server's objects (indeed a table's name, a WHERE
     condition, may contain "strange" characters).

     @param  key    key
     @param  value  value
     @returns a reference to the structure, useful for chaining like this:
     @verbatim add(x,y).add(z,t).add(u,v) @endverbatim

     String-related add() variants are named add_[something]():
     - to avoid confusing the compiler between:
     add(const char *value, size_t    val_length) and
     add(const char *key,   ulonglong value)
     - and because String::length() returns uint32 and not size_t, so for
     add(str.ptr(), str.length())
     compiler may pick
     add(const char *key,   double value) instead of
     add(const char *value, size_t val_length).
  */
  Opt_trace_struct &add_alnum(const char *key, const char *value) {
    if (likely(!started)) return *this;
    return do_add(key, value, strlen(value), false);
  }

  /**
     Adds a value (of string type) to the structure. No key is specified, so
     it adds only the value (the structure must thus be an array).
     @param  value  value
     @returns a reference to the structure
  */
  Opt_trace_struct &add_alnum(const char *value) {
    if (likely(!started)) return *this;
    return do_add(nullptr, value, strlen(value), false);
  }

  /**
     Like add_alnum() but supports any UTF8 characters in 'value'.
     Will "escape" 'value' to be JSON-compliant.
     @param  key         key
     @param  value       value
     @param  val_length  length of string 'value'
  */
  Opt_trace_struct &add_utf8(const char *key, const char *value,
                             size_t val_length) {
    if (likely(!started)) return *this;
    return do_add(key, value, val_length, true);
  }

  /// Variant of add_utf8() for adding to an array (no key)
  Opt_trace_struct &add_utf8(const char *value, size_t val_length) {
    if (likely(!started)) return *this;
    return do_add(nullptr, value, val_length, true);
  }

  /// Variant of add_utf8() where 'value' is 0-terminated
  Opt_trace_struct &add_utf8(const char *key, const char *value) {
    if (likely(!started)) return *this;
    return do_add(key, value, strlen(value), true);
  }

  /// Variant of add_utf8() where 'value' is 0-terminated
  Opt_trace_struct &add_utf8(const char *value) {
    if (likely(!started)) return *this;
    return do_add(nullptr, value, strlen(value), true);
  }

  /**
     Add a value (of Item type) to the structure. The Item should be a
     condition (like a WHERE clause) which will be pretty-printed into the
     trace. This is useful for showing condition transformations (equality
     propagation etc).
     @param  key    key
     @param  item   the Item
     @return a reference to the structure
  */
  Opt_trace_struct &add(const char *key, Item *item) {
    if (likely(!started)) return *this;
    return do_add(key, item);
  }
  Opt_trace_struct &add(Item *item) {
    if (likely(!started)) return *this;
    return do_add(nullptr, item);
  }

 public:
  Opt_trace_struct &add(const char *key, bool value) {
    if (likely(!started)) return *this;
    return do_add(key, value);
  }
  Opt_trace_struct &add(bool value) {
    if (likely(!started)) return *this;
    return do_add(nullptr, value);
  }
  Opt_trace_struct &add(const char *key, int value) {
    if (likely(!started)) return *this;
    return do_add(key, static_cast<longlong>(value));
  }
  Opt_trace_struct &add(int value) {
    if (likely(!started)) return *this;
    return do_add(nullptr, static_cast<longlong>(value));
  }
  Opt_trace_struct &add(const char *key, uint value) {
    if (likely(!started)) return *this;
    return do_add(key, static_cast<ulonglong>(value));
  }
  Opt_trace_struct &add(uint value) {
    if (likely(!started)) return *this;
    return do_add(nullptr, static_cast<ulonglong>(value));
  }
  Opt_trace_struct &add(const char *key, ulong value) {
    if (likely(!started)) return *this;
    return do_add(key, static_cast<ulonglong>(value));
  }
  Opt_trace_struct &add(ulong value) {
    if (likely(!started)) return *this;
    return do_add(nullptr, static_cast<ulonglong>(value));
  }
  Opt_trace_struct &add(const char *key, longlong value) {
    if (likely(!started)) return *this;
    return do_add(key, value);
  }
  Opt_trace_struct &add(longlong value) {
    if (likely(!started)) return *this;
    return do_add(nullptr, value);
  }
  Opt_trace_struct &add(const char *key, ulonglong value) {
    if (likely(!started)) return *this;
    return do_add(key, value);
  }
  Opt_trace_struct &add(ulonglong value) {
    if (likely(!started)) return *this;
    return do_add(nullptr, value);
  }
  Opt_trace_struct &add(const char *key, double value) {
    if (likely(!started)) return *this;
    return do_add(key, value);
  }
  Opt_trace_struct &add(double value) {
    if (likely(!started)) return *this;
    return do_add(nullptr, value);
  }
  /// Adds a 64-bit integer to trace, in hexadecimal format
  Opt_trace_struct &add_hex(const char *key, uint64 value) {
    if (likely(!started)) return *this;
    return do_add_hex(key, value);
  }
  Opt_trace_struct &add_hex(uint64 value) {
    if (likely(!started)) return *this;
    return do_add_hex(nullptr, value);
  }
  /// Adds a JSON null object (==Python's "None")
  Opt_trace_struct &add_null(const char *key) {
    if (likely(!started)) return *this;
    return do_add_null(key);
  }
  /**
     Helper to put the database/table name in an object.
     @param  tab  TABLE* pointer
  */
  Opt_trace_struct &add_utf8_table(const TABLE_LIST *tab) {
    if (likely(!started)) return *this;
    return do_add_utf8_table(tab);
  }
  /**
     Helper to put the number of select_lex in an object.
     @param  select_number  number of select_lex
  */
  Opt_trace_struct &add_select_number(uint select_number) {
    return unlikely(select_number >= INT_MAX) ?
                                              // Clearer than any huge number.
               add_alnum("select#", "fake")
                                              : add("select#", select_number);
  }
  /**
     Add a value to the structure.
     @param  key    key
     @param  cost   the value of Cost_estimate
     @return a reference to the structure
  */
  Opt_trace_struct &add(const char *key, const Cost_estimate &cost) {
    if (likely(!started)) return *this;
    return do_add(key, cost);
  }

  /**
    Informs this structure that we are adding data (scalars, structures) to
    it.
    This is used only if sending to I_S.
    @returns whether the structure was empty so far.
    @note this is reserved for use by Opt_trace_stmt.
  */
  bool set_not_empty() {
    const bool old_empty = empty;
    empty = false;
    return old_empty;
  }
  /**
    Validates the key about to be added.
    @note this is reserved for use by Opt_trace_stmt.

    When adding a value (or array or object) to an array, or a key/value pair
    to an object, we need to know this outer array or object.

    It would be possible, when trying to add a key to an array, which is wrong
    in JSON, or similarly when trying to add a value without any key to an
    object, to catch it at compilation time, if the adder received, as
    function parameter, the type of the structure (like @c
    Opt_trace_array*). Then the @c add(key,val) call would not compile as
    Opt_trace_array wouldn't feature it.

    But as explained in comment of class Opt_trace_context we
    cannot pass down the object, have to maintain a "current object or
    array" in the Opt_trace_context context (pointer to an instance of
    Opt_trace_struct), and the adder grabs it from the context.

    As this current structure is of type "object or array", we cannot do
    compile-time checks that only suitable functions are used. A call to @c
    add(key,value) is necessarily legal for the compiler as the structure may
    be an object, though it will be wrong in case the structure is actually
    an array at run-time. Thus we have the risk of an untested particular
    situation where the current structure is not an object (but an array)
    though the code expected it to be one. This happens in practice, because
    subqueries are evaluated in many possible places of code, not all of them
    being known. Same happens, to a lesser extent, with calls to the range
    optimizer.
    So at run-time, in check_key(), we detect wrong usage, like adding a value
    to an object without specifying a key, and then remove the unnecessary
    key, or add an autogenerated key.
  */
  const char *check_key(const char *key);

 private:
  /// Not implemented, use add_alnum() instead.
  Opt_trace_struct &add(const char *key, const char *value);
  Opt_trace_struct &add(const char *key);

  /// Full initialization. @sa Opt_trace_struct::Opt_trace_struct
  void do_construct(Opt_trace_context *ctx, bool requires_key, const char *key,
                    Opt_trace_context::feature_value feature);
  /// Really does destruction
  void do_destruct();
  /**
     Really adds to the object. @sa add().

     @note add() has an up-front if(), hopefully inlined, so that in the
     common case - tracing run-time disabled - we have no function call. If
     tracing is enabled, we call do_add().
     In a 20-table plan search (as in BUG#50595), the execution time was
     decreased from 2.6 to 2.0 seconds thanks to this inlined-if trick.

     @param key         key
     @param value       value
     @param val_length  length of string 'value'
     @param escape      do JSON-compliant escaping of 'value'. If 'escape' is
     false, 'value' should be ASCII. Otherwise, should be UTF8.
  */
  Opt_trace_struct &do_add(const char *key, const char *value,
                           size_t val_length, bool escape);
  Opt_trace_struct &do_add(const char *key, Item *item);
  Opt_trace_struct &do_add(const char *key, bool value);
  Opt_trace_struct &do_add(const char *key, longlong value);
  Opt_trace_struct &do_add(const char *key, ulonglong value);
  Opt_trace_struct &do_add(const char *key, double value);
  Opt_trace_struct &do_add_hex(const char *key, uint64 value);
  Opt_trace_struct &do_add_null(const char *key);
  Opt_trace_struct &do_add_utf8_table(const TABLE_LIST *tab);
  Opt_trace_struct &do_add(const char *key, const Cost_estimate &value);

  Opt_trace_struct(const Opt_trace_struct &);             ///< not defined
  Opt_trace_struct &operator=(const Opt_trace_struct &);  ///< not defined

  bool started;  ///< Whether the structure does tracing or is dummy

  /**
     Whether the structure requires/forbids keys for values inside it.
     true: this is an object. false: this is an array.

     @note The canonical way would be to not have such bool per instance, but
     rather have a pure virtual member function
     Opt_trace_struct::requires_key(), overloaded by Opt_trace_object
     (returning true) and by Opt_trace_array (returning false). But
     Opt_trace_object::requires_key() would not be accessible from
     Opt_trace_struct::do_construct() (which would complicate coding), whereas
     the bool is.
  */
  bool requires_key;

  /**
    Whether this structure caused tracing to be disabled in this statement
    because belonging to a not-traced optimizer feature, in accordance with
    the value of @@@@optimizer_trace_features.
  */
  bool has_disabled_I_S;
  bool empty;            ///< @see set_not_empty()
  Opt_trace_stmt *stmt;  ///< Trace owning the structure
  /// Key if the structure is the value of a key/value pair, NULL otherwise
  const char *saved_key;
#ifndef DBUG_OFF
  /**
     Fixed-length prefix of previous key in this structure, if this structure
     is an object. Serves to detect when adding two same consecutive keys to
     an object, which would be wrong.
  */
  char previous_key[25];
#endif
};

/**
   A JSON object (unordered set of key/value pairs).
   Defines only a constructor, all the rest is inherited from
   Opt_trace_struct.
*/
class Opt_trace_object : public Opt_trace_struct {
 public:
  /**
     Constructs an object. Key is specified, so the object is the value of a
     key/value pair.
     @param  ctx  context for this object
     @param  key  key
     @param  feature  optimizer feature to which this structure belongs
  */
  Opt_trace_object(
      Opt_trace_context *ctx, const char *key,
      Opt_trace_context::feature_value feature = Opt_trace_context::MISC)
      : Opt_trace_struct(ctx, true, key, feature) {}
  /**
     Constructs an object. No key is specified, so the object is just a value
     (serves for the single root object or for adding the object to an array).
     @param  ctx  context for this object
     @param  feature  optimizer feature to which this structure belongs
  */
  Opt_trace_object(
      Opt_trace_context *ctx,
      Opt_trace_context::feature_value feature = Opt_trace_context::MISC)
      : Opt_trace_struct(ctx, true, nullptr, feature) {}
};

/**
   A JSON array (ordered set of values).
   Defines only a constructor, all the rest in inherited from
   Opt_trace_struct.
*/
class Opt_trace_array : public Opt_trace_struct {
 public:
  /**
     Constructs an array. Key is specified, so the array is the value of a
     key/value pair.
     @param  ctx  context for this array
     @param  key  key
     @param  feature  optimizer feature to which this structure belongs
  */
  Opt_trace_array(
      Opt_trace_context *ctx, const char *key,
      Opt_trace_context::feature_value feature = Opt_trace_context::MISC)
      : Opt_trace_struct(ctx, false, key, feature) {}
  /**
     Constructs an array. No key is specified, so the array is just a value
     (serves for adding the object to an array).
     @param  ctx  context for this array
     @param  feature  optimizer feature to which this structure belongs
  */
  Opt_trace_array(
      Opt_trace_context *ctx,
      Opt_trace_context::feature_value feature = Opt_trace_context::MISC)
      : Opt_trace_struct(ctx, false, nullptr, feature) {}
};

/**
   Instantiate an instance of this class for specific cases where
   optimizer trace, in a certain section of Optimizer code, should write only
   to DBUG and not I_S. Example: see sql_select.cc.
   Note that this class should rarely be used; the "feature" parameter of
   Opt_trace_struct is a good alternative.
*/
class Opt_trace_disable_I_S {
 public:
  /**
     @param  ctx_arg      Context.
     @param  disable_arg  Whether the instance should really disable
                          anything. If false, the object is dummy. If true,
                          tracing-to-I_S is disabled at construction and
                          re-enabled at destruction.
     @details A dummy instance is there only for RAII reasons. Imagine we want
     to do this:
@verbatim
     {
       if (x) disable tracing;
       code;
     } // tracing should be re-enabled here
@endverbatim
     As we use RAII, we cannot put the instance declaration inside if(x):
@verbatim
     {
       if (x) Opt_trace_disable_I_S instance(ctx);
       code;
     }
@endverbatim
     because it would be destroyed as soon as the if() block is left, so
     tracing would be re-enabled before @c code;. It should rather be written
     as:
@verbatim
     {
       Opt_trace_disable_I_S instance(ctx, x); // if !x, does nothing
       code;
     } // re-enabling happens here, if x is true
@endverbatim
  */
  Opt_trace_disable_I_S(Opt_trace_context *ctx_arg, bool disable_arg) {
    if (disable_arg) {
      ctx = ctx_arg;
      ctx->disable_I_S_for_this_and_children();
    } else
      ctx = nullptr;
  }

  /// Destructor. Restores trace's "enabled" property to its previous value.
  ~Opt_trace_disable_I_S() {
    if (ctx != nullptr) ctx->restore_I_S();
  }

 private:
  /** Context. Non-NULL if and only if this instance really does disabling */
  Opt_trace_context *ctx;
  Opt_trace_disable_I_S(const Opt_trace_disable_I_S &);  // not defined
  Opt_trace_disable_I_S &operator=(
      const Opt_trace_disable_I_S &);  // not defined
};

/**
   @name Helpers connecting the optimizer trace to THD or Information Schema.
*/

//@{

class Opt_trace_start {
 public:
  /**
    Instantiate this class to start tracing a THD's actions (generally at a
    statement's start), and to set the "original" query (not transformed, as
    sent by client) for the new trace. Destructor will end the trace.

    If in a routine's instruction, there is no "query". To be helpful to the
    user, we craft a query using the instruction's print(). We don't leave this
    to the caller, because it would be inefficient if tracing is off.

    @param  thd_arg       the THD
    @param  tbl           list of tables read/written by the statement.
    @param  sql_command   SQL command being prepared or executed
    @param  set_vars      what variables are set by this command (only used if
                          sql_command is SQLCOM_SET_OPTION)
    @param  query         query
    @param  query_length  query's length
    @param  instr         routine's instruction, if applicable; if so, 'query'
                          and 'query_length' above are ignored
    @param  query_charset charset which was used to encode this query

    @note Each sub-statement is responsible for ending the trace which it
    has started; this class achieves this by keeping some memory inside.
  */
  Opt_trace_start(THD *thd_arg, TABLE_LIST *tbl,
                  enum enum_sql_command sql_command,
                  List<set_var_base> *set_vars, const char *query,
                  size_t query_length, sp_printable *instr,
                  const CHARSET_INFO *query_charset);
  ~Opt_trace_start();

 private:
  Opt_trace_context *const ctx;
  bool error;  ///< whether trace start() had an error
};

class SELECT_LEX;

/**
   Prints SELECT query to optimizer trace. It is not the original query (as in
   @c Opt_trace_context::set_query()) but a printout of the parse tree
   (Item-s).
   @param  thd         the THD
   @param  select_lex  query's parse tree
   @param  trace_object  Opt_trace_object to which the query will be added
*/
void opt_trace_print_expanded_query(const THD *thd, SELECT_LEX *select_lex,
                                    Opt_trace_object *trace_object);

/**
   If the security context is not that of the connected user, inform the trace
   system that a privilege is missing. With one exception: see below.

   @param thd

   This serves to eliminate the following issue.
   Any information readable by a SELECT may theoretically end up in
   the trace. And a SELECT may read information from other places than tables:
   - from views (reading their bodies)
   - from stored routines (reading their bodies)
   - from files (reading their content), with LOAD_FILE()
   - from the list of connections (reading their queries...), with
   I_S.PROCESSLIST.
   If the connected user has EXECUTE privilege on a routine which does a
   security context change, the routine can retrieve information internally
   (if allowed by the SUID context's privileges), and present only a portion
   of it to the connected user. But with tracing on, all information is
   possibly in the trace. So the connected user receives more information than
   the routine's definer intended to provide.  Fixing this issue would require
   adding, near many privilege checks in the server, a new
   optimizer-trace-specific check done against the connected user's context,
   to verify that the connected user has the right to see the retrieved
   information.

   Instead, our chosen simpler solution is that if we see a security context
   change where SUID user is not the connected user, we disable tracing. With
   only one safe exception: if the connected user has all global privileges
   (because then she/he can find any information anyway). By "all global
   privileges" we mean everything but WITH GRANT OPTION (that latter one isn't
   related to information gathering).

   Read access to I_S.OPTIMIZER_TRACE by another user than the connected user
   is restricted: @see fill_optimizer_trace_info().
*/
void opt_trace_disable_if_no_security_context_access(THD *thd);

/**
   If tracing is on, checks additional privileges for a view, to make sure
   that the user has the right to do SHOW CREATE VIEW. For that:
   - this function checks SHOW VIEW
   - SELECT is tested in opt_trace_disable_if_no_tables_access()
   - SELECT + SHOW VIEW is sufficient for SHOW CREATE VIEW.
   We also check underlying tables.
   If a privilege is missing, notifies the trace system.
   This function should be called when the view's underlying tables have not
   yet been merged.

   @param thd               THD context
   @param view              view to check
   @param underlying_tables underlying tables/views of 'view'
 */
void opt_trace_disable_if_no_view_access(THD *thd, TABLE_LIST *view,
                                         TABLE_LIST *underlying_tables);

/**
  If tracing is on, checks additional privileges on a stored routine, to make
  sure that the user has the right to do SHOW CREATE PROCEDURE/FUNCTION. For
  that, we use the same checks as in those SHOW commands.
  If a privilege is missing, notifies the trace system.

  This function is not redundant with
  opt_trace_disable_if_no_security_context_access().
  Indeed, for a SQL SECURITY INVOKER routine, there is no context change, but
  we must still verify that the invoker can do SHOW CREATE.

  For triggers, see note in sp_head::execute_trigger().

  @param thd
  @param sp  routine to check
 */
void opt_trace_disable_if_no_stored_proc_func_access(THD *thd, sp_head *sp);

/**
   Fills information_schema.OPTIMIZER_TRACE with rows (one per trace)
   @retval 0 ok
   @retval 1 error
*/
int fill_optimizer_trace_info(THD *thd, TABLE_LIST *tables, Item *);

//@}

/**
   Helper for defining query-transformation-related trace objects in one
   code line. This produces
   {
     "transformation": {
       "select#": @<select_number@>,
       "from": @<from@>,
       "to": @<to@>
   The objects are left open, so that one can add more to them (often a
   "chosen" property after making some computation). Objects get closed when
   going out of scope as usual.
   @param trace          optimizer trace
   @param object_level0  name of the outer Opt_trace_object C++ object
   @param object_level1  name of the inner Opt_trace_object C++ object
   @param select_number  number of the being-transformed SELECT_LEX
   @param from           description of the before-transformation state
   @param to             description of the after-transformation state
*/
#define OPT_TRACE_TRANSFORM(trace, object_level0, object_level1, \
                            select_number, from, to)             \
  Opt_trace_object object_level0(trace);                         \
  Opt_trace_object object_level1(trace, "transformation");       \
  object_level1.add_select_number(select_number);                \
  object_level1.add_alnum("from", from).add_alnum("to", to);

#endif /* OPT_TRACE_INCLUDED */
