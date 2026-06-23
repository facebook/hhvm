<?hh
// Function-typed parameters exercise variance composition through nested
// `Tfun`s. A callback that *accepts* `this` is safe (covariant); one that
// *returns* `this` is obtainable (contravariant). Nesting flips the sign each
// level.
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class A {
  // (function(this): void): contra . contra = covariant; obtainable=0 -> OK
  public static function s_accepts_two(
    (function(this): void) $_,
    (function(this): void) $_,
  ): void {}

  // (function(): this): contra . cov = contravariant; obtainable=2 -> ERROR
  public static function s_returns_two(
    (function(): this) $_,
    (function(): this) $_,
  ): void {}

  // depth 2, returns nested: contra . contra . cov = covariant; obtainable=0 -> OK
  public static function s_deep_safe(
    (function((function(): this)): void) $_,
    (function((function(): this)): void) $_,
  ): void {}

  // depth 2, accepts nested: contra . contra . contra = contravariant;
  // obtainable=2 -> ERROR
  public static function s_deep_unsafe(
    (function((function(this): void)): void) $_,
    (function((function(this): void)): void) $_,
  ): void {}

  // instance receiver(1) + accepts(0) = 1 -> OK
  public function i_accepts((function(this): void) $_): void {}
  // instance receiver(1) + returns(1) = 2 -> ERROR
  public function i_returns((function(): this) $_): void {}
}

function test(): void {
  $ok1 = A::s_accepts_two<>;
  $ok2 = A::s_deep_safe<>;
  $ok3 = meth_caller(A::class, 'i_accepts');

  $bad1 = A::s_returns_two<>;
  $bad2 = A::s_deep_unsafe<>;
  $bad3 = meth_caller(A::class, 'i_returns');
}
