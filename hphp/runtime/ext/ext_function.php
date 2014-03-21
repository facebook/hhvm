<?hh

namespace __SystemLib {

/**
 * For internal use only, used to merge array_slice() with func_get_args()
 * while avoiding unnecessary copies and allocations. This should not be called
 * directly.
 */
<<__Native>>
function func_slice_args(int $offset) : mixed;

}
