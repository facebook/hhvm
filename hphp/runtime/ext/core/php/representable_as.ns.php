<?hh

namespace HH\Runtime {

newtype RepresentableAs<+T> = T;

/**
 * Unwrap a RepresentableAs<T> to its underlying T.
 * At runtime this is the identity function.
 */
function reveal<T>(RepresentableAs<T> $x)[]: T {
  return $x;
}
}
