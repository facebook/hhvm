/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef WINDOWS_INCLUDED
#define WINDOWS_INCLUDED

#include <sys/types.h>
#include <cstring>  // std::memcpy

#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/enum_query_type.h"
#include "sql/handler.h"
#include "sql/mem_root_array.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/table.h"

/*
  Some Window-related symbols must be known to sql_lex.h which is a frequently
  included header.
  To avoid that any change to window.h causes a recompilation of the whole
  Server, those symbols go into this header:
*/
#include "sql/window_lex.h"

class Cached_item;
class Item;
class Item_func;
class Item_string;
class Item_sum;
class PT_border;
class PT_frame;
class PT_order_list;
class PT_window;
class String;
class THD;
class Temp_table_param;

/**
  Position hints for the frame buffer are saved for these kind of row
  accesses, cf. #Window::m_frame_buffer_positions.
*/
enum class Window_retrieve_cached_row_reason {
  WONT_UPDATE_HINT = -1,  // special value when using restore_special_record
  FIRST_IN_PARTITION = 0,
  CURRENT = 1,
  FIRST_IN_FRAME = 2,
  LAST_IN_FRAME = 3,
  LAST_IN_PEERSET = 4,
  MISC_POSITIONS = 5  // NTH_VALUE, LEAD/LAG have dynamic indexes 5..N
};

/**
  Represents the (explicit) window of a SQL 2003 section 7.11 \<window clause\>,
  or the implicit (inlined) window of a window function call, or a reference to
  a named window in a window function call (instead of the inlined definition)
  before resolution.  After resolving referencing instances become unused,
  having been replaced with the window resolved to in the w.f. call.

  @verbatim
  Cf. 7.11 <window definition> and <existing window name>
      6.10 <window name or specification> and
           <in-line window specification>
      5.4 <window name>
  @endverbatim

  See also PT_window (which wraps Window as a parse_tree_node), and the related
  classes PT_frame, PT_border and PT_exclusion in parse_tree_nodes.

  Currently includes both prepared query and execution state information.
  The latter is marked as such for ease of separation later.
*/
class Window {
  /*------------------------------------------------------------------------
   *
   * Variables stable during execution
   *
   *------------------------------------------------------------------------*/
 protected:
  SELECT_LEX *m_select;                 ///< The SELECT the window is on
  PT_order_list *const m_partition_by;  ///< \<window partition clause\>
  PT_order_list *const m_order_by;      ///< \<window order clause\>
  ORDER *m_sorting_order;               ///< merged partition/order by
  bool m_sort_redundant;                ///< Can use sort from previous w
  PT_frame *const m_frame;              ///< \<window frame clause\>
  Item_string *m_name;                  ///< \<window name\>
  /**
    Position of definition in query's text, 1 for leftmost. References don't
    count. Thus, anonymous windows in SELECT list, then windows of WINDOW
    clause, then anonymous windows in ORDER BY.
  */
  uint m_def_pos;
  Item_string *const m_inherit_from;  ///< \<existing window name\>
  /**
    If true, m_name is an unbound window reference, other fields
    are unused.
  */
  const bool m_is_reference;

  /**
    (At least) one window function needs to buffer frame rows for evaluation
    i.e. it cannot be evaluated on the fly just from previous rows seen
  */
  bool m_needs_frame_buffering;

  /**
    (At least) one window function needs the peer set of the current row to
    evaluate the wf for the current row
  */
  bool m_needs_peerset;

  /**
    (At least) one window function (currently JSON_OBJECTAGG) needs the
    last peer for the current row to evaluate the wf for the current row.
    (This is used only during inversion/optimization)
  */
  bool m_needs_last_peer_in_frame;

  /**
    (At least) one window function needs the cardinality of the partition of
    the current row to evaluate the wf for the current row
  */
  bool m_needs_card;

  /**
    The functions are optimizable with ROW unit. For example SUM is, MAX is
    not always optimizable. Optimized means we can use the optimized evaluation
    path in process_buffered_windowing_record which uses inversion to avoid
    revisiting all frame rows for every row being evaluated.
  */
  bool m_row_optimizable;

  /**
    The functions are optimizable with RANGE unit. For example SUM is, MAX is
    not always optimizable. Optimized means we can use the optimized evaluation
    path in process_buffered_windowing_record which uses inversion to avoid
    revisiting all frame rows for every row being evaluated.
  */
  bool m_range_optimizable;

  /**
    The aggregates (SUM, etc) can be evaluated once for a partition, since it
    is static, i.e. all rows will have the same value for the aggregates, e.g.
    ROWS/RANGE BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING.
  */
  bool m_static_aggregates;

  /**
    Window equires re-evaluation of the first row in optimized moving frame mode
    e.g. FIRST_VALUE.
  */
  bool m_opt_first_row;

  /**
    Window requires re-evaluation of the last row in optimized moving frame mode
    e.g. LAST_VALUE.
  */
  bool m_opt_last_row;

  /**
    Can be true if first window after a join: we may need to restore the input
    record after buffered window processing if EQRefIterator's caching logic
    presumes the record hasn't been modified (when last qep_tab uses JT_EQ_REF).
  */
  bool m_needs_restore_input_row;

  /**
    The last window to be evaluated at execution time.
  */
  bool m_last;

 public:
  struct st_offset {
    int64 m_rowno;
    bool m_from_last;
    st_offset() : m_rowno(0), m_from_last(false) {}
    /**
      Used for sorting offsets in ascending order for faster traversal of
      frame buffer tmp file
    */
    bool operator<(const st_offset &a) const { return m_rowno < a.m_rowno; }
  };

