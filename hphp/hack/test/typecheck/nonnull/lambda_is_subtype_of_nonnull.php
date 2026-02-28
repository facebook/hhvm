<?hh

function f(): nonnull {
  return function(int $x) {
    return $x;
  };
}
