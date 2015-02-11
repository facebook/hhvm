<?hh

namespace HH {

/**
 * Do a memory scan and collect information about allocated objects
 *
 * @return Array - an Array of (object allocation datas) as well as
 *  It is possible for the output of this function to change in the near
 *  future.  If so, it will be documented.
 */

type ObjprofObjectStats = shape(
  'instances' => int,
  'bytes' => int,
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

}