  struct st_ll_offset {
    int64 m_rowno;  ///< negative values is LEAD
    st_ll_offset() : m_rowno(INT_MIN64 /* uninitialized marker*/) {}
    /**
      Used for sorting offsets in ascending order for faster traversal of
      frame buffer tmp file
    */
    bool operator<(const st_ll_offset &a) const { return m_rowno < a.m_rowno; }
  };

  struct st_nth {
    Mem_root_array_YY<st_offset>
        m_offsets;  ///< sorted set of NTH_VALUE offsets
  };

  struct st_lead_lag {
    Mem_root_array_YY<st_ll_offset>
        m_offsets;  ///< sorted set of LEAD/LAG offsets
  };

 protected:
  /**
    Window requires re-evaluation of the Nth row in optimized moving frame mode
    e.g. NTH_VALUE.
  */
  st_nth m_opt_nth_row;
  st_lead_lag m_opt_lead_lag;

 protected:
  const Window *m_ancestor;             ///< resolved from existing window name
  List<Item_sum> m_functions;           ///< window functions based on 'this'
  List<Cached_item> m_partition_items;  ///< items for the PARTITION BY columns
  List<Cached_item> m_order_by_items;   ///< items for the ORDER BY exprs.

  /*------------------------------------------------------------------------
   *
   * Execution state variables
   *
   *------------------------------------------------------------------------*/
 public:
  /**
    Cardinality of m_frame_buffer_positions if no NTH_VALUE, LEAD/LAG
  */
  static constexpr int FRAME_BUFFER_POSITIONS_CARD =
      static_cast<int>(Window_retrieve_cached_row_reason::MISC_POSITIONS);

  /**
    Holds information about a position in the buffer frame as stored in a
    temporary file (cf. m_frame_buffer). We save a number of such positions
    to speed up lookup when we move the window frame, cf.
    m_frame_buffer_positions
  */
  struct Frame_buffer_position {
    ///< The size of the file position is determined by handler::ref_length
    uchar *m_position;
    ///< Row number in partition, 1-based
    int64 m_rowno;
    Frame_buffer_position(uchar *position, int64 rowno)
        : m_position(position), m_rowno(rowno) {}
  };

  /**
    Execution state: used iff m_needs_frame_buffering. Holds pointers to
    positions in the file in m_frame_buffer. We use these saved positions to
    avoid having to position to the first row in the partition and then
    making many read calls to find the desired row. By repositioning to a
    suitably(*) saved position we normally (**) need to do only one positioned
    read and one ha_rdn_next call to get at a desired row.
    @verbatim

      [0] the first row in the partition: at the
          beginning of a new partition that is the first row in the partition.
          Its rowno == 1 by definition.
      [1] position of the current row N  (for jump-back to current row or next
                                          current row in combo with ha_rnd_next)
      [2] position of the current first row M in frame (for aggregation looping
          jump-back)
      [3] position of the current last row in a frame
      [4] position and line number of the row last read
   and optionally:
      [5..X] positions of Nth row of X-5+1 NTH_VALUE functions invoked on window
      [X+1..Y] position of last row of lead/lag functions invoked on window

    @endverbatim
    Pointers are lazily initialized if needed.

    (*) We use the position closest below the desired position, cf logic in
    read_frame_buffer_row.

    (**) Unless we have a frame beyond the current row, in which case we need
    to do some scanning for the first row in the partition.
    Also NTH_VALUE with RANGE might sometimes needs to read several rows, since
    the frame start can jump several rows ahead when the current row moves
    forward.
  */
  Mem_root_array_YY<Frame_buffer_position> m_frame_buffer_positions;

  /**
    Sometimes we read one row too many, so that the saved position will
    be too far out because we subsequently need to read an earlier (previous)
    row of the same kind (reason). For such cases, we first save the
    current position, read, and if we see we read too far, restore the old
    position. See #save_pos and #restore_pos.
  */
  Frame_buffer_position m_tmp_pos;

  /**
    See #m_tmp_pos
  */
  void save_pos(Window_retrieve_cached_row_reason reason) {
    int reason_index = static_cast<int>(reason);
    m_tmp_pos.m_rowno = m_frame_buffer_positions[reason_index].m_rowno;
    std::memcpy(m_tmp_pos.m_position,
                m_frame_buffer_positions[reason_index].m_position,
                frame_buffer()->file->ref_length);
  }

  /**
    See #m_tmp_pos
  */
  void restore_pos(Window_retrieve_cached_row_reason reason) {
    int reason_index = static_cast<int>(reason);
    m_frame_buffer_positions[reason_index].m_rowno = m_tmp_pos.m_rowno;
    std::memcpy(m_frame_buffer_positions[reason_index].m_position,
                m_tmp_pos.m_position, frame_buffer()->file->ref_length);
  }

  /**
    Copy frame buffer position hint from one to another.
  */
  void copy_pos(Window_retrieve_cached_row_reason from_reason,
                Window_retrieve_cached_row_reason to_reason) {
    int from_index = static_cast<int>(from_reason);
    int to_index = static_cast<int>(to_reason);
    m_frame_buffer_positions[to_index].m_rowno =
        m_frame_buffer_positions[from_index].m_rowno;

    std::memcpy(m_frame_buffer_positions[to_index].m_position,
                m_frame_buffer_positions[from_index].m_position,
                frame_buffer()->file->ref_length);
  }

  /**
     Keys for m_frame_buffer_cache and m_special_rows_cache, for special
     rows.
  */
  enum Special_keys {
    /**
      We read an incoming row. We notice it is the start of a new
      partition. We must thus process the just-finished partition, but that
      processing uses this row's buffer; so, we save this row first, process
     the partition, and restore it later.
    */
    FBC_FIRST_IN_NEXT_PARTITION = -1,
    /// The last row cached in the frame buffer; needed to resurrect input row
    FBC_LAST_BUFFERED_ROW = -2,
    // Insert new values here.
    // And keep the ones below up to date.
    FBC_FIRST_KEY = FBC_FIRST_IN_NEXT_PARTITION,
    FBC_LAST_KEY = FBC_LAST_BUFFERED_ROW,
  };

