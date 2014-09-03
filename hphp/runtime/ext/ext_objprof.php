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
  'class' => string,
  'instances' => int,
);

<<__Native>>
function objprof_get_data(): array<array<string, ObjprofObjectStats>>;

}
