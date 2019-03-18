<?hh // partial

namespace HH {

/**
 * Do a memory scan and collect information about allocated objects
 *
 * @return Array - an Array of (object allocation datas) as well as
 *  It is possible for the output of this function to change in the near
 *  future.  If so, it will be documented.
 */

type ObjprofPathsStats = shape(
  'refs' => int,
);

type ObjprofObjectStats = shape(
  'instances'        => int,
  'bytes'            => int,
  'bytes_normalized' => float,
  'paths'            => ?darray<string, ObjprofPathsStats>,
);

type ObjprofStringStats = shape(
  'dups'  => int,
  'refs'  => int,
  'srefs' => int,
  'path'  => string,
);

<<__Native>>
function objprof_get_data(
  int $flags = \OBJPROF_FLAGS_DEFAULT,
  varray<string> $exclude_list = varray[],
): darray<string, ObjprofObjectStats>;

<<__Native>>
function objprof_get_paths(
  int $flags = \OBJPROF_FLAGS_DEFAULT,
  varray<string> $exclude_list = varray[],
): darray<string, ObjprofObjectStats>;

<<__Native>>
function objprof_get_strings(int $min_dup): darray<string, ObjprofStringStats>;

<<__Native>>
function thread_memory_stats(): darray<string, int>;

<<__Native>>
function thread_mark_stack(): void;

<<__Native>>
function set_mem_threshold_callback(int $threshold, mixed $callback): void;

} // namespace HH