 protected:
  /**
    Execution state: used iff m_needs_frame_buffering. Holds the temporary
    file (used for the frame buffering) parameters
  */
  Temp_table_param *m_frame_buffer_param;

  /**
    Execution state: Holds the temporary output table (for next step) parameters
  */
  Temp_table_param *m_outtable_param;

  /**
    Execution state: used iff m_needs_frame_buffering. Holds the TABLE
   object for the the temporary file used for the frame buffering.
  */
  TABLE *m_frame_buffer;

  /**
    Execution state: The frame buffer tmp file is not truncated for each new
    partition.  We need to keep track of where a partition starts in case we
    need to switch from heap to innodb tmp file on overflow, in order to
    re-initialize m_frame_buffer_positions with the current partition's row 1
    (which is the minimum hint required) as we cross over.  This number is
    incremented for each write.
  */
  int64 m_frame_buffer_total_rows;

  /**
    Execution state: Snapshot of m_frame_buffer_total_rows when we start a new
    partition, i.e. for the first row in the first partition we will have a
    value of 1.
  */
  int64 m_frame_buffer_partition_offset;

  /**
     If >=1: the row with this number (1-based, relative to start of
     partition) currently has its fields in the record buffer of the IN table
     and of the OUT table. 0 means "unset".
     Usable only with buffering. Set and read by bring_back_frame_row(), so
     that multiple successive calls to it for same row do only one read from
     FB (optimization).
  */
  int64 m_row_has_fields_in_out_table;

  /**
    Holds a fixed number of copies of special rows; each copy can use up to
    #m_special_rows_cache_max_length bytes.
    cf. the Special_keys enumeration.
  */
  uchar *m_special_rows_cache;
  /// Length of each copy in #m_special_rows_cache, in bytes
  size_t m_special_rows_cache_length[FBC_FIRST_KEY - FBC_LAST_KEY + 1];
  /// Maximum allocated size in #m_special_rows_cache
  size_t m_special_rows_cache_max_length;

  /**
    Execution state: used iff m_needs_frame_buffering. Holds the row
    number (in the partition) of the last row (hitherto) saved in the frame
    buffer
  */
  int64 m_last_rowno_in_cache;

  /**
    Execution state: used iff m_needs_peerset. Holds the rowno
    for the last row in this peer set.
  */
  int64 m_last_rowno_in_peerset;

  /**
    Execution state: used iff m_needs_last_peer_in_frame. True if a row
    leaving the frame is the last row in the peer set withing the frame.
  */
  int64 m_is_last_row_in_peerset_within_frame;

  /**
    Execution state: the current row number in the current partition.
    Set in check_partition_boundary. Used while reading input rows, in contrast
    to m_rowno_in_partition, which is used when processing buffered rows.
    Cf. check_partition_boundary.
  */
  int64 m_part_row_number;

  /**
    Execution state: the current row starts a new partition.
    Set in check_partition_boundary.
  */
  bool m_partition_border;

  /**
    Execution state: The number, in the current partition, of the last output
    row, i.e. the row number of the last row hitherto evaluated and output to
    the next phase.
  */
  int64 m_last_row_output;

  /**
    Execution state: The number of the row being visited for its contribution
    to a window function, relative to the start of the partition. Note that
    this will often be different from the current row for which we are
    processing the window function, reading it for output. That is given by
    m_rowno_in_partition, q.v.
  */
  int64 m_rowno_being_visited;

  /**
    Execution state: the row number of the current row within a frame, cf.
    m_is_last_row_in_frame, relative to start of the frame. 1-based.
  */
  int64 m_rowno_in_frame;

  /**
    Execution state: The row number of the current row being readied for
    output within the partition. 1-based.
  */
  int64 m_rowno_in_partition;

  /**
   Execution state: for optimizable aggregates, cf. m_row_optimizable and
   m_range_optimizable, we need to keep track of when we have computed the
   first aggregate, since aggregates for rows 2..N are computed in an optimized
   way by inverse aggregation of the row moving out of the frame.
   */
  bool m_aggregates_primed;

  /*------------------------------------------------------------------------
   *
   * RANGE boundary frame state variables.
   *
   *------------------------------------------------------------------------*/
 public:
  /**
    RANGE bound determination computation; first index is a value of the
    enum_window_border_type enum; second index 0 for the start bound, 1 for
    the end bound. Each item is Item_func_lt/gt.
  */
  Item_func *m_comparators[WBT_VALUE_FOLLOWING + 1][2];
  /**
    Each item has inverse operation of the corresponding
    comparator in m_comparators. Determines if comparison
    should continue with next field in order by list.
  */
  Item_func *m_inverse_comparators[WBT_VALUE_FOLLOWING + 1][2];

 protected:
  /**
    Execution state: the row number of the first row in a frame when evaluating
    RANGE based frame bounds. When using RANGE bounds, we don't know a priori
    when moving the frame which row number will be the next lower bound, but
    we know it will have to be a row number higher than the lower bound of
    the previous frame, since the bounds increase monotonically as long
    as the frame bounds are static within the query (current limitation).
    So, it makes sense to remember the first row number in a frame until
    we have determined the start of the next frame.

    If the frame for the previous current row in the partition was empty (cf.
    "current_row" in process_buffered_windowing_record), this should point
    to the next possible frame start. Relative to partition start, 1-based.
  */
  int64 m_first_rowno_in_range_frame;

