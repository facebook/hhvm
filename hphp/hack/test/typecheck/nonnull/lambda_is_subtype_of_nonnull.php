<?hh // strict

function f(): nonnull {
  return function(int $x) {
    return $x;
  };
}
