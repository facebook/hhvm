<?hh // strict

class C {}

function cast_C(C $x): nonnull {
  return $x;
}

interface I {}

function cast_I(I $x): nonnull {
  return $x;
}
