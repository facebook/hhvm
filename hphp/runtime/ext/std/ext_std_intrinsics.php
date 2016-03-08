<?hh

namespace __hhvm_intrinsics {

/**
 * @param callable $callback
 *
 * @return void
 */
<<__Native, __HipHopSpecific>>
function disable_inlining(mixed $callback): void;

/**
 * @param bool $oom
 *
 * @return void
 */
<<__Native, __HipHopSpecific>>
function trigger_oom(bool $oom): void;

}