  /**
    Execution state: used for RANGE bounds frame evaluation
    for the continued evaluation for current row > 2 in a partition.
    If the frame for the current row visited (cf "current_row" in
    process_buffered_windowing_record) was empty, the invariant

         m_last_rowno_in_range_frame < m_first_rowno_in_range_frame

    should hold after the visit. Relative to partition start. 1-based.
  */
  int64 m_last_rowno_in_range_frame;

  /*------------------------------------------------------------------------
   *
   * Window function special behaviour toggles. These boolean flag influence
   * the action taken when a window function is evaluated, i.e.
   *
   *     Item_xxx::val_xxx
   *
   * They are set locally just before and after a call to evaluate functions,
   * i.e.
   *
   *     w.set_<toggle>(true)
   *     copy_<kind>_window_functions()  [see process_buffered_windowing_record]
   *     w.set_<toggle>(false)
   *
   * to achive a special semantic, since we cannot pass down extra parameters.
   *
   *------------------------------------------------------------------------*/

  /**
    Execution state: the current row is the last row in a window frame
    For some aggregate functions, e.g AVG, we can save computation by not
    evaluating the entire function value before the last row has been read.
    For AVG, do the summation for each row in the frame, but the division only
    for the last row, at which time the result is needed for the wf.
    Probably only useful for ROW based or static frames.
    For frame with peer set computation, determining the last row in the
    peer set ahead of processing is not possible, so we use a pessimistic
    assumption.
  */
  bool m_is_last_row_in_frame;

  /**
    Execution state: make frame wf produce a NULL (or 0 depending, e.g. if
    COUNT) value because no rows are available for aggregation: e.g. for first
    row in partition if frame is ROWS BETWEEN 2 PRECEDING and 1 PRECEDING has
    no rows for which aggregation can happen
  */
  bool m_do_copy_null;

  /**
    Execution state: do inverse, e.g. subtract rather than add in aggregates.
    Used for optimizing computation of sliding frames for eligible aggregates,
    cf. Item_sum::check_wf_semantics.
  */
  bool m_inverse_aggregation;

  /*------------------------------------------------------------------------
   *
   * Constructors
   *
   *------------------------------------------------------------------------*/
 private:
  /**
    Generic window constructor, shared
  */
  Window(Item_string *name, PT_order_list *part, PT_order_list *ord,
         PT_frame *frame, bool is_reference, Item_string *inherit)
      : m_select(nullptr),
        m_partition_by(part),
        m_order_by(ord),
        m_sorting_order(nullptr),
        m_sort_redundant(false),
        m_frame(frame),
        m_name(name),
        m_def_pos(0),
        m_inherit_from(inherit),
        m_is_reference(is_reference),
        m_needs_frame_buffering(false),
        m_needs_peerset(false),
        m_needs_last_peer_in_frame(false),
        m_needs_card(false),
        m_row_optimizable(true),
        m_range_optimizable(true),
        m_static_aggregates(false),
        m_opt_first_row(false),
        m_opt_last_row(false),
        m_needs_restore_input_row(false),
        m_last(false),
        m_ancestor(nullptr),
        m_tmp_pos(nullptr, -1),
        m_frame_buffer_param(nullptr),
        m_outtable_param(nullptr),
        m_frame_buffer(nullptr),
        m_frame_buffer_total_rows(0),
        m_frame_buffer_partition_offset(0),
        m_row_has_fields_in_out_table(0),
        m_special_rows_cache_max_length(0),
        m_last_rowno_in_cache(0),
        m_last_rowno_in_peerset(0),
        m_is_last_row_in_peerset_within_frame(false),
        m_part_row_number(0),
        m_partition_border(true),
        m_last_row_output(0),
        m_rowno_being_visited(0),
        m_rowno_in_frame(0),
        m_rowno_in_partition(0),
        m_aggregates_primed(false),
        m_first_rowno_in_range_frame(1),
        m_last_rowno_in_range_frame(0),
        m_is_last_row_in_frame(false),
        m_do_copy_null(false),
        m_inverse_aggregation(false) {
    m_opt_nth_row.m_offsets.init_empty_const();
    m_opt_lead_lag.m_offsets.init_empty_const();
    m_frame_buffer_positions.init_empty_const();
  }

 public:
  /**
    Reference to a named window. This kind is only used before resolution,
    references to it being replaced by the referenced window object thereafter.
  */
  Window(Item_string *name)
      : Window(name, nullptr, nullptr, nullptr, true, nullptr) {}

  /**
    Unnamed window. If the window turns out to be named, the name will be set
    later, cf. #set_name().
  */
  Window(PT_order_list *partition_by, PT_order_list *order_by, PT_frame *frame)
      : Window(nullptr, partition_by, order_by, frame, false, nullptr) {}

  /**
    Unnamed window based on a named window. If the window turns out to be
    named, the name will be set later, cf. #set_name().
  */
  Window(PT_order_list *partition_by, PT_order_list *order_by, PT_frame *frame,
         Item_string *inherit)
      : Window(nullptr, partition_by, order_by, frame, false, inherit) {}

  /*------------------------------------------------------------------------
   *
   * Methods
   *
   *------------------------------------------------------------------------*/

  /**
    We have a named window. Now set its name. Used once, if at all, for a
    window as part of parsing.
  */
  void set_name(Item_string *name) { m_name = name; }

  /**
    After resolving an existing window name reference in a window definition,
    we set the ancestor pointer to easy access later.
  */
  void set_ancestor(Window *a) { m_ancestor = a; }

  /**
    Get the name of a window. Can be empty, cf. #printable_name which is not.
  */
  Item_string *name() const { return m_name; }

  uint def_pos() const { return m_def_pos; }       ///< @see #m_def_pos
  void set_def_pos(uint pos) { m_def_pos = pos; }  ///< @see #m_def_pos

