<?hh // strict

function f(): nonnull {
  return function($x) {
    return $x;
  };
}
