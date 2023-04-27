/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef OPT_TRACE_CONTEXT_INCLUDED
#define OPT_TRACE_CONTEXT_INCLUDED

#include "my_dbug.h"
#include "my_inttypes.h"
#include "mysql/psi/psi_base.h"
#include "prealloced_array.h"

/**
   @file
   This contains the declaration of class Opt_trace_context, which is needed
   to declare THD.
   It is recommend to read opt_trace.h first.
*/

class Opt_trace_stmt;  // implementation detail local to opt_trace.cc

typedef Prealloced_array<Opt_trace_stmt *, 16> Opt_trace_stmt_array;

/**
  @class Opt_trace_context

  A per-session context which is always available at any point of execution,
  because in practice it's accessible from THD which contains:
  @verbatim Opt_trace_context opt_trace; @endverbatim
  It maintains properties of the session's regarding tracing: enabled/disabled
  state, style (all trace on one line, or not, etc), a list of all remembered
  traces of previous and current SQL statement (as restricted by
  OFFSET/LIMIT), and a pointer to the current (being-generated) trace (which
  itself has a pointer to its current open object/array).

  Here is why the context needs to provide the current open object/array:

  @li When adding a value (scalar or array or object) to an array, or adding a
  key/value pair to an object, we need this outer object or array (from now
  on, we will use the term "structure" for "object or array", as both are
  structured types).

  @li The most natural way would be that the outer object would be passed in
  argument to the adder (the function which wants to add the value or
  key/value).

  @li But tracing is sometimes produced from deep down the stack trace, with
  many intermediate frames doing no tracing (writing nothing to the trace), so
  it would require passing the outer structure through many levels, thus
  modifying many function prototypes.
  Example (in gdb "backtrace" notation: inner frames first):
@verbatim
    #0  Item_in_subselect::single_value_transformer
        - opens an object for key "transformation"
    #1  Item_in_subselect::select_in_like_transformer - does no tracing
    #2  Item_allany_subselect::select_transformer - does no tracing
    #3  SELECT_LEX::prepare - opens an object for key "join_preparation"
@endverbatim
  So the object opened in #3 would have to be passed in argument to #2 and #1
  in order to finally reach #0 where object "transformation" would be added to
  it.

  @li So, as we cannot practically pass the object down, we rather maintain a
  "current object or array" accessible from the Opt_trace_context context;
  it's a pointer to an instance of Opt_trace_struct, and the function deep
  down (frame #0) grabs it from the context, where it was depositted by the
  function high up (frame #3 in the last example).
*/

class Opt_trace_context {
 public:
  Opt_trace_context() : pimpl(nullptr), I_S_disabled(0) {}
  ~Opt_trace_context();

  /**
     Starts a new trace.
     @param  support_I_S      Whether this statement should have its trace in
                              information_schema
     @param  support_dbug_or_missing_priv  'true' if this statement
                              should have its trace in the dbug log (--debug),
                              or if missing_privilege() may be called on this
                              trace
     @param  end_marker       For a key/(object|array) pair, should the key be
                              repeated in a comment when the object|array
                              closes? like
@verbatim
                              "key_foo": {
                                           multi-line blah
                                         } / * key_foo * /
@endverbatim
                              This is for human-readability only, not valid in
                              JSON. Note that YAML supports #-prefixed
                              comments (we would just need to put the next
                              item's "," before the current item's "#").
     @param  one_line         Should the trace be on a single line without
                              indentation? (More compact for network transfer
                              to programs, less human-readable.)
     @param  offset           Offset for restricting trace production.
     @param  limit            Limit for restricting trace production.
     @param  max_mem_size     Maximum allowed for cumulated size of all
                              remembered traces.
     @param  features         Only these optimizer features should be traced.

     @retval false            ok
     @retval true             error (OOM): instance is unusable, so only
                              destructor is permitted on it; any other
                              member function has undefined effects.
  */
  bool start(bool support_I_S, bool support_dbug_or_missing_priv,
             bool end_marker, bool one_line, long offset, long limit,
             ulong max_mem_size, ulonglong features);

  /**
    Ends the current (=open, unfinished, being-generated) trace.

    If @c missing_privilege() has been called between start() and end(),
    end() restores I_S support to what it was before the call to
    @c missing_privilege(). This is the key to ensure that missing_privilege()
    does not disable I_S support for the rest of the connection's life!
  */
  void end();

  /// Returns whether there is a current trace
  bool is_started() const {
    return unlikely(pimpl != nullptr) && pimpl->current_stmt_in_gen != nullptr;
  }