  /**
    Get the frame, if any. SQL 2011 7.11 GR 1.b.i.6
  */
  const PT_frame *frame() const { return m_frame; }

  /**
    Get the ORDER BY, if any. That is, the first we find along
    the ancestor chain. Uniqueness checked in #setup_windows
    SQL 2011 7.11 GR 1.b.i.5.A-C
  */
  const PT_order_list *effective_order_by() const {
    const PT_order_list *o = m_order_by;
    const Window *w = m_ancestor;

    while (o == nullptr && w != nullptr) {
      o = w->m_order_by;
      w = w->m_ancestor;
    }
    return o;
  }

  /**
    Get the first argument of the ORDER BY clause for this window
    if any. "ORDER BY" is not checked in ancestor unlike
    effective_order_by().
    Use when the goal is to operate on the set of item clauses for
    all windows of a query. When interrogating the effective order
    by for a window (specified for it or inherited from another
    window) use effective_order_by().
  */
  ORDER *first_order_by() const;

  /**
    Get partition, if any. That is, the partition if any, of the
    root window. SQL 2011 7.11 GR 1.b.i.4.A-C
  */
  const PT_order_list *effective_partition_by() const {
    const PT_order_list *p = m_partition_by;
    const Window *w = m_ancestor;
    while (w != nullptr) {
      if (w->m_ancestor == nullptr) {
        /* root  */
        p = w->m_partition_by;
      } else {
        /* See #setup_windows for checking */
        DBUG_ASSERT(w->m_partition_by == nullptr);
      }
      w = w->m_ancestor;
    }
    return p;
  }
  /**
    Get the first argument of the PARTITION clause for this window
    if any. "PARTITION BY" is not checked in ancestor unlike
    effective_partition_by().
    Use when the goal is to operate on the set of item clauses for
    all windows of a query. When interrogating the effective
    partition by for a window (specified for it or inherited from
    another window) use effective_partition_by().
  */
  ORDER *first_partition_by() const;

  /**
    Get the list of functions invoked on this window.
  */
  List<Item_sum> &functions() { return m_functions; }

  /**
    Concatenation of columns in PARTITION BY and ORDER BY.
    Columns present in both list (redundancies) are eliminated, while
    making sure the order of columns in the ORDER BY is maintained
    in the merged list.

    @param thd                Optional. Session state. If not nullptr,
                              initialize the cache.

    @param implicit_grouping  Optional. If true, we won't sort (single row
                              result set). Presence implies thd != nullptr for
                              the first call when we lazily set up this
                              information.  Succeeding calls return the cached
                              value.

    @returns The start of the concatenated ordering expressions, or nullptr
  */
  ORDER *sorting_order(THD *thd = nullptr, bool implicit_grouping = false);

  /**
    Check that the semantic requirements for window functions over this
    window are fulfilled, and accumulate evaluation requirements
  */
  bool check_window_functions(THD *thd, SELECT_LEX *select);

  /**
    For RANGE frames we need to do computations involving add/subtract and
    less than, smaller than. To make this work across types, we construct
    item trees to do the computations, so we can reuse all the special case
    handling, e.g. for signed/unsigned int wrap-around, overflow etc.
  */
  bool setup_range_expressions(THD *thd);

  /**
    Return if this window represents an unresolved window reference seen
    in a window function OVER clause.
  */
  bool is_reference() const { return m_is_reference; }

  /**
    Check if the just read input row marks the start of a new partition.
    Sets the member variables:

    m_partition_border and m_part_row_number

  */
  void check_partition_boundary();

  /**
    Reset the current row's ORDER BY expressions when starting a new
    peer set.
  */
  void reset_order_by_peer_set();

  /**
    Determine if the current row is not in the same peer set as the previous
    row. Used for RANGE frame and implicit RANGE frame (the latter is used by
    aggregates in the presence of ORDER BY).

    The current row is in the same peer set if all ORDER BY columns
    have the same value as in the previous row.

    For JSON_OBJECTAGG only the first order by column needs to be
    compared to check if a row is in peer set.

    @param compare_all_order_by_items If true, compare all the order by items
                                      to determine if a row is in peer set.
                                      Else, compare only the first order by
                                      item to determine peer set.

    @return true if current row is in a new peer set
  */
  bool in_new_order_by_peer_set(bool compare_all_order_by_items = true);

  /**
    While processing buffered rows in RANGE frame mode we, determine
    if the present row revisited from the buffer is before the row
    being processed; i.e. the current row.

    @return true if the present row is before the RANGE, i.e. not to
    be included
  */
  bool before_frame() { return before_or_after_frame(true); }

  ///< See before_frame()
  bool after_frame() { return before_or_after_frame(false); }

  /**
    Check if we have read all the rows in a partition, possibly
    having buffered them for further processing

    @returns true if this is the case
  */
  bool at_partition_border() const { return m_partition_border; }

  void save_special_record(uint64 special_rowno, TABLE *t);
  void restore_special_record(uint64 special_rowno, uchar *record);

  /**
    Resolve any named window to its definition
    and update m_window to point to the definition instead
  */
  static bool resolve_reference(THD *thd, Item_sum *wf, PT_window **m_window);

