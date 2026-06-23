<?hh
// `this` inside a *covariant* container is still contravariant overall when the
// container sits in a parameter (you can read a `this` out of it), so it counts.
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

final class Co<+T> {}

abstract class A {
  // contra . covariant-container = contravariant; c=1 each -> all OK (single)
  public static function s_vec_one(vec<this> $_): void {}
  public static function s_opt_one(?this $_): void {}
  public static function s_co_one(Co<this> $_): void {}
  public static function s_shape_one(shape('x' => this) $_): void {}

  // c=2 -> ERROR
  public static function s_vec_two(vec<this> $_, vec<this> $_): void {}
  public static function s_opt_two(?this $_, ?this $_): void {}
  public static function s_co_two(Co<this> $_, Co<this> $_): void {}
  public static function s_shape_two(
    shape('x' => this) $_,
    shape('x' => this) $_,
  ): void {}
  // mixed covariant-container kinds, c=2 -> ERROR
  public static function s_vec_and_bare(vec<this> $_, this $_): void {}
}

function test(): void {
  $ok1 = A::s_vec_one<>;
  $ok2 = A::s_opt_one<>;
  $ok3 = A::s_co_one<>;
  $ok4 = A::s_shape_one<>;

  $bad1 = A::s_vec_two<>;
  $bad2 = A::s_opt_two<>;
  $bad3 = A::s_co_two<>;
  $bad4 = A::s_shape_two<>;
  $bad5 = A::s_vec_and_bare<>;
}