  /**
     @returns whether the current trace writes to I_S.
     This function should rarely be used. Don't you use this for some clever
     optimizations bypassing opt trace!
  */
  bool support_I_S() const;

  /**
     Set the "original" query (not transformed, as sent by client) for the
     current trace.
     @param   query    query
     @param   length   query's length
     @param   charset  charset which was used to encode this query
  */
  void set_query(const char *query, size_t length, const CHARSET_INFO *charset);

  /**
     Brainwash: deletes all remembered traces and resets counters regarding
     OFFSET/LIMIT (so that the next statement is considered as "at offset
     0"). Does not reset the @@@@optimizer_trace_offset/limit variables.
  */
  void reset();

  /// @sa parameters of Opt_trace_context::start()
  bool get_end_marker() const { return pimpl->end_marker; }
  /// @sa parameters of Opt_trace_context::start()
  bool get_one_line() const { return pimpl->one_line; }

  /**
     Names of flags for @@@@optimizer_trace variable of @c sys_vars.cc :
     @li "enabled" = tracing enabled
     @li "one_line"= see parameter of @ref Opt_trace_context::start
     @li "default".
  */
  static const char *flag_names[];

  /** Flags' numeric values for @@@@optimizer_trace variable */
  enum { FLAG_DEFAULT = 0, FLAG_ENABLED = 1 << 0, FLAG_ONE_LINE = 1 << 1 };

  /**
     Features' names for @@@@optimizer_trace_features variable of
     @c sys_vars.cc:
     @li "greedy_search" = the greedy search for a plan
     @li "range_optimizer" = the cost analysis of accessing data through
     ranges in indexes
     @li "dynamic_range" = the range optimization performed for each record
                           when access method is dynamic range
     @li "repeated_subselect" = the repeated execution of subselects
     @li "default".
  */
  static const char *feature_names[];

  /** Features' numeric values for @@@@optimizer_trace_features variable */
  enum feature_value {
    GREEDY_SEARCH = 1 << 0,
    RANGE_OPTIMIZER = 1 << 1,
    DYNAMIC_RANGE = 1 << 2,
    REPEATED_SUBSELECT = 1 << 3,
    /*
      If you add here, update feature_value of empty implementation
      and default_features!
    */
    /**
       Anything unclassified, including the top object (thus, by "inheritance
       from parent", disabling MISC makes an empty trace).
       This feature cannot be disabled by the user; for this it is important
       that it always has biggest flag; flag's value itself does not matter.
    */
    MISC = 1 << 7
  };

  /**
     User lacks privileges to see the current trace. Make the trace appear
     empty in Opt_trace_info, and disable I_S for all its upcoming children.

     Once a call to this function has been made, subsequent calls to it before
     @c end() have no effects.
  */
  void missing_privilege();

  /// Optimizer features which are traced by default.
  static const feature_value default_features;

  /**
     @returns whether an optimizer feature should be traced.
     @param  f  feature
  */
  bool feature_enabled(feature_value f) const {
    return unlikely(pimpl != nullptr) && (pimpl->features & f);
  }

  /**
     Opt_trace_struct is passed Opt_trace_context*, and needs to know
     to which statement's trace to attach, so Opt_trace_context must provide
     this information.
  */
  Opt_trace_stmt *get_current_stmt_in_gen() {
    return pimpl->current_stmt_in_gen;
  }

  /**
     @returns the next statement to show in I_S.
     @param[in,out]  got_so_far  How many statements the caller got so far
     (by previous calls to this function); function updates this count.
     @note traces are returned from oldest to newest.
   */
  const Opt_trace_stmt *get_next_stmt_for_I_S(long *got_so_far) const;

  /// Temporarily disables I_S for this trace and its children.
  void disable_I_S_for_this_and_children() {
    ++I_S_disabled;
    if (unlikely(pimpl != nullptr)) pimpl->disable_I_S_for_this_and_children();
  }

  /**
     Restores I_S support to what it was before the previous call to
     disable_I_S_for_this_and_children().
  */
  void restore_I_S() {
    --I_S_disabled;
    DBUG_ASSERT(I_S_disabled >= 0);
    if (unlikely(pimpl != nullptr)) pimpl->restore_I_S();
  }