  /**
    Semantic checking of windows.

    * Process any window inheritance, that is a window, that in its
    specification refer to another named window.

    Rules:
    1) There should be no loops
    2) The inheriting window can not specify partitioning
    3) The inheriting window can not specify is already specify by
       an ancestor.
    4) An ancestor can not specify window framing clause.

    Cf. SQL 2011 7.11 window clause SR 10-11.

    * Check requirements to the window from its using window functions and
    make a note of those so we know at execution time, for example if we need
    to buffer rows to process the window functions, whether inversion
    optimzation will be used for moving frames etc.

    * Prepare the physical ordering lists used by sorting at execution time.

    * Set up cached items for partition determination and for range/peer
    determination based on order by columns.

    * Check any frame semantics and for RANGE frames, set up bounds computation
    item trees.

    @param thd              The session's execution thread
    @param select           The select for which we are doing windowing
    @param ref_item_array   The base ref items
    @param tables           The list of tables involved
    @param fields           The list of selected fields
    @param all_fields       The list of all fields, including hidden ones
    @param windows          The list of windows defined for this select

    @return false if success, true if error
  */
  static bool setup_windows(THD *thd, SELECT_LEX *select,
                            Ref_item_array ref_item_array, TABLE_LIST *tables,
                            List<Item> &fields, List<Item> &all_fields,
                            List<Window> &windows);

  /**
    Remove unused window definitions. Do this only after syntactic and
    semantic checking for errors has been performed.

    @param thd             The session's execution thread
    @param windows         The list of windows defined for this select
  */
  static void remove_unused_windows(THD *thd, List<Window> &windows);

  /**
    Resolve and set up the PARTITION BY or an ORDER BY list of a window.

    @param thd              The session's execution thread
    @param ref_item_array
    @param tables           The list of tables involved
    @param fields           The list of selected fields
    @param all_fields       The list of all fields, including hidden ones
    @param o                A list of order by expressions
    @param partition_order  If true, o represent a windowing PARTITION BY,
           else it represents a windowing ORDER BY
    @returns false if success, true if error
  */
  bool resolve_window_ordering(THD *thd, Ref_item_array ref_item_array,
                               TABLE_LIST *tables, List<Item> &fields,
                               List<Item> &all_fields, ORDER *o,
                               bool partition_order);
  /**
    Return true if this window's name is not unique in windows
  */
  bool check_unique_name(List<Window> &windows);

  /**
    Set up cached items for an partition or an order by list
    updating m_partition_items or m_order_by_items respectively.

    @param thd              The session's execution thread
    @param select           The select for which we are doing windowing
    @param o                The list of ordering expressions
    @param partition_order  If true, o represents a partition order list,
                            else an ORDER BY list.

    @returns false if success, true if error
  */
  bool setup_ordering_cached_items(THD *thd, SELECT_LEX *select,
                                   const PT_order_list *o,
                                   bool partition_order);

  /**
    Determine if the window had either a partition clause (inclusive) or a
    ORDER BY clause, either defined by itself or inherited from another window.

    @return true if we have such a clause, which means we need to sort the
            input table before evaluating the window functions, unless it has
            been made redundant by a previous windowing step, cf.
            m_sort_redundant, or due to a single row result set, cf.
            SELECT_LEX::is_implicitly_grouped().
  */
  bool needs_sorting() const { return m_sorting_order != nullptr; }

  /**
    If we cannot compute one of window functions without looking at succeeding
    rows, return true, else false.
  */
  bool needs_buffering() const { return m_needs_frame_buffering; }

  /**
    If we cannot compute one of window functions without looking at all
    rows in the peerset of the current row, return true, else
    false. E.g. CUME_DIST.
  */
  bool needs_peerset() const { return m_needs_peerset; }

  /**
    If we cannot compute one of window functions without looking at all
    rows in the peerset of the current row in this frame, return true, else
    false. E.g. JSON_OBJECTAGG.
  */
  bool needs_last_peer_in_frame() const { return m_needs_last_peer_in_frame; }
  /**
    If we need to read the entire partition before we can evaluate
    some window function(s) on this window,
    @returns true if that is the case, else false
  */
  bool needs_card() const { return m_needs_card; }

  /**
    Return true if the set of window functions are all ROW unit optimizable.
    Only relevant if m_needs_buffering and m_row_optimizable are true.
  */
  bool optimizable_row_aggregates() const { return m_row_optimizable; }

  /**
    Return true if the set of window functions are all RANGE unit optimizable.
    Only relevant if m_needs_buffering and m_range_optimizable are true.
  */
  bool optimizable_range_aggregates() const { return m_range_optimizable; }

  /**
    Return true if the aggregates are static, i.e. the same aggregate values for
    all rows in partition. Only relevant if m_needs_buffering is true.
  */
  bool static_aggregates() const { return m_static_aggregates; }

  /**
    See #m_opt_first_row
  */
  bool opt_first_row() const { return m_opt_first_row; }

  /**
    See #m_opt_last_row
  */
  bool opt_last_row() const { return m_opt_last_row; }

  /**
    See #m_last
  */
  bool is_last() const { return m_last; }

  /**
    See #m_needs_restore_input_row
  */
  void set_needs_restore_input_row(bool b) { m_needs_restore_input_row = b; }

  /**
    See #m_needs_restore_input_row
  */
  bool needs_restore_input_row() const { return m_needs_restore_input_row; }

  /**
    See #m_opt_nth_row
  */
  const st_nth &opt_nth_row() const { return m_opt_nth_row; }

  /**
    See #m_opt_lead_lag
  */
  const st_lead_lag &opt_lead_lag() const { return m_opt_lead_lag; }

  /**
    Getter for m_frame_buffer_param, q.v.
  */
  Temp_table_param *frame_buffer_param() const { return m_frame_buffer_param; }

  /**
    Setter for m_frame_buffer_param, q.v.
  */
  void set_frame_buffer_param(Temp_table_param *p) { m_frame_buffer_param = p; }

  /**
    Getter for m_frame_buffer, q.v.
  */
  TABLE *frame_buffer() const { return m_frame_buffer; }

  /**
    Setter for m_frame_buffer, q.v.
  */
  void set_frame_buffer(TABLE *tab) { m_frame_buffer = tab; }

