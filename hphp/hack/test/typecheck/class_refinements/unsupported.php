<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

abstract class Box {
  abstract const type T as mixed;
  abstract public function get() : this::T;
  abstract public function set(this::T $val) : void;
}

function unsupported_bounds<T>(Box with {type T as T} $b): T {
  while (true) {}
}

function unsupported_root<Ta as Box, T>(Ta with {type T = T} $b): T {
  while (true) {}
}

interface Covariant<+T> { public function unwrap(): T; }

abstract class UnsupportedClassGenericRefinement<TBox as Box> {
  public static function subOuterRefinement<T0, TBox0>(
    Covariant<TBox0> $cov_box,
  ): T0 where TBox0 as TBox with { type T = T0 } { // unsupported in Hack
    return $cov_box->unwrap()->get(); // ERROR: `TBox0` doesn't have `get`
  } // NOTE: ^ works in Scala, but we disallow `with` on generics by design

  public static function eqOuterRefinement<T0, TBox0>(
    Covariant<TBox0> $cov_box,
  ): T0 where TBox0 = TBox with { type T = T0 } { // unsupported in Hack
    return $cov_box->unwrap()->get(); // ERROR: `TBox0` doesn't have `get`
  } // NOTE: ^ works in Scala, but we disallow `with` on generics by design
}