 private:
  /**
     To have the smallest impact on THD's size, most of the implementation is
     moved to a separate class Opt_trace_context_impl which is instantiated on
     the heap when really needed. So if a connection never sets
     @@@@optimizer_trace to "enabled=on" and does not use --debug, this heap
     allocation never happens.
     This class is declared here so that frequently called functions like
     Opt_trace_context::is_started() can be inlined.
  */
  class Opt_trace_context_impl {
   public:
    Opt_trace_context_impl()
        : current_stmt_in_gen(nullptr),
          stack_of_current_stmts(PSI_INSTRUMENT_ME),
          all_stmts_for_I_S(PSI_INSTRUMENT_ME),
          all_stmts_to_del(PSI_INSTRUMENT_ME),
          features(feature_value(0)),
          offset(0),
          limit(0),
          since_offset_0(0) {}

    void disable_I_S_for_this_and_children();
    void restore_I_S();

    /**
       Trace which is currently being generated, where structures are being
       added. "in_gen" stands for "in generation", being-generated.

       In simple cases it is equal to the last element of array
       all_stmts_for_I_S. But it can be prior to it, for example when calling
       a stored routine:
@verbatim
       CALL statement starts executing
         create trace of CALL (call it "trace #1")
         add structure to trace #1
         add structure to trace #1
         First sub-statement executing
           create trace of sub-statement (call it "trace #2")
           add structure to trace #2
           add structure to trace #2
         First sub-statement ends
         add structure to trace #1
@endverbatim
       In the beginning, the CALL statement's trace is the newest and current;
       when the first sub-statement is executing, that sub-statement's trace
       is the newest and current; when the first sub-statement ends, it is
       still the newest but it's not the current anymore: the current is then
       again the CALL's one, where structures will be added, until a second
       sub-statement is executed.
       Another case is when the current statement sends only to DBUG:
       all_stmts_for_I_S lists only traces shown in OPTIMIZER_TRACE.
    */
    Opt_trace_stmt *current_stmt_in_gen;

    /**
       To keep track of what is the current statement, as execution goes into
       a sub-statement, and back to the upper statement, we have a stack of
       successive values of current_stmt_in_gen:
       when in a statement we enter a substatement (a new trace), we push the
       statement's trace on the stack and change current_stmt_in_gen to the
       substatement's trace; when leaving the substatement we pop from the
       stack and set current_stmt_in_gen to the popped value.
    */
    Opt_trace_stmt_array stack_of_current_stmts;

    /**
       List of remembered traces for putting into the OPTIMIZER_TRACE
       table. Element 0 is the one created first, will be first row of
       OPTIMIZER_TRACE table. The array structure fullfills those needs:
       - to output traces "oldest first" in OPTIMIZER_TRACE
       - to preserve traces "newest first" when @@@@optimizer_trace_offset<0
       - to delete a trace in the middle of the list when it is permanently
       out of the offset/limit showable window.
    */
    Opt_trace_stmt_array all_stmts_for_I_S;
    /**
       List of traces which are unneeded because of OFFSET/LIMIT, and
       scheduled for deletion from memory.
    */
    Opt_trace_stmt_array all_stmts_to_del;

    bool end_marker;  ///< copy of parameter of Opt_trace_context::start
    bool one_line;
    feature_value features;
    long offset;
    long limit;
    size_t max_mem_size;

    /**
       Number of statements traced so far since "offset 0", for comparison
       with OFFSET and LIMIT, when OFFSET >= 0.
    */
    long since_offset_0;
  };

  Opt_trace_context_impl *pimpl;  /// Dynamically allocated implementation.

  /**
    <>0 <=> any to-be-created statement's trace should not be in
    information_schema. This applies to next statements, their substatements,
    etc.
  */
  int I_S_disabled;

  /**
     Find and delete unneeded traces.
     For example if OFFSET=-1,LIMIT=1, only the last trace is needed. When a
     new trace is started, the previous traces becomes unneeded and this
     function deletes them which frees memory.
     @param  purge_all  if true, ignore OFFSET and thus delete everything
  */
  void purge_stmts(bool purge_all);

  /**
     Compute maximum allowed memory size for current trace. The current trace
     is the only one active. Other traces break down in two groups:
     - the finished ones (from previously executed statements),
     - the "started but not finished ones": they are not current, are not
     being updated at this moment: this must be the trace of a top
     statement calling a substatement which is the current trace now: trace's
     top statement is not being updated at this moment.
     So the current trace can grow in the room left by all traces above.
  */
  size_t allowed_mem_size_for_current_stmt() const;

  /// Not defined copy constructor, to disallow copy.
  Opt_trace_context(const Opt_trace_context &);
  /// Not defined assignment operator, to disallow assignment.
  Opt_trace_context &operator=(const Opt_trace_context &);
};

#endif /* OPT_TRACE_CONTEXT_INCLUDED */