  /**
   Getter for m_outtable_param, q.v.
   */
  Temp_table_param *outtable_param() const { return m_outtable_param; }

  /**
   Setter for m_outtable_param, q.v.
   */
  void set_outtable_param(Temp_table_param *p) { m_outtable_param = p; }

  /**
    Getter for m_part_row_number, q.v., the current row number within the
    partition.
  */
  int64 partition_rowno() const { return m_part_row_number; }

  /**
    Allocate the cache for special rows
    @param thd      thread handle
    @param out_tbl  The table where this window function's value is written to
    @returns true if error.
  */
  bool make_special_rows_cache(THD *thd, TABLE *out_tbl);

  /**
    See #m_last_row_output
  */
  int64 last_row_output() const { return m_last_row_output; }

  /**
    See #m_last_row_output
  */
  void set_last_row_output(int64 rno) { m_last_row_output = rno; }

  /**
   See #m_rowno_being_visited
   */
  int64 rowno_being_visited() const { return m_rowno_being_visited; }

  /**
   See #m_rowno_being_visited
   */
  void set_rowno_being_visited(int64 rno) { m_rowno_being_visited = rno; }

  /**
    See #m_last_rowno_in_cache
  */
  int64 last_rowno_in_cache() const { return m_last_rowno_in_cache; }

  /**
    See #m_last_rowno_in_cache
  */
  void set_last_rowno_in_cache(uint64 rno) { m_last_rowno_in_cache = rno; }

  /**
    See #m_last_rowno_in_range_frame
  */
  int64 last_rowno_in_range_frame() const {
    return m_last_rowno_in_range_frame;
  }

  /**
    See #m_last_rowno_in_range_frame
  */
  void set_last_rowno_in_range_frame(uint64 rno) {
    m_last_rowno_in_range_frame = rno;
  }

  /**
    See #m_last_rowno_in_peerset
  */
  int64 last_rowno_in_peerset() const { return m_last_rowno_in_peerset; }

  /**
    See #m_last_rowno_in_peerset
  */
  void set_last_rowno_in_peerset(uint64 rno) { m_last_rowno_in_peerset = rno; }

  /**
    See #m_is_last_row_in_peerset_within_frame
  */
  int64 is_last_row_in_peerset_within_frame() const {
    return m_is_last_row_in_peerset_within_frame;
  }

  /**
    See #m_is_last_row_in_peerset_within_frame
  */
  void set_is_last_row_in_peerset_within_frame(bool value) {
    m_is_last_row_in_peerset_within_frame = value;
  }

  /**
    See #m_do_copy_null
  */
  bool do_copy_null() const { return m_do_copy_null; }

  /**
    See #m_do_copy_null
  */
  void set_do_copy_null(bool b) { m_do_copy_null = b; }

  /**
    See #m_inverse_aggregation
  */
  bool do_inverse() const { return m_inverse_aggregation; }

  /**
    See #m_inverse_aggregation
  */
  Window &set_inverse(bool b) {
    m_inverse_aggregation = b;
    return *this;
  }

  /**
    See #m_aggregates_primed
  */
  bool aggregates_primed() const { return m_aggregates_primed; }

  /**
    See #m_aggregates_primed
  */
  void set_aggregates_primed(bool b) { m_aggregates_primed = b; }

  /**
    See #m_is_last_row_in_frame
  */
  bool is_last_row_in_frame() const {
    return m_is_last_row_in_frame || m_select->table_list.elements == 0;
  }

  /**
    See #m_is_last_row_in_frame
  */
  void set_is_last_row_in_frame(bool b) { m_is_last_row_in_frame = b; }

  /**
    Return the size of the frame in number of rows
    @returns frame size
  */
  // int64 frame_cardinality();

  /**
    See #m_rowno_in_frame
  */
  int64 rowno_in_frame() const { return m_rowno_in_frame; }

  /**
    See #m_rowno_in_frame
  */
  Window &set_rowno_in_frame(int64 rowno) {
    m_rowno_in_frame = rowno;
    return *this;
  }

  /**
    See #m_rowno_in_partition
  */
  int64 rowno_in_partition() const { return m_rowno_in_partition; }

  /**
    See #m_rowno_in_partition
  */
  void set_rowno_in_partition(int64 rowno) { m_rowno_in_partition = rowno; }

  /**
    See #m_first_rowno_in_range_frame
  */
  void set_first_rowno_in_range_frame(int64 rowno) {
    m_first_rowno_in_range_frame = rowno;
  }

  /**
    See #m_first_rowno_in_range_frame
  */
  int64 first_rowno_in_range_frame() const {
    return m_first_rowno_in_range_frame;
  }

  /**
    See #m_frame_buffer_total_rows
  */
  void set_frame_buffer_total_rows(int64 rows) {
    m_frame_buffer_total_rows = rows;
  }

  /**
    See #m_frame_buffer_total_rows
  */
  int64 frame_buffer_total_rows() const { return m_frame_buffer_total_rows; }

  /**
    See #m_frame_buffer_partition_offset
  */
  void set_frame_buffer_partition_offset(int64 offset) {
    m_frame_buffer_partition_offset = offset;
  }

  /**
    See #m_frame_buffer_partition_offset
  */
  int64 frame_buffer_partition_offset() const {
    return m_frame_buffer_partition_offset;
  }

  /**
    See #m_row_has_fields_in_out_table
  */
  int64 row_has_fields_in_out_table() const {
    return m_row_has_fields_in_out_table;
  }

  /**
    See #m_row_has_fields_in_out_table
  */
  void set_row_has_fields_in_out_table(int64 rowno) {
    m_row_has_fields_in_out_table = rowno;
  }

