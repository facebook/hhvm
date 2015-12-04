<?hh

namespace HH {

/**
 * Do a memory scan and collect information about allocated objects
 *
 * @return Array - an Array of (object allocation datas) as well as
 *  It is possible for the output of this function to change in the near
 *  future.  If so, it will be documented.
 */

type ObjprofPathsStats = shape(
  'instances' => int,
  'bytes' => int,
  'path' => array<string>,
);

type ObjprofObjectStats = shape(
  'instances' => int,
  'bytes' => int,
  'paths' => ObjprofPathsStats,
);

type ObjprofStringStats = shape(
  'dups' => int,
  'refs' => int,
  'srefs' => int,
  'path' => string,
);


<<__Native>>
function objprof_get_data(): array<string, ObjprofObjectStats>;

<<__Native>>
function objprof_get_strings(int $min_dup): array<string, ObjprofStringStats>;

<<__Native>>
function objprof_get_paths(): array<string, ObjprofObjectStats>;

<<__Native>>
function thread_memory_stats(): array<string, int>;

<<__Native>>
function thread_mark_stack(): void;

<<__Native>>
function set_mem_threshold_callback(int $threshold, mixed $callback): void;
}
