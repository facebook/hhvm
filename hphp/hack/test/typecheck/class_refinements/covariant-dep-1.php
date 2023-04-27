<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Covariant<+T> { public function unwrap(): T; }

interface Box {
  abstract const type T;
  public function get(): this::T;
}

function genericSubRefinement<T0, TBox>(
    Covariant<TBox> $cov_box,
): T0 where TBox as Box with { type T = T0 } {
  return $cov_box->unwrap()->get(); // OK
}

abstract class OuterGet<TBox as Box> {
  public static function outerSubRefinement<T0>(
    Covariant<TBox> $cov_box,
  ): T0 where TBox as Box with { type T = T0 } {
    return $cov_box->unwrap()->get(); // OK
  }

  public static function genericSubOuterAndRefinement<T0, TBox0>(
    Covariant<TBox0> $cov_box,
  ): T0 where TBox0 as Box with { type T = T0 }, TBox0 as TBox {
    return $cov_box->unwrap()->get(); // OK
  }

  public static function outerEqRefinement<T0>(
    Covariant<TBox> $cov_box,
  ): T0 where TBox = Box with { type T = T0 } {
    return $cov_box->unwrap()->get(); // OK
  }

  public static function eqOuterProjection<T0>(
    Covariant<TBox> $cov_box,
  ): T0 where TBox::T = T0 {
    return $cov_box->unwrap()->get(); // OK?
  }

  public static function superOuterProjection<T0>(
    Covariant<TBox> $cov_box,
  ): T0 where TBox::T as T0 {
    return $cov_box->unwrap()->get(); // OK
  }

  public static function subOuterProjection<T0>(
    Covariant<TBox> $cov_box,
  ): T0 where TBox::T super T0 {
    return $cov_box->unwrap()->get(); // ERROR
  }
}

function anon_refinement<T0>(
  Covariant<Box with { type T = T0 }> $cov_box,
): T0 {
  return $cov_box->unwrap()->get(); // OK
}