  /**
    Free up any resource used to process the window functions of this window,
    e.g. temporary files and in-memory data structures. Called when done
    with all window processing steps from SELECT_LEX::cleanup.
  */
  void cleanup(THD *thd);

  /**
   Reset window state for a new partition.

   Reset the temporary storage used for window frames, typically when we find
   a new partition. The rows in the buffer are then no longer needed.
   */
  void reset_partition_state() { reset_execution_state(RL_PARTITION); }

  /**
    Reset execution state for next call to JOIN::exec, cf. JOIN::reset,
    or using [Buffering]WindowingIterator::Init.
  */
  void reset_round() { reset_execution_state(RL_ROUND); }

  /**
    Reset resolution and execution state to prepare for next execution of a
    prepared statement.
  */
  void reinit_before_use() { reset_execution_state(RL_FULL); }

  /**
    Reset execution state for LEAD/LAG for the current row in partition.
  */
  void reset_lead_lag();

  enum Reset_level { RL_FULL, RL_ROUND, RL_PARTITION };

 private:
  /// Common function for all types of resetting
  void reset_execution_state(Reset_level level);

 public:
  /**
    Reset the execution state for all window functions defined on this window.
  */
  void reset_all_wf_state();

  const char *printable_name() const;

  void print(const THD *thd, String *str, enum_query_type qt,
             bool expand_definition) const;

  bool has_windowing_steps() const;

  /**
    Compute sorting costs for windowing.

    @param cost Cost of sorting result set once
    @param windows The set of windows

    @returns the aggregated sorting costs of the windowing
  */
  static double compute_cost(double cost, List<Window> &windows);

 private:
  /**
    Common implementation of before_frame() and after_frame().
    @param  before  True if 'before' is wanted; false if 'after' is.
  */
  bool before_or_after_frame(bool before);
  void print_frame(const THD *thd, String *str, enum_query_type qt) const;
  void print_border(const THD *thd, String *str, PT_border *b,
                    enum_query_type qt) const;

  /**
    Reorder windows and eliminate redundant ordering. If a window has the
    same ordering requirements as another, we will move them next to each other
    in the evaluation sequence, so we can sort only once, i.e. before the
    first window step. This allows us to fulfill the guarantee given by
    SQL standard when it comes to repeatability of non-deterministic (partially
    ordered) result sets for windowing inside a query, cf. #equal_sort.
    If more than two have the same ordering, the same applies, we only sort
    before the first (sort equivalent) window.

    If the result set is implicitly grouped, we also skip any sorting for
    windows.

    @param windows     list of windows
    @param first_exec  if true, the as done a part of a first prepare, not a
                       reprepare. On a reprepare the analysis part will be
                       skipped, since the flag m_sort_redundant flag is stable
                       across prepares.
    @todo in WL#6570 we may set m_sorting_order only once, during preparation,
    then Window::m_sort_redundant could be removed, as well as the first_exec
    argument.
  */
  static void reorder_and_eliminate_sorts(List<Window> &windows,
                                          bool first_exec);

  /**
    Return true of the physical[1] sort orderings for the two windows are the
    same, cf. guarantee of
    SQL 2014 4.15.15 Windowed tables bullet two: The windowing functions are
    computed using the same row ordering if they specify the same ordering.

    Collation and null handling is not supported, so moot.

    The two other bullet points are also covered by this test.

    [1] After concatenating effective PARTITION BY and ORDER BY (including
    inheritance) expressions.
  */
  static bool equal_sort(Window *w1, Window *w2);

  /**
    Check that a frame border is constant during execution and that it does
    not contain subqueries (relevant for INTERVAL only): implementation
    limitation.

    @param thd    Session thread
    @param border The border to check

    @return false if OK, true if error
  */
  bool check_constant_bound(THD *thd, PT_border *border);

  /**
    Check that frame borders are sane, e.g. they are not negative .

    @param thd      Session thread
    @param w        The window whose frame we are checking
    @param f        The frame to check, if any
    @param prepare  false at EXECUTE ps prepare time, else true

   @returns true if error
  */
  static bool check_border_sanity(THD *thd, Window *w, const PT_frame *f,
                                  bool prepare);
};

/**
  Collects evaluation requirements from a window function,
  used by Item_sum::check_wf_semantics and its overrides.
*/
struct Window_evaluation_requirements {
  /**
    Set to true if window function requires row buffering
  */
  bool needs_buffer;
  /**
    Set to true if we need peerset for evaluation (e.g. CUME_DIST)
  */
  bool needs_peerset;
  /**
    Set to true if we need last peer for evaluation within a frame
    (e.g. JSON_OBJECTAGG)
  */
  bool needs_last_peer_in_frame;
  /**
    Set to true if we need FIRST_VALUE or optimized MIN/MAX
  */
  bool opt_first_row;
  /**
    Set to true if we need LAST_VALUE or optimized MIN/MAX
  */
  bool opt_last_row;
  Window::st_offset opt_nth_row;    ///< Used if we have NTH_VALUE
  Window::st_ll_offset opt_ll_row;  ///< Used if we have LEAD or LAG
  /**
    Set to true if we can compute a sliding window by a combination of
    undoing the contribution of rows going out of the frame and adding the
    contribution of rows coming into the frame.  For example, SUM and AVG
    allows this, but MAX/MIN do not. Only applicable if the frame has ROW
    bounds unit.
  */
  bool row_optimizable;
  /**
    Similar to row_optimizable but for RANGE frame bounds unit
  */
  bool range_optimizable;

  Window_evaluation_requirements()
      : needs_buffer(false),
        needs_peerset(false),
        needs_last_peer_in_frame(false),
        opt_first_row(false),
        opt_last_row(false),
        row_optimizable(true),
        range_optimizable(true) {}
};

#endif /* WINDOWS_INCLUDED */
