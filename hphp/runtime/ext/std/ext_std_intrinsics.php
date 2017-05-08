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

/**
 * Return the given value. This function is purposefully not optimized. It can
 * be used to hide information about values from the optimizer for unit testing.
 */
<<__Native, __HipHopSyntax>>
function launder_value(mixed $value): mixed;

}
